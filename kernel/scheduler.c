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
struct queue_t * queue;


// --- Funcoes da API ---

void sched_init () {
    queue = queue_create();
    if (!queue) {
        ppos_panic("Erro ao alocar a fila.\n");
        return;
    }
}

void sched_term () {
    if (!queue) {
        ppos_warn("Fila ja foi desalocada.\n");
        return;
    }

    // Percorre a fila desalocando os itens ainda contidos nela
    for (struct task_t * task = queue_head(queue); task; task = queue_next(queue)) {
        free(task);
    }

    queue_destroy(queue);
    queue = NULL;
}

struct task_t * scheduler (struct queue_t * ready_queue) {
    
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

