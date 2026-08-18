// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.1 -- 07/2026

// ATENÇÃO: ESTE ARQUIVO NÃO DEVE SER ALTERADO;
// ALTERAÇÕES SERÃO DESCARTADAS NA CORREÇÃO.

// Teste do TAD mapa de objetos

#include <stdio.h>
#include <assert.h>
#include "map.h"

char *fruta[] = {"banana", "uva", "morango", "goiaba",
                 "laranja", "pera", "melancia", "jaca"};

int main()
{
    struct map_t *a;
    int id;
    char *obj;

    // create
    a = map_create(5);
    map_print("map", a);
    assert(a);

    printf("\n");

    // put
    obj = fruta[0];
    id = map_put(a, obj);
    assert(id >= 0);
    printf("Object \"%s\" registered under ID %d\n", obj, id);
    map_print("map", a);

    // store
    obj = fruta[1];
    id = map_put(a, obj);
    assert(id >= 0);
    printf("Object \"%s\" registered under ID %d\n", obj, id);
    map_print("map", a);

    // store
    obj = fruta[2];
    id = map_put(a, obj);
    assert(id >= 0);
    printf("Object \"%s\" registered under ID %d\n", obj, id);
    map_print("map", a);

    // store
    obj = fruta[3];
    id = map_put(a, obj);
    assert(id >= 0);
    printf("Object \"%s\" registered under ID %d\n", obj, id);
    map_print("map", a);

    // store
    obj = fruta[4];
    id = map_put(a, obj);
    assert(id >= 0);
    printf("Object \"%s\" registered under ID %d\n", obj, id);
    map_print("map", a);

    // store (error)
    obj = fruta[5];
    id = map_put(a, obj);
    assert(id < 0);
    printf("Object \"%s\" not registered\n", obj);
    map_print("map", a);

    printf("\n");

    // retrieve
    id = 0;
    obj = map_get(a, id);
    assert(obj == fruta[0]);
    printf("Object under ID %d is \"%s\"\n", id, obj);
    map_print("map", a);

    // retrieve
    id = 4;
    obj = map_get(a, id);
    assert(obj == fruta[4]);
    printf("Object under ID %d is \"%s\"\n", id, obj);
    map_print("map", a);

    // retrieve (error)
    id = 5;
    obj = map_get(a, id);
    assert(obj == NULL);
    printf("ID %d is not associated to an object\n", id);
    map_print("map", a);

    printf("\n");

    // release
    id = 3;
    obj = map_del(a, id);
    assert(obj == fruta[3]);
    printf("ID %d deleted had object \"%s\"\n", id, obj);
    map_print("map", a);

    // release
    id = 1;
    obj = map_del(a, id);
    assert(obj == fruta[1]);
    printf("ID %d deleted had object \"%s\"\n", id, obj);
    map_print("map", a);

    // release (error)
    id = 5;
    obj = map_del(a, id);
    assert(obj == NULL);
    printf("ID %d is not associated to an object\n", id);
    map_print("map", a);

    printf("\n");

    // store
    obj = fruta[6];
    id = map_put(a, obj);
    assert(id == 1);
    printf("Object \"%s\" registered under ID %d\n", obj, id);
    map_print("map", a);

    // store
    obj = fruta[5];
    id = map_put(a, obj);
    assert(id == 3);
    printf("Object \"%s\" registered under ID %d\n", obj, id);
    map_print("map", a);

    // store (error)
    obj = fruta[7];
    id = map_put(a, obj);
    assert(id == -1);
    printf("Object \"%s\" not registered\n", obj);
    map_print("map", a);

    printf("\n");

    map_destroy(a);
    printf("Map destroyed\n");

    printf("Testes concluídos com sucesso!\n");
    return(0);
}

