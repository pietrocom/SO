// PingPongOS - PingPong Operating System

// Sistema produtor/consumidor com buffer limitado, usando semáforos.

// Autor: Pietro Comin [GRR20241955]

#include <assert.h>
#include "lib/pplibc.h"
#include "syscall.h"

#define NUM_PROD   3        // Numero de produtores
#define NUM_CONS   2        // Numero de consumidores
#define BUF_SIZE   5        // Capacidade do buffer
#define RUN_TIME   20000    // Tempo de execucao do sistema (ms)

// Buffer circular FIFO de inteiros
static int buffer[BUF_SIZE];
static int buf_head = 0;    // Posicao do item mais antigo
static int buf_count = 0;   // Numero de itens no buffer

// Semaforos
static int s_buffer;        // Exclusao mutua no acesso ao buffer
static int s_item;          // Itens disponiveis no buffer
static int s_vaga;          // Vagas livres no buffer

// Tarefas
static struct task_t *prod[NUM_PROD], *cons[NUM_CONS];

// Insere um item no final do buffer (deve haver vaga)
static void buffer_put(int item)
{
    buffer[(buf_head + buf_count) % BUF_SIZE] = item;
    buf_count++;
}

// Retira o item do inicio do buffer (deve haver item)
static int buffer_get()
{
    int item = buffer[buf_head];
    buf_head = (buf_head + 1) % BUF_SIZE;
    buf_count--;
    return item;
}

// Tarefas produtoras; as operacoes falham (ERROR) quando os
// semaforos sao destruidos, o que encerra a tarefa
void produtor(void *arg)
{
    char *name = arg;
    int item;

    while (1)
    {
        task_sleep(1000);
        item = randnum() % 100;

        if (sem_down(s_vaga) == ERROR)
            break;

        if (sem_down(s_buffer) == ERROR)
            break;
        buffer_put(item);
        printk("%s produziu %d\n", name, item);
        if (sem_up(s_buffer) == ERROR)
            break;

        if (sem_up(s_item) == ERROR)
            break;
    }

    task_exit(NOERROR);
}

// Tarefas consumidoras
void consumidor(void *arg)
{
    char *name = arg;
    int item;

    while (1)
    {
        if (sem_down(s_item) == ERROR)
            break;

        if (sem_down(s_buffer) == ERROR)
            break;
        item = buffer_get();
        printk("                             %s consumiu %d\n", name, item);
        if (sem_up(s_buffer) == ERROR)
            break;

        if (sem_up(s_vaga) == ERROR)
            break;

        task_sleep(1000);
    }

    task_exit(NOERROR);
}

// Tarefa principal
void user_main(void *arg)
{
    int status;

    printk("%5u ms: %s inicio\n", time(), task_name(NULL));

    // Inicia os semaforos
    s_buffer = sem_create(1);
    assert(s_buffer >= 0);
    s_item = sem_create(0);
    assert(s_item >= 0);
    s_vaga = sem_create(BUF_SIZE);
    assert(s_vaga >= 0);

    // Cria produtores e consumidores
    prod[0] = task_create("p1", produtor, "p1");
    prod[1] = task_create("p2", produtor, "p2");
    prod[2] = task_create("p3", produtor, "p3");
    cons[0] = task_create("c1", consumidor, "c1");
    cons[1] = task_create("c2", consumidor, "c2");
    for (int i = 0; i < NUM_PROD; i++)
        assert(prod[i]);
    for (int i = 0; i < NUM_CONS; i++)
        assert(cons[i]);

    // Deixa o sistema executar por algum tempo
    task_sleep(RUN_TIME);

    // Destroi os semaforos, o que libera as tarefas bloqueadas com erro
    printk("%5u ms: %s destroi os semaforos\n", time(), task_name(NULL));
    status = sem_destroy(s_buffer);
    assert(status == NOERROR);
    status = sem_destroy(s_item);
    assert(status == NOERROR);
    status = sem_destroy(s_vaga);
    assert(status == NOERROR);

    // Aguarda o encerramento e destroi as tarefas
    for (int i = 0; i < NUM_PROD; i++)
    {
        task_wait(prod[i]);
        status = task_destroy(prod[i]);
        assert(status == NOERROR);
    }
    for (int i = 0; i < NUM_CONS; i++)
    {
        task_wait(cons[i]);
        status = task_destroy(cons[i]);
        assert(status == NOERROR);
    }

    printk("%5u ms: %s fim\n", time(), task_name(NULL));
    task_exit(NOERROR);
}
