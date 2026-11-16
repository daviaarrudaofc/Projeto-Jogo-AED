#include "ranking.h"

#include <stdio.h>

#include "ordenacao.h"

static int comparar_pontuacoes(const void *primeiro, const void *segundo)
{
    const Pontuacao *pontuacao_primeira = primeiro;
    const Pontuacao *pontuacao_segunda = segundo;

    if (pontuacao_primeira->pontos > pontuacao_segunda->pontos) {
        return -1;
    }
    if (pontuacao_primeira->pontos < pontuacao_segunda->pontos) {
        return 1;
    }
    return 0;
}

void ranking_registrar(Ranking *ranking, const char *nome, int pontos)
{
    /* Com o ranking cheio, a menor pontuação fica no fim e só sai se a nova for maior. */
    if (ranking->quantidade < LIMITE_RANKING) {
        ranking->quantidade++;
    } else if (pontos <= ranking->resultados[LIMITE_RANKING - 1].pontos) {
        return;
    }

    Pontuacao resultado = {.pontos = pontos};
    snprintf(resultado.nome, sizeof resultado.nome, "%s", nome);
    ranking->resultados[ranking->quantidade - 1] = resultado;

    quicksort(ranking->resultados, ranking->quantidade,
              sizeof ranking->resultados[0], comparar_pontuacoes);
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
