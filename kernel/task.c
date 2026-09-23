// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.


// --- Includes ---

#include "tcb.h"
#include "task.h"
#include "ctx.h"
#include "memory.h"
#include "macros.h"
#include "queue.h"
#include "dispatcher.h"
#include "time.h"


// --- Variaveis Globais ---

static int ids;                // Contador usado para setar os IDs das tasks
struct task_t * current_task;  // Task que esta sendo executada
struct task_t * kernel;        // Task inicial do kernel


// --- Variaveis Externas ---

extern struct queue_t * ready_queue;
extern int user_tasks;


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
    kernel->current_queue = NULL;
    kernel->static_prio  = 0;
    kernel->dynamic_prio = 0;

    // Tempo nao pode ser registrado pois time_init() ainda nao foi executado
    kernel->time.cpu_activations = 0;
    kernel->time.cpu_time = 0;
    kernel->time.current_exec_start_time = 0;
    kernel->time.initial_time = 0;

    current_task = kernel;

    ppos_debug("subsystem task initiated\n");
}

void task_term () {
    if (kernel) mem_free(kernel);
    kernel = NULL;
}

struct task_t * task_create (char * name, void (* entry)(void *), void * arg) {
    if (!entry) return NULL;

    if (!ready_queue) {
        ppos_panic("Erro ao inserir tarefa na fila de prontas: fila inacessivel.\n");
        return NULL;
    }

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

    task->time.cpu_activations = 0;
    task->time.cpu_time = 0;
    task->time.current_exec_start_time = 0;
    task->time.initial_time = time();

    if (ctx_create(&task->context, entry, arg, stack_pointer, STACK_SIZE) == -1) {
        mem_free(task);
        mem_free(stack_pointer);
        
        return NULL;
    }
    task->status = READY;

    ppos_debug("task %d (%s) create task %d (%s)\n",
               current_task->id, current_task->name, task->id, task->name);

    if (queue_add(ready_queue, task) == ERROR) {
        mem_free(task);
        mem_free(stack_pointer);

        ppos_panic("Erro ao inserir tarefa na fila de prontas: queue_add retornou com erro.\n");
        return NULL;
    }

    task->current_queue = ready_queue;

    user_tasks++;

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

void task_yield () {
    if (!current_task) {
        ppos_panic("Nenhuma tarefa esta rodando.\n");
        return;
    }

    current_task->status = READY;

    // Coloca a tarefa atual no fim da lista de prontas
    int status = queue_add(ready_queue, current_task);
    if (status == ERROR) {
        ppos_panic("Erro ao adicionar tarefa a fila.\n");
        return;
    }

    current_task->current_queue = ready_queue;

    // Volta para o dispatcher
    task_switch(kernel);
}

int task_wait (struct task_t * task) { return ERROR; }

void task_sleep (int t) {}

void task_exit (int exit_code) {
    if (!current_task) {
        ppos_panic("Erro ao terminar uma tarefa: nenhuma esta rodando.\n");
        return;
    }

    current_task->status = TERMINATED;

    // Volta ao dispatcher
    task_switch(kernel);
}
