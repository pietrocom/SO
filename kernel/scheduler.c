// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Escalonador de tarefas prontas.

// --- Includes ---

#include <stdlib.h>
#include "scheduler.h"
#include "queue.h"
#include "memory.h"
#include "macros.h"
#include "tcb.h"


// --- Constantes ---

#define ALPHA 1


// --- Variaveis Globais ---

extern struct task_t * current_task;


// --- Funcoes da API ---

void sched_init () {
}

void sched_term () {
}

struct task_t * scheduler (struct queue_t * ready_queue) {
    if (!ready_queue) {
        ppos_panic("Erro no acesso a fila de prontas no scheduler.\n");
        return NULL;
    }

    return queue_head(ready_queue);
}

void sched_setprio (struct task_t * task, int prio) {
    if (prio > 20 || prio < -20) {
        ppos_panic("Prioridade da tarefa nao atende ao intervalo especificado.\n");
        return;
    }

    if (!task) {
        current_task->static_prio = prio;
        current_task->dynamic_prio = prio;
        return;
    }
    task->static_prio = prio;
    task->dynamic_prio = prio;
}

int sched_getprio (struct task_t * task) {
    if (!task) return current_task->static_prio;

    return task->static_prio;
}

