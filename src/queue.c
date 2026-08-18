#include <stdio.h>
#include "../include/queue.h"

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
    struct queue_t * queue = malloc(sizeof (struct queue_t *));
    if (!queue) return NULL;

    queue->first = NULL;
    queue->last = NULL;
    queue->iterator = NULL;
}

int queue_destroy(struct queue_t *queue) {
    if (!queue) return ERROR;

    while (queue->first != NULL) {
        struct node_t * node = queue->first;
        queue->first = node->next;

        node = NULL;
        free(node);
    }
    queue->first = NULL;
    queue->last = NULL;
    queue->iterator = NULL;
    free (queue);
}