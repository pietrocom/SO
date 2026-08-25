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


// --- Funcoes/Variaveis Externas ---

extern struct task_t * current_task;
extern struct task_t * kernel;
extern void user_main (void * arg);


// --- Funcoes da API ---

void dispatcher_init () {}

void dispatcher_term () {}

void dispatcher () {
    struct task_t * usr_main_task = task_create("user_main", user_main, NULL);
    if (!usr_main_task) {
        ppos_panic("Falha ao alocar o user_main.");
        return;
    }

    task_switch(usr_main_task);

    task_destroy(usr_main_task);
}

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

    ppos_debug("Task %d (%s) switch to task %d (%s)\n",
               prev->id, prev->name, task->id, task->name);

    prev->status = SUSPENDED;
    task->status = RUNNING;
    current_task = task;

    if (ctx_switch(&prev->context, &task->context) == ERROR) {
        // Desfaz pois a troca nao ocorreu
        current_task = prev;
        prev->status = RUNNING;
        return ERROR;
    }

    return NOERROR;
}


void task_run (struct task_t * task) {}
void task_suspend (struct queue_t * queue) {}
void task_awake (struct task_t * task) {}