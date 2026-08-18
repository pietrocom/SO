// TAD filas genéricas
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.0 - 09/2025

// ATENÇÃO: ESTE ARQUIVO NÃO DEVE SER ALTERADO;
// ALTERAÇÕES SERÃO DESCARTADAS NA CORREÇÃO.

// Teste do TAD fila genérica (fila de itens void *)
// usando uma fila de strings com nomes de frutas.

#include <stdio.h>
#include <assert.h>
#include "queue.h"

// verifica o sistema operacional
#ifndef __linux__
#error "Este código foi planejado para ambientes Linux."
#endif

// imprime na tela um elemento da fila (chamada pela função queue_print)
void print_item(void *ptr)
{
    printf("%s", (char *)ptr);
}

int main()
{
    char *fruta[] = {"ameixa", "banana", "goiaba",
                     "laranja", "morango", "uva"};
    int num_frutas = 6;
    struct queue_t *q;
    char *item;
    int pos, status;

    // Testes com fila inexistente
    printf("1 Testes com uma fila inexistente\n");
    q = NULL;

    // tamanho deve ser ERROR
    printf("  teste queue_size\n");
    assert(queue_size(q) == ERROR);

    // destruir deve retornar ERROR
    printf("  teste queue_destroy\n");
    status = queue_destroy(q);
    assert(status == ERROR);

    // operações com iterador devem retornar NULL
    printf("  teste queue_head\n");
    assert(queue_head(q) == NULL);
    printf("  teste queue_next\n");
    assert(queue_next(q) == NULL);
    printf("  teste queue_item\n");
    assert(queue_item(q) == NULL);

    // inserção e remoção devem deve retornar ERROR
    printf("  teste queue_add\n");
    assert(queue_add(q, "teste") == ERROR);
    printf("  teste queue_del\n");
    assert(queue_del(q, "teste") == ERROR);

    queue_print("  fila", q, print_item);

    // Testes com fila existente, mas vazia
    printf("2 Testes com uma fila existente mas vazia\n");

    // cria a fila
    printf("  teste queue_create\n");
    q = queue_create();
    assert(q);

    // tamanho deve ser 0
    printf("  teste queue_size\n");
    assert(queue_size(q) == 0);

    // operações com iterador devem retornar NULL
    printf("  teste queue_head\n");
    assert(queue_head(q) == NULL);
    printf("  teste queue_next\n");
    assert(queue_next(q) == NULL);
    printf("  teste queue_item\n");
    assert(queue_item(q) == NULL);

    // remoção deve deve retornar NULL
    printf("  teste queue_del\n");
    assert(queue_del(q, "teste") == ERROR);

    queue_print("  fila", q, print_item);

    // Testes de inserção
    printf("3 Testes de inserção\n");

    for (int i = 0; i < num_frutas; i++)
    {
        item = fruta[i];
        printf("  colocando %s\n", item);
        assert(queue_add(q, item) == NOERROR);
        assert(queue_size(q) == (i + 1));
        queue_print("  fila", q, print_item);
    }

    printf("4 Testes do queue_print:\n");

    queue_print("  fila", NULL, print_item);
    queue_print("  fila", q, NULL);

    printf("5 Testes do iterador:\n");

    // iterador deve apontar para a primeira posição
    item = queue_item(q);
    pos = 0;

    // testa se iterador aponta e se desloca corretamente
    while (item)
    {
        printf("  iterador em %s (posição %d)\n", item, pos);
        assert(item == fruta[pos]);
        item = queue_next(q);
        pos++;
    }
    printf("  iterador em %s\n", (char *)queue_item(q));

    // testando queue_head
    assert(queue_head(q) == fruta[0]);
    assert(queue_item(q) == fruta[0]);

    printf("6 Testes de remoção\n");

    // testando queue_del com fila NULL
    printf("  retirando de fila NULL\n");
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_del(NULL, fruta[0]) == ERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[0]);

    // testando queue_del com item NULL
    printf("  retirando item NULL\n");
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_del(q, NULL) == ERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[0]);

    // testando queue_del com item que não está na fila
    printf("  retirando acerola (que não está na fila)\n");
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_del(q, "acerola") == ERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[0]);

    // testando queue_del com item no meio da fila
    pos = num_frutas / 2;
    printf("  retirando %s (no meio da fila)\n", fruta[pos]);
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_del(q, fruta[pos]) == NOERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[0]);

    // testando queue_del com item no inicio da fila
    pos = 0;
    printf("  retirando %s (no início da fila)\n", fruta[pos]);
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_del(q, fruta[pos]) == NOERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[1]);

    // testando queue_del com item no fim da fila
    pos = num_frutas - 1;
    printf("  retirando %s (no fim da fila)\n", fruta[pos]);
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_del(q, fruta[pos]) == NOERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[1]);

    // testando queue_del com item apontado pelo iterador, no meio
    queue_next(q);
    pos = 2;
    printf("  retirando sob o iterador\n");
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[pos]);
    assert(queue_del(q, fruta[pos]) == NOERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[4]);

    // testando queue_del com item apontado pelo iterador, no fim
    pos = 4;
    printf("  retirando sob o iterador\n");
    queue_print("    fila antes ", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == fruta[pos]);
    assert(queue_del(q, fruta[pos]) == NOERROR);
    queue_print("    fila depois", q, print_item);
    printf("    iterador em %s\n", (char *)queue_item(q));
    assert(queue_item(q) == NULL);

    // esvaziando a fila
    printf("7 Esvaziando a fila\n");
    queue_print("  fila", q, print_item);
    item = queue_head(q);
    while (item)
    {
        assert(queue_del(q, item) == NOERROR);
        printf("  retirei %s\n", item);
        queue_print("  fila", q, print_item);
        item = queue_item(q);
    }
    assert(queue_size(q) == 0);

    // testando queue_destroy
    printf("8 Teste queue_destroy\n");
    status = queue_destroy(q);
    assert(status == NOERROR);

    printf("Testes concluídos com sucesso!\n");
    return(0);
}
