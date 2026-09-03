// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.


// --- Includes ---

#include "tcb.h"
#include "task.h"
#include "ctx.h"
#include "memory.h"
#include "macros.h"


// --- Variaveis Globais ---

static int ids;                // Contador usado para setar os IDs das tasks
struct task_t * current_task;  // Task que esta sendo executada
struct task_t * kernel;        // Task inicial do kernel


// --- Funcoes da API ---

void task_init () {
    // Inicializacoes das variaveis globais desse arquivo
    ids = 0;

    kernel = mem_alloc(sizeof(struct task_t));
    if (!kernel) {
        ppos_panic("Alocacao do kernel falhou.\n");
        return;
    }
    kernel->id = ids; ids++;
    kernel->name = "kernel";
    kernel->status = READY;
    kernel->parent = NULL;
    kernel->stack_pointer = NULL;

    current_task = kernel;

    ppos_debug("subsystem task initiated\n");
}

void task_term () {
    if (kernel) mem_free(kernel);
}

struct task_t * task_create (char * name, void (* entry)(void *), void * arg) {
    if (!entry) return NULL;

    struct task_t * task = mem_alloc(sizeof(struct task_t));
    if (!task) return NULL;

    void * stack_pointer = mem_alloc(STACK_SIZE);
    if (!stack_pointer) {
        mem_free(task);

        return NULL;
    }

    task->static_prio  = 0; // Prioridade default
    task->dynamic_prio = 0; // Setado como igual a prioridade estatica
    task->parent = current_task;
    task->stack_pointer = stack_pointer;
    task->name   = name;
    task->status = NEW;
    task->id     = ids; ids++;

    if (ctx_create(&task->context, entry, arg, stack_pointer, STACK_SIZE) == -1) {
        mem_free(task);
        mem_free(stack_pointer);
        
        return NULL;
    }
    task->status = READY;

    ppos_debug("task %d (%s) create task %d (%s)\n",
               current_task->id, current_task->name, task->id, task->name);

    return task;
}

int task_destroy (struct task_t * task) {
    // Cuida para nao destruir uma tarefa em execucao
    if (!task || task == current_task) return ERROR;

    ppos_debug("task %d (%s) destroy task %d (%s)\n",
               current_task->id, current_task->name, task->id, task->name);

    if (task->stack_pointer) mem_free(task->stack_pointer);
    mem_free(task);

    return NOERROR;
}

int task_id (struct task_t * task) {
    if (!task) {
        if (!current_task) {
            ppos_warn("Nenhuma task disponivel para retorno do ID.\n");
            return ERROR;
        }
        return current_task->id;
    }

    return task->id;
}

char * task_name (struct task_t * task) {
    if (!task) {
        if (!current_task) {
            ppos_warn("Nenhuma task disponivel para retorno do nome.\n");
            return NULL;
        }
        return current_task->name;
    }

    return task->name;
}

void task_yield () {}
int task_wait (struct task_t * task) { return ERROR; }
void task_sleep (int t) {}
void task_exit (int exit_code) {}
