// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

// --- Includes ---

#include "tcb.h"
#include "ctx.h"
#include "task.h"
#include "pplibc.h"
#include "dispatcher.h"
#include "macros.h"
#include "queue.h"
#include "memory.h"
#include "scheduler.h"


// --- Variaveis Globais ---

struct queue_t * ready_queue;
int user_tasks;


// --- Funcoes/Variaveis Externas ---

extern struct task_t * current_task;
extern struct task_t * kernel;
extern void user_main (void * arg);


// --- Funcoes da API ---

void dispatcher_init () {
    if (!ready_queue) {
        ready_queue = queue_create();
        if (!ready_queue) {
            ppos_panic("Erro ao alocar a fila de tarefas porntas.\n");
            return;
        }
    }

    user_tasks = 0;
}

void dispatcher_term () {
    if (!ready_queue) {
        ppos_warn("Fila de prontas ja liberada, pulando sua liberacao.\n");
        return;
    }

    // A funcao principal do dispatcher tem que ter destruido todas as tarefas
    if (user_tasks > 0) {
        ppos_panic("Nem todas as tarefas terminaram antes do fim do dispatcher.\n");
        return;
    }

    queue_destroy(ready_queue);
    ready_queue = NULL;
}

void dispatcher () {
    ppos_debug("dispatcher started\n");

    struct task_t * user_main_task = task_create("user_main", user_main, NULL);
    if (!user_main_task) {
        ppos_panic("Falha ao alocar o user_main.\n");
        return;
    }

    while (user_tasks > 0) {
        struct task_t * next_task = scheduler(ready_queue);

        if (next_task) {
            task_run(next_task);

            switch (next_task->status) {
                case READY:
                    // Tarefa ja reinserida na fila de prontas
                    break;
                
                case RUNNING:
                    // Caso impossivel
                    ppos_warn("Estado impossivel de uma tarefa ao retornar ao dispatcher.\n");
                    break;
                
                case SUSPENDED:
                    // Tarefa ja na fila de espera
                    break;

                case TERMINATED:
                    // Task user main sera terminada e destruida aqui
                    task_destroy(next_task);
                    user_tasks--;
                    break;
                
                default:
                    break;
            }
        }
        else ppos_warn("Escalonador nao escolheu uma tarefa.\n");
    }

    ppos_debug("dispatcher stopping, no more user tasks\n");
}

// Nao administra os status das tarefas
int task_switch (struct task_t * task) {
    if (!current_task) return ERROR;

    // Devolve a CPU para quem criou a tarefa atual
    if (!task) task = current_task->parent;
    if (!task) return ERROR;

    // Confere se ja terminou de executar
    if (task->status == TERMINATED) return NOERROR;

    // Switch para si mesma
    if (task == current_task) return NOERROR;

    struct task_t * prev = current_task;

    ppos_debug("task %d (%s) switch to task %d (%s)\n",
               prev->id, prev->name, task->id, task->name);

    current_task = task;

    if (ctx_switch(&prev->context, &task->context) == ERROR) {
        // Desfaz pois a troca nao ocorreu
        current_task = prev;
        return ERROR;
    }

    return NOERROR;
}


void task_run (struct task_t * task) {
    if (!task) {
        ppos_panic("Task selecionada para rodar nao existe.\n");
        return;
    }

    // Retira a tarefa da fila de prontas
    int status = queue_del(ready_queue, task);
    if (status == ERROR) {
        ppos_panic("Erro ao retirar a tarefa da fila de prontas.\n");
        return;
    }

    task->status = RUNNING;

    status = task_switch(task);
    if (status == ERROR) {
        ppos_panic("Erro ao colocar a tarefa para rodar.\n");
        return;
    }
}

void task_suspend (struct queue_t * queue) {
    if (!current_task) {
        ppos_panic("Erro ao acessar a tarefa atualmente rodando.\n");
        return;
    }

    current_task->status = SUSPENDED;

    int status;
    if (queue) {
        status = queue_add(queue, current_task);
        if (status == ERROR) {
            ppos_panic("Erro ao adicionar task suspensa a fila.\n");
            return;
        }
    }

    status = task_switch(kernel);
    if (status == ERROR) {
        ppos_panic("Erro na troca de contexto.\n");
        return;
    }
}

void task_awake (struct task_t * task) {
    if (!task) {
        ppos_panic("Erro ao acordar: tarefa nao existe.\n");
        return;
    }

    // Se a task estiver em alguma fila, remova-a
    // Isso tera que ser modificado no futuro
    if (queue_has(ready_queue, task)) queue_del(ready_queue, task);
    
    task->status = READY;

    int status = queue_add(ready_queue, task);
    if (status == ERROR) {
        ppos_panic("Erro ao adicionar task recem-acordada a fila.\n");
        return;
    }

    // A tarefa atual continua em execucao
}