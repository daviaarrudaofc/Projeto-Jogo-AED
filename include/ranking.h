#ifndef RANKING_H
#define RANKING_H

enum { TAMANHO_NOME = 32, LIMITE_RANKING = 10 };

typedef struct {
    char nome[TAMANHO_NOME];
    int pontos;
} Pontuacao;

typedef struct {
    Pontuacao resultados[LIMITE_RANKING];
    int quantidade;
} Ranking;

void ranking_registrar(Ranking *ranking, const char *nome, int pontos);
void ranking_exibir(const Ranking *ranking);

#endif
