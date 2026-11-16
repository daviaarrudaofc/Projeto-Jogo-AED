#include "inventario.h"

#include <stdlib.h>

struct NoItem {
    Item item;
    NoItem *proximo;
};

void inventario_iniciar(Inventario *inventario)
{
    inventario->inicio = NULL;
}

bool inventario_adicionar(Inventario *inventario, Item item)
{
    NoItem *novo = malloc(sizeof *novo);
    if (novo == NULL) {
        return false;
    }

    novo->item = item;
    novo->proximo = NULL;

    if (inventario->inicio == NULL) {
        inventario->inicio = novo;
        return true;
    }

    NoItem *ultimo = inventario->inicio;
    while (ultimo->proximo != NULL) {
        ultimo = ultimo->proximo;
    }
    ultimo->proximo = novo;
    return true;
}

bool inventario_buscar(const Inventario *inventario, size_t posicao, Item *item)
{
    const NoItem *atual = inventario->inicio;
    for (size_t indice = 1; atual != NULL; indice++) {
        if (indice == posicao) {
            if (item != NULL) {
                *item = atual->item;
            }
            return true;
        }
        atual = atual->proximo;
    }
    return false;
}

bool inventario_remover(Inventario *inventario, size_t posicao, Item *item)
{
    NoItem *atual = inventario->inicio;
    NoItem *anterior = NULL;

    if (posicao == 0) {
        return false;
    }

    for (size_t indice = 1; atual != NULL && indice < posicao; indice++) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (atual == NULL) {
        return false;
    }

    if (anterior == NULL) {
        inventario->inicio = atual->proximo;
    } else {
        anterior->proximo = atual->proximo;
    }

    if (item != NULL) {
        *item = atual->item;
    }
    free(atual);
    return true;
}

size_t inventario_quantidade(const Inventario *inventario)
{
    size_t quantidade = 0;
    const NoItem *atual = inventario->inicio;
    while (atual != NULL) {
        quantidade++;
        atual = atual->proximo;
    }
    return quantidade;
}

void inventario_limpar(Inventario *inventario)
{
    while (inventario->inicio != NULL) {
        inventario_remover(inventario, 1, NULL);
    }
}
