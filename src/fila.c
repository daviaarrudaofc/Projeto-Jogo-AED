#include "fila.h"

#include <stdlib.h>

struct NoInimigo {
    Inimigo inimigo;
    NoInimigo *proximo;
};

void fila_iniciar(Fila *fila)
{
    fila->inicio = NULL;
    fila->fim = NULL;
}

bool fila_enfileirar(Fila *fila, Inimigo inimigo)
{
    NoInimigo *novo = malloc(sizeof *novo);
    if (novo == NULL) {
        return false;
    }

    novo->inimigo = inimigo;
    novo->proximo = NULL;

    if (fila->fim == NULL) {
        fila->inicio = novo;
    } else {
        fila->fim->proximo = novo;
    }
    fila->fim = novo;
    return true;
}

Inimigo *fila_frente(const Fila *fila)
{
    if (fila->inicio == NULL) {
        return NULL;
    }
    return &fila->inicio->inimigo;
}

bool fila_desenfileirar(Fila *fila)
{
    if (fila->inicio == NULL) {
        return false;
    }

    NoInimigo *removido = fila->inicio;
    fila->inicio = removido->proximo;
    if (fila->inicio == NULL) {
        /* Saiu o último. Limpa o fim também, porque esse nó vai ser liberado. */
        fila->fim = NULL;
    }

    free(removido);
    return true;
}

void fila_limpar(Fila *fila)
{
    while (fila->inicio != NULL) {
        fila_desenfileirar(fila);
    }
}
