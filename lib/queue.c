// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD fila genérica

#include <stdio.h>
#include <stdlib.h>
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
    struct queue_t * queue = malloc(sizeof(struct queue_t));
    if (!queue) return NULL;

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
        free(node);
    }

    free(queue);

    return NOERROR;
}

int queue_add (struct queue_t * queue, void * item) {
    if (!queue || !item) return ERROR;

    struct node_t * node = malloc(sizeof(struct node_t));
    if (!node) return ERROR;
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

    if (queue->first == NULL) return ERROR;

    struct node_t * node = queue->first;
    // Caso seja o primeiro da fila
    if (node->item == item) {
        queue->first = node->next;
        if (queue->last == node)     queue->last = NULL;
        if (queue->iterator == node) queue->iterator = node->next;
        free(node);
        return NOERROR;
    }
    while (node->next != NULL) {
        if (node->next->item == item) {
            // Caso seja o ultimo da fila
            if (node->next == queue->last) {
                queue->last = node;
                if (queue->iterator == node->next) queue->iterator = NULL;
                free(node->next);
                node->next = NULL;
            }
            // Caso seja um nodo do meio da fila
            else {
                struct node_t * aux = node->next;
                if (queue->iterator == aux) queue->iterator = aux->next;
                node->next = aux->next;
                free(aux);
            }
            return NOERROR;
        }
        node = node->next;
    }

    return ERROR;
}

bool queue_has (struct queue_t * queue, void * item) {
    if (!queue || !item) return false;

    struct node_t * node = queue->first;
    while (node != NULL) {
        if (node->item == item) return true;
        node = node->next;
    }
    return false;
}

int queue_size (struct queue_t * queue) {
    if (!queue) return ERROR;

    int size = 0;
    struct node_t * node = queue->first;
    while (node != NULL) {
        size++;
        node = node->next;
    }

    return size;
}

void * queue_head (struct queue_t * queue) {
    if (!queue)               return NULL;
    if (queue->first == NULL) return NULL;

    queue->iterator = queue->first;
    return queue->iterator->item;
}

void * queue_next (struct queue_t * queue) {
    if (!queue)                  return NULL;
    if (queue->first == NULL)    return NULL;
    if (queue->iterator == NULL) return NULL;

    queue->iterator = queue->iterator->next;
    if (queue->iterator == NULL) return NULL;
    return queue->iterator->item;
}

void * queue_item (struct queue_t * queue) {
    if (!queue)                  return NULL;
    if (queue->first == NULL)    return NULL;
    if (queue->iterator == NULL) return NULL;

    return queue->iterator->item;
}

void queue_print (char * name, struct queue_t * queue, void(func)(void *)) {
    printf("%s: ", name);

    if (!queue) {
        printf("undef\n");
        return;
    }

    printf("[ ");

    int size = 0;
    struct node_t * node = queue->first;
    while (node != NULL) {
        // Caso nao haja funcao de impressao, o item sai como indefinido
        if (func) func(node->item);
        else      printf("undef");
        printf(" ");
        size++;
        node = node->next;
    }

    printf("] (%d items)\n", size);
}