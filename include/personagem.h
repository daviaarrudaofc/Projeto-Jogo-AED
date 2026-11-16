#ifndef PERSONAGEM_H
#define PERSONAGEM_H

#include <raylib.h>
#include <stdbool.h>

typedef struct {
    Vector2 posicao;
    float fase;
    int direcao;
    bool andando;
} Personagem;

typedef enum {
    VISUAL_CACADOR,
    VISUAL_MONSTRO,
    VISUAL_CAVALEIRO,
    VISUAL_ESTATUA,
    VISUAL_SOMBRA
} VisualPersonagem;

typedef enum {
    POSE_NORMAL,
    POSE_ATAQUE,
    POSE_DEFESA,
    POSE_FERIDO
} PosePersonagem;

void personagem_reiniciar(Personagem *personagem, Vector2 posicao, int direcao);
void personagem_mover(Personagem *personagem, Vector2 eixo, float segundos, Rectangle limites);
void personagem_desenhar(const Personagem *personagem, VisualPersonagem visual,
                         PosePersonagem pose, float progresso, float escala);

#endif
