#ifndef ENTRADA_H
#define ENTRADA_H

#include <stdbool.h>
#include <stddef.h>

bool ler_texto(const char *mensagem, char *texto, size_t capacidade);
bool ler_opcao(int minimo, int maximo, int *opcao);

#endif
