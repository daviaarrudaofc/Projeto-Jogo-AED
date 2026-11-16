#include "entrada.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool ler_texto(const char *mensagem, char *texto, size_t capacidade)
{
    for (;;) {
        printf("%s", mensagem);
        fflush(stdout);

        if (fgets(texto, (int)capacidade, stdin) == NULL) {
            return false;
        }

        char *fim_linha = strchr(texto, '\n');
        if (fim_linha != NULL) {
            *fim_linha = '\0';
            return true;
        }

        int caractere = getchar();
        if (caractere == '\n' || caractere == EOF) {
            return true;
        }

        do {
            caractere = getchar();
        } while (caractere != '\n' && caractere != EOF);

        printf("Digite no maximo %zu caracteres.\n", capacidade - 1);
    }
}

bool ler_opcao(int minimo, int maximo, int *opcao)
{
    char texto[32];

    while (ler_texto("> ", texto, sizeof texto)) {
        char *fim;
        errno = 0;
        long numero = strtol(texto, &fim, 10);

        if (fim != texto && errno == 0 && numero >= minimo && numero <= maximo) {
            while (isspace((unsigned char)*fim)) {
                ++fim;
            }

            if (*fim == '\0') {
                *opcao = (int)numero;
                return true;
            }
        }

        printf("Escolha uma opcao entre %d e %d.\n", minimo, maximo);
    }

    return false;
}
