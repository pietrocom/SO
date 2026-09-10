// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Escalonador de tarefas prontas.

// --- Includes ---

#include "scheduler.h"
#include "queue.h"
#include "memory.h"
#include "macros.h"
#include "tcb.h"


// --- Constantes ---

#define ALPHA (-1)


// --- Variaveis Globais ---

extern struct task_t * current_task;


// --- Funcoes da API ---

// Nada a fazer: o escalonador nao possui estruturas proprias
// Prioridades ficam no TCB e a fila de prontas pertence ao dispatcher
void sched_init () {
}

void sched_term () {
}

// Aumenta a prioridade dinamica da tarefa
static void increase_prio (struct task_t * task) {
    if (task->dynamic_prio > -20) task->dynamic_prio += ALPHA;
}

struct task_t * scheduler (struct queue_t * ready_queue) {
    if (!ready_queue) {
        ppos_panic("Erro no acesso a fila de prontas no scheduler.\n");
        return NULL;
    }

    struct task_t * selected_task = queue_head(ready_queue);
    if (!selected_task) {
        ppos_warn("Erro ao selecionar proxima tarefa: fila vazia.\n");
        return NULL;
    }

    // Percorre a fila e retorna a primeira tarefa da fila com maior prioridade
    // Aumenta a prioridade das tarefas nao selecionadas
    int best_prio = selected_task->dynamic_prio;
    for (struct task_t * task_it = queue_next(ready_queue); task_it; task_it = queue_next(ready_queue)) {
        // Caso haja uma com mais prioridade
        if (best_prio > task_it->dynamic_prio) {
            increase_prio(selected_task);
            selected_task = task_it;
            best_prio = task_it->dynamic_prio;
        }
        else increase_prio(task_it);
    }

    // Reseta a prioridade dinamica da tarefa selecionada
    selected_task->dynamic_prio = selected_task->static_prio;

    return selected_task;
}

void sched_setprio (struct task_t * task, int prio) {
    check_parm(prio > 20 || prio < -20, "Prioridade fora do intervalo [-20,+20]", );

    if (!task) {
        if (!current_task) {
            ppos_panic("Erro ao acessar a tarefa atualmente rodando.\n");
            return;
        }
        current_task->static_prio = prio;
        current_task->dynamic_prio = prio;
        return;
    }
    task->static_prio = prio;
    task->dynamic_prio = prio;
}

int sched_getprio (struct task_t * task) {
    if (!task) {
        if (!current_task) {
            ppos_panic("Erro ao acessar a tarefa atualmente rodando.\n");
            return ERROR;
        }
        return current_task->static_prio;
    }

    return task->static_prio;
}

