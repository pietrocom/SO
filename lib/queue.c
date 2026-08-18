// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD fila genérica

#include <stdio.h>
#include "queue.h"

struct node_t {
    struct node_t * next;
    void * item;
};

struct queue_t {
    struct node_t * first;
    struct node_t * last;
    struct node_t * iterator;
};

struct queue_t * queue_create () {
    struct queue_t * queue = malloc(sizeof(struct queue_t *));
    if (!queue) return;

    queue->first = NULL;
    queue->last = NULL;
    queue->iterator = NULL;

    return queue;
}

int queue_destroy (struct queue_t * queue) {
    if (!queue) return ERROR;

    while (queue->first != NULL) {
        struct node_t * node = queue->first;
        queue->first = node->next;
        node = NULL;
        free(node);
    }

    queue = NULL;
    free(queue);

    return NOERROR;
}

int queue_add (struct queue_t * queue, void * item) {
    if (!queue || !item) return ERROR;

    struct node_t * node = malloc(sizeof(struct node_t *));
    node->item = item;
    node->next = NULL;

    // Caso a fila esteja vazia
    if (queue->first == NULL) {
        queue->first = node;
        queue->last = node;
        queue->iterator = node;
    }
    else {
        queue->last->next = node;
        queue->last = node;
    }

    return NOERROR;
}

int queue_del (struct queue_t * queue, void * item) {
    if (!queue || !item) return ERROR;


}
