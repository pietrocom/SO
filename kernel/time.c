// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica do tempo.

#include "time.h"
#include "hardware/cpu.h"

// Variaveis globais

static unsigned int cur_time;   // Current time


// Funcoes estaticas

static void handle_time() {
    cur_time++;
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

