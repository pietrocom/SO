// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Semáforos e spinlocks


// --- Includes ---

#include "semaphore.h"
#include "tcb.h"
#include "macros.h"
#include "memory.h"
#include "queue.h"
#include "map.h"
#include "dispatcher.h"


// --- Constantes ---

#define SEM_MAX 1024    // numero maximo de semaforos simultaneos


// --- Estruturas ---

struct semaphore_t {
    int id;                 // UID do semaforo
    int counter;            // contador (negativo: no. de tarefas esperando)
    struct queue_t * queue; // tarefas suspensas aguardando o semaforo
    int lock;               // spinlock que protege este semaforo
};


// --- Variaveis Globais ---

static struct map_t * sem_map;  // mapa UID -> semaforo
static int map_lock;            // spinlock que protege o mapa


// --- Variaveis Externas ---

extern struct task_t * current_task;


// --- Funcoes internas ---

// Busca o semaforo pelo UID e o devolve ja travado, ou NULL se nao existir.
// O mapa permanece travado durante a busca, entao o semaforo nao pode ser
// destruido entre a busca e a aquisicao da sua trava (ordem: mapa -> sem).
static struct semaphore_t * sem_get_locked (int sem_id) {
    if (!sem_map) return NULL;

    spin_lock(&map_lock);
    struct semaphore_t * sem = map_get(sem_map, sem_id);
    if (sem) spin_lock(&sem->lock);
    spin_unlock(&map_lock);

    return sem;
}

// Libera os recursos de um semaforo (que nao pode ter tarefas na fila)
static void sem_free (struct semaphore_t * sem) {
    queue_destroy(sem->queue);
    mem_free(sem);
}


// --- Funcoes da API ---

void spin_lock (int * lock) {
    if (!lock) return;

    // test-and-test-and-set: so tenta a troca atomica quando a trava
    // parece livre, evitando escritas desnecessarias durante a espera
    while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE))
        while (__atomic_load_n(lock, __ATOMIC_RELAXED))
            ;

    // Enquanto mantiver a trava, a tarefa nao sofre preempcao por tempo
    if (current_task) current_task->lock_count++;
}

void spin_unlock (int * lock) {
    if (!lock) return;

    __atomic_store_n(lock, 0, __ATOMIC_RELEASE);

    if (current_task && current_task->lock_count > 0)
        current_task->lock_count--;
}

void sem_init () {
    map_lock = 0;

    sem_map = map_create(SEM_MAX);
    if (!sem_map) {
        ppos_panic("Erro ao alocar o mapa de semaforos.\n");
        return;
    }

    ppos_debug("subsystem semaphore initiated\n");
}

void sem_term () {
    if (!sem_map) return;

    // Libera semaforos que a aplicacao nao destruiu
    for (int id = 0; id < map_size(sem_map); id++) {
        struct semaphore_t * sem = map_del(sem_map, id);
        if (sem) {
            ppos_warn("Semaforo %d nao foi destruido pela aplicacao.\n", id);
            sem_free(sem);
        }
    }

    map_destroy(sem_map);
    sem_map = NULL;
}

int sem_create (int value) {
    check_parm(value < 0, "valor inicial negativo", ERROR);
    check_parm(!sem_map, "subsistema de semaforos nao iniciado", ERROR);

    struct semaphore_t * sem = mem_alloc(sizeof(struct semaphore_t));
    if (!sem) return ERROR;

    sem->queue = queue_create();
    if (!sem->queue) {
        mem_free(sem);
        return ERROR;
    }
    sem->counter = value;
    sem->lock = 0;

    spin_lock(&map_lock);
    sem->id = map_put(sem_map, sem);
    spin_unlock(&map_lock);

    if (sem->id < 0) {
        sem_free(sem);
        return ERROR;
    }

    ppos_debug("task %d create semaphore %d (value %d)\n",
               current_task->id, sem->id, value);

    return sem->id;
}

int sem_destroy (int sem_id) {
    if (!sem_map) return ERROR;

    // Trava mapa e semaforo (mesma ordem de sem_get_locked) e retira o
    // semaforo do mapa: a partir dai nenhuma outra tarefa consegue acessa-lo
    spin_lock(&map_lock);
    struct semaphore_t * sem = map_del(sem_map, sem_id);
    if (sem) spin_lock(&sem->lock);
    spin_unlock(&map_lock);

    if (!sem) return ERROR;

    ppos_debug("task %d destroy semaphore %d\n", current_task->id, sem_id);

    // Acorda todas as tarefas que aguardavam, sinalizando erro no sem_down
    struct task_t * task;
    while ((task = queue_head(sem->queue)) != NULL) {
        task->wait_result = ERROR;
        task_awake(task);   // retira da fila do semaforo
    }

    spin_unlock(&sem->lock);
    sem_free(sem);

    return NOERROR;
}

int sem_down (int sem_id) {
    struct semaphore_t * sem = sem_get_locked(sem_id);
    if (!sem) return ERROR;

    sem->counter--;
    if (sem->counter >= 0) {
        spin_unlock(&sem->lock);
        return NOERROR;
    }

    // Precisa esperar. O status muda para SUSPENDED antes de soltar a trava:
    // assim o temporizador nao preempta a tarefa entre o unlock e a entrada
    // na fila, o que poderia fazer um sem_up "perder" o despertar.
    current_task->status = SUSPENDED;
    current_task->wait_result = NOERROR;
    spin_unlock(&sem->lock);

    task_suspend(sem->queue);

    // Acordada por sem_up (NOERROR) ou sem_destroy (ERROR); o semaforo
    // pode nao existir mais, entao nao deve ser acessado aqui
    return current_task->wait_result;
}

int sem_up (int sem_id) {
    struct semaphore_t * sem = sem_get_locked(sem_id);
    if (!sem) return ERROR;

    sem->counter++;
    if (sem->counter <= 0) {
        // Ha tarefas esperando: acorda a primeira da fila (FIFO)
        struct task_t * task = queue_head(sem->queue);
        if (task) {
            task->wait_result = NOERROR;
            task_awake(task);
        }
        else ppos_warn("Semaforo %d com contador negativo e fila vazia.\n", sem_id);
    }

    spin_unlock(&sem->lock);

    return NOERROR;
}
