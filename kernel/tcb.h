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
    NEW =        0,
    READY =      1,
    RUNNING =    2,
    SUSPENDED =  3,
    TERMINATED = 4
} Status;

// Estrutura de dados relativos a temporizacao
struct time_t {
    int initial_time;
    int current_exec_start_time;
    int cpu_time;
    int cpu_activations;
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
};

#endif
