// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.


// --- Includes ---

#include "tcb.h"
#include "task.h"
#include "ctx.h"
#include "memory.h"
#include "queue.h"
#include "macros.h"


// --- Variaveis Globais ---

static int ids;
struct task_t * current_task;  // Task que esta sendo executada
struct queue_t * task_queue;


// --- Funcoes da API ---

void task_init () {
    // Inicializacoes
    ids = 0;
    current_task = NULL;
    task_queue = queue_create();
    if (!task_queue) {
        ppos_panic("Nao foi possivel criar a fila de tarefas.\n");
        return;
    }
}

void task_term () {
    if (!task_queue) {
        ppos_warn("Fila de tasks nao pode ser liberada: endereco invalido.");
        return;
    }

    while (queue_head(task_queue) != NULL) {
        void * item = queue_head(task_queue);
        queue_del(task_queue, queue_head(task_queue));
        mem_free(((struct task_t *)item)->stack_pointer);
        mem_free(item);
    }
    queue_destroy(task_queue);
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

    queue_add(task_queue, task);

    return task;
}