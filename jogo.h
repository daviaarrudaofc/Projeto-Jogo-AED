#ifndef JOGO_H
#define JOGO_H

#include "fila.h"
#include "inventario.h"

typedef struct {
    int vida;
    int vida_maxima;
    int ataque;
    int nivel;
    int experiencia;
    int pontos;
    Inventario inventario;
} Cacador;

typedef enum {
    JOGO_EM_COMBATE,
    JOGO_VITORIA,
    JOGO_DERROTA,
    JOGO_ABANDONADO,
    JOGO_ERRO
} EstadoJogo;

enum { TAMANHO_MENSAGEM_JOGO = 512 };

typedef struct {
    Cacador cacador;
    Fila fila;
    size_t indice_masmorra;
    int vida_maxima_inimigo;
    EstadoJogo estado;
    char mensagem[TAMANHO_MENSAGEM_JOGO];
} Jogo;

/* Para reutilizar a mesma variavel, encerre ou limpe a partida anterior. */
bool jogo_iniciar(Jogo *jogo);
void jogo_limpar(Jogo *jogo);
void jogo_atacar(Jogo *jogo);
void jogo_usar_item(Jogo *jogo, size_t posicao);
void jogo_abandonar(Jogo *jogo);

/* O inimigo pode mudar ou sair da fila depois de uma acao. */
const Inimigo *jogo_inimigo_atual(Jogo *jogo);
const char *jogo_nome_masmorra(const Jogo *jogo);
size_t jogo_total_masmorras(void);
int jogo_experiencia_para_subir(const Cacador *cacador);

#endif
