// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência de filas de mensagens

// Autor: Pietro Comin [GRR20241955]


// --- Includes ---

#include "mqueue.h"
#include "tcb.h"
#include "macros.h"
#include "memory.h"
#include "queue.h"
#include "map.h"
#include "dispatcher.h"
#include "semaphore.h"


// --- Constantes ---

#define MQUEUE_MAX 1024     // numero maximo de filas simultaneas


// --- Estruturas ---

// Fila de mensagens: buffer circular de max_msgs mensagens de msg_size bytes
struct mqueue_t {
    int id;                     // UID da fila
    int max_msgs;               // capacidade (em mensagens)
    int msg_size;               // tamanho de cada mensagem (em bytes)
    char * buffer;              // max_msgs * msg_size bytes
    int head;                   // indice da mensagem mais antiga
    int count;                  // mensagens armazenadas
    struct queue_t * senders;   // tarefas bloqueadas em send (fila cheia)
    struct queue_t * receivers; // tarefas bloqueadas em recv (fila vazia)
    int lock;                   // spinlock que protege esta fila
};


// --- Variaveis Globais ---

static struct map_t * mq_map;   // mapa UID -> fila de mensagens
static int map_lock;            // spinlock que protege o mapa


// --- Variaveis Externas ---

extern struct task_t * current_task;


// --- Funcoes internas ---

// Busca a fila pelo UID e a devolve ja travada, ou NULL se nao existir.
// O mapa fica travado ate a fila ser travada
static struct mqueue_t * mq_get_locked (int mqueue_id) {
    if (!mq_map) return NULL;

    spin_lock(&map_lock);
    struct mqueue_t * mq = map_get(mq_map, mqueue_id);
    if (mq) spin_lock(&mq->lock);
    spin_unlock(&map_lock);

    return mq;
}

// Endereco da mensagem de numero i a partir da cabeca do buffer circular
static char * mq_slot (struct mqueue_t * mq, int i) {
    return mq->buffer + ((mq->head + i) % mq->max_msgs) * mq->msg_size;
}

// Copia a mensagem para o final do buffer (deve haver espaco)
static void mq_push (struct mqueue_t * mq, void * msg) {
    mem_copy(msg, mq_slot(mq, mq->count), mq->msg_size);
    mq->count++;
}

// Copia a mensagem do inicio do buffer para msg e a retira (nao vazio)
static void mq_pop (struct mqueue_t * mq, void * msg) {
    mem_copy(mq_slot(mq, 0), msg, mq->msg_size);
    mq->head = (mq->head + 1) % mq->max_msgs;
    mq->count--;
}

// Acorda uma tarefa bloqueada (task_awake a retira da fila de espera),
// informando o resultado da sua operacao
static void mq_wake (struct task_t * task, int result) {
    task->wait_result = result;
    task->wait_msg = NULL;
    task_awake(task);
}

// Bloqueia a tarefa atual na fila de espera indicada, liberando a trava da
// fila de mensagens. O buffer msg fica no TCB para que quem acordar a tarefa
// conclua a copia por ela. Retorna o resultado informado por quem a acordar.
static int mq_block (struct mqueue_t * mq, struct queue_t * waiters, void * msg) {
    current_task->wait_msg = msg;
    current_task->status = SUSPENDED;
    current_task->wait_result = NOERROR;
    spin_unlock(&mq->lock);

    task_suspend(waiters);

    // A fila pode ter sido destruida, so o resultado da espera eh valido
    return current_task->wait_result;
}

// Libera os recursos de uma fila (sem tarefas esperando)
static void mq_free (struct mqueue_t * mq) {
    if (mq->senders)   queue_destroy(mq->senders);
    if (mq->receivers) queue_destroy(mq->receivers);
    if (mq->buffer)    mem_free(mq->buffer);
    mem_free(mq);
}


// --- Funcoes da API ---

void mqueue_init () {
    map_lock = 0;

    mq_map = map_create(MQUEUE_MAX);
    if (!mq_map) {
        ppos_panic("Erro ao alocar o mapa de filas de mensagens.\n");
        return;
    }

    ppos_debug("subsystem mqueue initiated\n");
}

