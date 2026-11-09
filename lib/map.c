// PingPongOS - PingPong Operating System
// © Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 06/2026

// Este arquivo PODE/DEVE ser alterado.

// Implementação do TAD Mapa de objetos

#include <stdio.h>
#include <stdlib.h>
#include "map.h"

// Um slot com valor NULL indica um ID livre; por isso o mapa
// não aceita registrar objetos NULL.
struct map_t {
    void ** slots;  // vetor de objetos indexado pelo ID
    int size;       // número de IDs disponíveis (0..size-1)
    int items;      // número de IDs em uso
    int last;       // último ID alocado (-1 se nenhum ainda)
};

// Retorna 1 se o ID é válido para o mapa, 0 caso contrário
static int valid_id (struct map_t * map, int id) {
    return (id >= 0 && id < map->size);
}

struct map_t * map_create (int size) {
    if (size <= 0) return NULL;

    struct map_t * map = malloc(sizeof(struct map_t));
    if (!map) return NULL;

    // calloc garante todos os slots inicialmente NULL (livres)
    map->slots = calloc(size, sizeof(void *));
    if (!map->slots) {
        free(map);
        return NULL;
    }

    map->size  = size;
    map->items = 0;
    map->last  = -1;    // primeiro ID alocado será 0

    return map;
}

int map_destroy (struct map_t * map) {
    if (!map) return ERROR;

    // os objetos em si pertencem a quem os criou; libera só a estrutura
    free(map->slots);
    free(map);

    return NOERROR;
}

int map_put (struct map_t * map, void * object) {
    if (!map || !object)         return ERROR;
    if (map->items >= map->size) return ERROR;

    // busca circular pelo próximo ID livre, a partir do último alocado;
    // como items < size, existe pelo menos um slot livre
    int id = map->last;
    do {
        id = (id + 1) % map->size;
    } while (map->slots[id] != NULL);

    map->slots[id] = object;
    map->items++;
    map->last = id;

    return id;
}

void * map_get (struct map_t * map, int id) {
    if (!map || !valid_id(map, id)) return NULL;

    return map->slots[id];
}

void * map_del (struct map_t * map, int id) {
    if (!map || !valid_id(map, id)) return NULL;

    void * object = map->slots[id];
    if (!object) return NULL;   // ID não estava em uso

    map->slots[id] = NULL;
    map->items--;

    return object;
}

int map_items (struct map_t * map) {
    if (!map) return ERROR;

    return map->items;
}

int map_size (struct map_t * map) {
    if (!map) return ERROR;

    return map->size;
}

void map_print (char * name, struct map_t * map) {
    printf("%s: ", name);

    if (!map) {
        printf("undef\n");
        return;
    }

    printf("[ ");
    for (int id = 0; id < map->size; id++)
        printf("%c ", map->slots[id] ? '*' : '-');

    printf("] (%d/%d)\n", map->items, map->size);
}
