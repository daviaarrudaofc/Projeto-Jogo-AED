#ifndef INVENTARIO_H
#define INVENTARIO_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *nome;
    int cura;
} Item;

typedef struct NoItem NoItem;

typedef struct {
    NoItem *inicio;
} Inventario;

void inventario_iniciar(Inventario *inventario);
bool inventario_adicionar(Inventario *inventario, Item item);

/* A primeira posicao do inventario e 1. */
bool inventario_buscar(const Inventario *inventario, size_t posicao, Item *item);
bool inventario_remover(Inventario *inventario, size_t posicao, Item *item);

size_t inventario_quantidade(const Inventario *inventario);
void inventario_limpar(Inventario *inventario);

#endif
