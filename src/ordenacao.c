#include "ordenacao.h"

void quicksort_inimigos(Inimigo vetor[], int inicio, int fim)
{
    if (inicio >= fim) {
        return;
    }

    int poder_pivo = vetor[inicio].vida + vetor[inicio].ataque;
    int posicao_pivo = fim;

    for (int atual = fim; atual > inicio; --atual) {
        int poder = vetor[atual].vida + vetor[atual].ataque;
        if (poder >= poder_pivo) {
            Inimigo temporario = vetor[atual];
            vetor[atual] = vetor[posicao_pivo];
            vetor[posicao_pivo] = temporario;
            --posicao_pivo;
        }
    }

    Inimigo pivo = vetor[inicio];
    vetor[inicio] = vetor[posicao_pivo];
    vetor[posicao_pivo] = pivo;

    quicksort_inimigos(vetor, inicio, posicao_pivo - 1);
    quicksort_inimigos(vetor, posicao_pivo + 1, fim);
}

void quicksort_pontuacoes(Pontuacao vetor[], int inicio, int fim)
{
    if (inicio >= fim) {
        return;
    }

    int pontos_pivo = vetor[inicio].pontos;
    int posicao_pivo = fim;

    for (int atual = fim; atual > inicio; --atual) {
        if (vetor[atual].pontos <= pontos_pivo) {
            Pontuacao temporario = vetor[atual];
            vetor[atual] = vetor[posicao_pivo];
            vetor[posicao_pivo] = temporario;
            --posicao_pivo;
        }
    }

    Pontuacao pivo = vetor[inicio];
    vetor[inicio] = vetor[posicao_pivo];
    vetor[posicao_pivo] = pivo;

    quicksort_pontuacoes(vetor, inicio, posicao_pivo - 1);
    quicksort_pontuacoes(vetor, posicao_pivo + 1, fim);
}
