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
    int essencia;
    bool tem_igris;
    Inventario inventario;
} Cacador;

typedef enum {
    JOGO_EM_COMBATE,
    JOGO_PORTAL,
    JOGO_VITORIA,
    JOGO_DERROTA,
    JOGO_ABANDONADO,
    JOGO_ERRO
} EstadoJogo;

enum { TAMANHO_MENSAGEM_JOGO = 512, ESSENCIA_MAXIMA = 3 };

typedef struct {
    Cacador cacador;
    Fila fila;
    size_t indice_masmorra;
    int vida_maxima_inimigo;
    int turno_inimigo;
    bool chefe_em_furia;
    bool portal_examinado;
    EstadoJogo estado;
    char mensagem[TAMANHO_MENSAGEM_JOGO];
} Jogo;

bool jogo_iniciar(Jogo *jogo);
void jogo_limpar(Jogo *jogo);
void jogo_atacar(Jogo *jogo);
void jogo_defender(Jogo *jogo);
void jogo_invocar_sombra(Jogo *jogo);
void jogo_usar_item(Jogo *jogo, size_t posicao);
void jogo_examinar_portal(Jogo *jogo);
void jogo_escolher_portal(Jogo *jogo, bool investigar);
void jogo_abandonar(Jogo *jogo);

const Inimigo *jogo_inimigo_atual(const Jogo *jogo);
const char *jogo_nome_masmorra(const Jogo *jogo);
int jogo_experiencia_para_subir(const Cacador *cacador);
bool jogo_golpe_carregado(const Jogo *jogo);
int jogo_dano_previsto(const Jogo *jogo);
int jogo_dano_sombra(const Jogo *jogo);

#endif
