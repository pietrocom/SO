#ifndef QUEUE_H
#define QUEUE_H

struct Node {
    struct Node * next;
    void * item;
};

struct Queue {
    struct Node * first;
    struct Node * last;
    struct Node * iterator;
};


// ----- Operacoes basicas -----

struct Queue * queue_create ();
void queue_destroy (struct Queue * queue);
void queue_push (struct Queue * queue, struct Node * node);
// Pop nao retorna o proximo da fila
void queue_pop (struct Queue * queue);
int queue_size (struct Queue * queue);
void queue_print (struct Queue * queue);

// ----- Operacoes com Iterador -----

void queue_head (struct Queue * queue);
void queue_next (struct Queue * queue);
// Retorna o item indicado pelo iterador
struct Node queue_item (struct Queue * queue);

#endif  