#include "ranking.h"

#include <stdio.h>

#include "ordenacao.h"

void ranking_registrar(Ranking *ranking, const char *nome, int pontos)
{
    if (ranking->quantidade < LIMITE_RANKING) {
        ranking->quantidade++;
    } else if (pontos <= ranking->resultados[LIMITE_RANKING - 1].pontos) {
        return;
    }

    Pontuacao resultado = {0};
    resultado.pontos = pontos;
    snprintf(resultado.nome, sizeof resultado.nome, "%s", nome);
    ranking->resultados[ranking->quantidade - 1] = resultado;

    quicksort_pontuacoes(ranking->resultados, 0, ranking->quantidade - 1);
}

void ranking_exibir(const Ranking *ranking)
{
    puts("\n=== Ranking da sessao ===");
    if (ranking->quantidade == 0) {
        puts("Nenhuma pontuacao registrada.");
        return;
    }

    for (int posicao = 0; posicao < ranking->quantidade; posicao++) {
        const Pontuacao *resultado = &ranking->resultados[posicao];
        printf("%2d. %s - %d pontos\n", posicao + 1, resultado->nome,
               resultado->pontos);
    }
}
