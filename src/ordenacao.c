#include "ordenacao.h"

static void trocar(unsigned char *primeiro, unsigned char *segundo, size_t tamanho)
{
    /* Trocar byte por byte deixa a gente usar o mesmo QuickSort pra inimigos e pontuações. */
    for (size_t byte = 0; byte < tamanho; ++byte) {
        unsigned char temporario = primeiro[byte];
        primeiro[byte] = segundo[byte];
        segundo[byte] = temporario;
    }
}

static int particionar(unsigned char *vetor, int inicio, int fim, size_t tamanho,
                       int (*comparar)(const void *, const void *))
{
    unsigned char *pivo = vetor + (size_t)inicio * tamanho;
    int indice = fim;

    /* Pelo comparador, quem vem depois do pivô ou empata com ele vai pra direita. */
    for (int atual = fim; atual > inicio; --atual) {
        unsigned char *elemento = vetor + (size_t)atual * tamanho;
        if (comparar(elemento, pivo) >= 0) {
            trocar(elemento, vetor + (size_t)indice * tamanho, tamanho);
            --indice;
        }
    }

    trocar(pivo, vetor + (size_t)indice * tamanho, tamanho);
    return indice;
}

static void ordenar_intervalo(unsigned char *vetor, int inicio, int fim, size_t tamanho,
                              int (*comparar)(const void *, const void *))
{
    if (inicio >= fim) {
        return;
    }

    int pivo = particionar(vetor, inicio, fim, tamanho, comparar);
    ordenar_intervalo(vetor, inicio, pivo - 1, tamanho, comparar);
    ordenar_intervalo(vetor, pivo + 1, fim, tamanho, comparar);
}

void quicksort(void *vetor, int quantidade, size_t tamanho_elemento,
               int (*comparar)(const void *, const void *))
{
    ordenar_intervalo(vetor, 0, quantidade - 1, tamanho_elemento, comparar);
}
