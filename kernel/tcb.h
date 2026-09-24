// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#define STACK_SIZE (32 * 1024)

#include "ctx.h"
#include "queue.h"

typedef enum {
    NEW,
    READY,
    RUNNING,
    SUSPENDED,
    TERMINATED
} Status;

typedef enum {
    USER,
    KERNEL
} Type;

// Estrutura de dados relativos a temporizacao
struct time_t {
    unsigned int initial_time;
    unsigned int current_exec_start_time;
    unsigned int cpu_time;
    unsigned int cpu_activations;
};

// Task Control Block (TCB), infos sobre uma tarefa
struct task_t
{
    int id;                 // identificador da tarefa
    char * name;            // nome da tarefa
    struct ctx_t context;   // contexto da tarefa
    Status status;          // pronta, executando, ...
    void * stack_pointer;   // facilita o acesso da stack do ctx
    struct task_t * parent; // ponteiro para a task que estava executando na hora de sua criacao
    int static_prio;        // prioridade estatica
    int dynamic_prio;       // prioridade dinamica
    struct queue_t * current_queue; // fila onde esta atualmente ou NULL    
    struct time_t time;     // cuida dos dados de tempo
    Type type;              // tipo da task (sistema, usuario, etc)
    int quantum;            // garante o tempo maximo de execucao da tarefa
    int exit_code;          // codigo de saida da tarefa
};

#endif