void mqueue_term () {
    if (!mq_map) return;

    // Libera filas que a aplicacao nao destruiu
    for (int id = 0; id < map_size(mq_map); id++) {
        struct mqueue_t * mq = map_del(mq_map, id);
        if (mq) {
            ppos_warn("Fila de mensagens %d nao foi destruida pela aplicacao.\n", id);
            mq_free(mq);
        }
    }

    map_destroy(mq_map);
    mq_map = NULL;
}

int mqueue_create (int max_msgs, int msg_size) {
    check_parm(max_msgs <= 0, "capacidade deve ser positiva", ERROR);
    check_parm(msg_size <= 0, "tamanho de mensagem deve ser positivo", ERROR);
    check_parm(!mq_map, "subsistema de filas de mensagens nao iniciado", ERROR);

    struct mqueue_t * mq = mem_alloc(sizeof(struct mqueue_t));
    if (!mq) return ERROR;

    mq->max_msgs  = max_msgs;
    mq->msg_size  = msg_size;
    mq->head      = 0;
    mq->count     = 0;
    mq->lock      = 0;
    mq->buffer    = mem_alloc(max_msgs * msg_size);
    mq->senders   = queue_create();
    mq->receivers = queue_create();

    if (!mq->buffer || !mq->senders || !mq->receivers) {
        mq_free(mq);
        return ERROR;
    }

    spin_lock(&map_lock);
    mq->id = map_put(mq_map, mq);
    spin_unlock(&map_lock);

    if (mq->id < 0) {
        mq_free(mq);
        return ERROR;
    }

    ppos_debug("task %d create mqueue %d (%d x %d bytes)\n",
               current_task->id, mq->id, max_msgs, msg_size);

    return mq->id;
}

int mqueue_destroy (int mqueue_id) {
    if (!mq_map) return ERROR;

    // Retira a fila do map, a partir dai nenhuma tarefa consegue acessa-la
    spin_lock(&map_lock);
    struct mqueue_t * mq = map_del(mq_map, mqueue_id);
    if (mq) spin_lock(&mq->lock);
    spin_unlock(&map_lock);

    if (!mq) return ERROR;

    ppos_debug("task %d destroy mqueue %d\n", current_task->id, mqueue_id);

    // Acorda todas as tarefas bloqueadas, com erro
    struct task_t * task;
    while ((task = queue_head(mq->senders)) != NULL)
        mq_wake(task, ERROR);
    while ((task = queue_head(mq->receivers)) != NULL)
        mq_wake(task, ERROR);

    spin_unlock(&mq->lock);
    mq_free(mq);

    return NOERROR;
}

int mqueue_send (int mqueue_id, void * msg) {
    if (!msg) return ERROR;

    struct mqueue_t * mq = mq_get_locked(mqueue_id);
    if (!mq) return ERROR;

    // Ha receptor esperando (logo, buffer vazio), entrega direto a ele
    struct task_t * task = queue_head(mq->receivers);
    if (task) {
        mem_copy(msg, task->wait_msg, mq->msg_size);
        mq_wake(task, NOERROR);
        spin_unlock(&mq->lock);
        return NOERROR;
    }

    // Ha espaco no buffer
    if (mq->count < mq->max_msgs) {
        mq_push(mq, msg);
        spin_unlock(&mq->lock);
        return NOERROR;
    }

    // Fila cheia, espera um receptor retirar uma mensagem e depositar
    // esta no buffer (em ordem FIFO entre os emissores)
    return mq_block(mq, mq->senders, msg);
}

int mqueue_recv (int mqueue_id, void * msg) {
    if (!msg) return ERROR;

    struct mqueue_t * mq = mq_get_locked(mqueue_id);
    if (!mq) return ERROR;

    // Ha mensagem no buffer
    if (mq->count > 0) {
        mq_pop(mq, msg);

        // Abriu uma vaga, deposita a mensagem do primeiro emissor bloqueado
        struct task_t * task = queue_head(mq->senders);
        if (task) {
            mq_push(mq, task->wait_msg);
            mq_wake(task, NOERROR);
        }

        spin_unlock(&mq->lock);
        return NOERROR;
    }

    // Fila vazia
    return mq_block(mq, mq->receivers, msg);
}

int mqueue_msgs (int mqueue_id) {
    struct mqueue_t * mq = mq_get_locked(mqueue_id);
    if (!mq) return ERROR;

    int count = mq->count;
    spin_unlock(&mq->lock);

    return count;
}
