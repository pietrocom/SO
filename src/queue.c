#include "../include/queue.h"

#include <stdio.h>

struct Queue * queue_create () {
    struct Queue * queue = malloc(sizeof(struct Queue));
    if (!queue) return NULL;

    queue->first = NULL;
    queue->last = NULL;
    queue->iterator = NULL;

    return queue;
}

void queue_destroy (struct Queue * queue) {
    if (!queue) return;

    while (queue->first != NULL) {
        struct Node * node = queue->first;
        // Pop garante que o proximo primeiro estara correto
        queue_pop(queue);
        node->item = NULL;
        node->next = NULL;
    }
}

void queue_push (struct Queue * queue, struct Node * node) {
    if (!queue || !node) return;

    // Caso a fila esteja vazia
    if (queue->last == NULL) {
        queue->first == node;
        queue->last == node;
    }
    else {
        queue->last->next = node;
        queue->last = node;
    }
}