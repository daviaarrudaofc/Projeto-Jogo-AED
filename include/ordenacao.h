#ifndef ORDENACAO_H
#define ORDENACAO_H

#include <stddef.h>

void quicksort(void *vetor, int quantidade, size_t tamanho_elemento,
               int (*comparar)(const void *, const void *));

#endif
