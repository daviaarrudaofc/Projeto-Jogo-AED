#include "entrada.h"
#include "jogo.h"
#include "ranking.h"

#include <stdio.h>
#include <string.h>

static bool ler_nome(char nome[TAMANHO_NOME])
{
    while (ler_texto("Nome do cacador: ", nome, TAMANHO_NOME)) {
        if (nome[strspn(nome, " \t\r")] != '\0') {
            return true;
        }

        puts("Digite um nome para iniciar.");
    }

    return false;
}

static void exibir_controles(void)
{
    puts("\nNas masmorras, enfrente um inimigo por vez para avancar.");
    puts("Atacar ou usar uma pocao gasta seu turno.");
    puts("Voce pode consultar o inventario e voltar sem gastar o turno.");
    puts("Seus inimigos so contra-atacam se sobreviverem ao turno.");
    puts("Derrote inimigos para ganhar XP, subir de nivel e acumular pontos.");
}

int main(void)
{
    Ranking ranking = {0};

    for (;;) {
        puts("\n=== Solo Leveling: Dungeon Quest ===");
        puts("1. Iniciar jogo");
        puts("2. Ranking da sessao");
        puts("3. Como jogar");
        puts("0. Sair");

        int escolha;
        if (!ler_opcao(0, 3, &escolha) || escolha == 0) {
            break;
        }

        switch (escolha) {
            case 1: {
                char nome[TAMANHO_NOME];
                if (!ler_nome(nome)) {
                    return 0;
                }

                int pontos = jogar(nome);
                if (pontos >= 0) {
                    ranking_registrar(&ranking, nome, pontos);
                }
                break;
            }
            case 2:
                ranking_exibir(&ranking);
                break;
            case 3:
                exibir_controles();
                break;
        }
    }

    return 0;
}
