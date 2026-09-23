// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica do tempo.

#include "time.h"
#include "hardware/cpu.h"
#include "tcb.h"
#include "task.h"

// Variaveis globais

static unsigned int cur_time;   // Current time
extern struct task_t * current_task;


// Funcoes estaticas

static void handle_time (int irq) {
    cur_time += TICK;

    if (current_task->type == KERNEL) return;

    current_task->quantum -= TICK;
    // Se o quantum da tarefa encerrou
    if (current_task->quantum <= 0) {
        current_task->quantum = QUANTUM;
        task_yield();
    }
}


// Funcoes da API

void time_init() {
    cur_time = 0;

    // IRQs devem estar habilitadas
    hw_irq_handle(IRQ_TIMER, handle_time);

    hw_timer(TICK, TICK);  // Aumenta o tempo a cada tick
}

void time_term() {
    hw_timer(0, 0);     // Desarma o timer
}

unsigned int time()
{
    return (cur_time);
}

