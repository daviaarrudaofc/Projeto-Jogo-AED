#ifndef FILA_H
#define FILA_H

#include <stdbool.h>

typedef struct {
    const char *nome;
    int vida;
    int ataque;
    int experiencia;
    int pontos;
    bool chefe;
} Inimigo;

typedef struct NoInimigo NoInimigo;

typedef struct {
    NoInimigo *inicio;
    NoInimigo *fim;
} Fila;

void fila_iniciar(Fila *fila);
bool fila_enfileirar(Fila *fila, Inimigo inimigo);

/* O ponteiro devolvido permanece valido ate a remocao do inimigo. */
Inimigo *fila_frente(const Fila *fila);

bool fila_desenfileirar(Fila *fila);
void fila_limpar(Fila *fila);

#endif
