// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

// Autor: Pietro Comin [GRR20241955]

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
#include "time.h"


// --- Variaveis Globais ---

struct queue_t * ready_queue;
struct queue_t * sleeping_tasks_queue;
int user_tasks;


// --- Funcoes/Variaveis Externas ---

extern struct task_t * current_task;
extern struct task_t * kernel;
extern void user_main (void * arg);
extern int preemption_enabled;


// --- Funcoes internas ---

// Atualiza os tempos da tarefa sendo interrompida e a sendo ativada.
// Retorna ERROR ou NOERROR.
static int task_update_time_values (struct task_t * start_task, struct task_t * finish_task) {
    if (!start_task || !finish_task) return ERROR;

    // Lida com a tarefa sendo iniciada
    start_task->time.cpu_activations++;
    start_task->time.current_exec_start_time = time();

    // Lida com a tarefa sendo interrompida
    unsigned int start_time = finish_task->time.current_exec_start_time;
    unsigned int exec_time = time() - start_time;
    finish_task->time.cpu_time += exec_time;
    finish_task->time.current_exec_start_time = 0;

    return NOERROR;
}

static void task_print_final_specs (struct task_t * task) {
    unsigned int task_total_time = time() - task->time.initial_time;
    printk("PPOS: task %3d (%s), %5u ms run, %5u ms cpu, %5u acts, exit code %3d\n", 
            task->id, task->name, 
            task_total_time, task->time.cpu_time, 
            task->time.cpu_activations, task->exit_code);
}

// Percorre a fila de adormecidas e acorda as tarefas cujo tempo de sleep terminou
static void wake_sleeping_tasks () {
    unsigned int now = time();

    struct task_t * task = queue_head(sleeping_tasks_queue);
    while (task) {
        if (task->time.awake_time <= now) {
            // queue_del ja avanca o iterador
            task_awake(task);
            task = queue_item(sleeping_tasks_queue);
        }
        else task = queue_next(sleeping_tasks_queue);
    }
}

// --- Funcoes da API ---

void dispatcher_init () {
    if (!ready_queue) {
        ready_queue = queue_create();
        if (!ready_queue) {
            ppos_panic("Erro ao alocar a fila de tarefas prontas.\n");
            return;
        }
    }

    if (!sleeping_tasks_queue) {
        sleeping_tasks_queue = queue_create();
        if (!sleeping_tasks_queue) {
            ppos_panic("Erro ao alocar a fila de tarefas adormecidas.\n");
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

    queue_destroy(sleeping_tasks_queue);
    sleeping_tasks_queue = NULL;

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
        // Acorda as tarefas que ja dormiram o tempo pedido
        wake_sleeping_tasks();

        // Todas as tarefas podem estar dormindo
        if (queue_size(ready_queue) == 0) continue;

        struct task_t * next_task = scheduler(ready_queue);

        if (next_task) {
            preemption_enabled = 1; // Garante que a prox tarefa rode corretamente
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
                    // As demais tarefas sao destruidas por quem as criou (task_destroy)
                    // Task user main sera terminada e destruida aqui
                    task_print_final_specs(next_task);
                    if (next_task == user_main_task) task_destroy(next_task);
                    user_tasks--;
                    break;
                
                default:
                    break;
            }
        }
        else ppos_panic("Escalonador nao escolheu uma tarefa.\n");
    }

    task_print_final_specs(kernel);

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

    if (task_update_time_values(task, prev) == ERROR) return ERROR;

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

    task->current_queue = NULL;

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

    current_task->current_queue = queue;

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

    // Se a task estiver em alguma fila, remove-a
    if (task->current_queue) {
        if (queue_del(task->current_queue, task) == ERROR) 
            ppos_panic("Erro ao remover a tarefa da fila atual.\n");

        task->current_queue = NULL;
    }
    
    task->status = READY;

    int status = queue_add(ready_queue, task);
    if (status == ERROR) {
        ppos_panic("Erro ao adicionar task recem-acordada a fila.\n");
        return;
    }

    task->current_queue = ready_queue;

    // A tarefa atual continua em execucao
}