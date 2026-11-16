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

static int escolher_item(const Inventario *inventario)
{
    size_t quantidade = inventario_quantidade(inventario);
    if (quantidade == 0) {
        puts("O inventario esta vazio.");
        return 0;
    }

    puts("\nInventario");
    for (size_t posicao = 1; posicao <= quantidade; ++posicao) {
        Item item;
        if (inventario_buscar(inventario, posicao, &item)) {
            printf("%zu. %s (+%d PV)\n", posicao, item.nome, item.cura);
        }
    }
    puts("0. Voltar");

    int escolha;
    return ler_opcao(0, (int)quantidade, &escolha) ? escolha : -1;
}

static void jogar_turno(Jogo *jogo, const Inimigo *inimigo)
{
    const Cacador *cacador = &jogo->cacador;
    printf("\nSeus PV: %d/%d | Nivel: %d | Ataque: %d | XP: %d/%d\n",
           cacador->vida, cacador->vida_maxima, cacador->nivel,
           cacador->ataque, cacador->experiencia, jogo_experiencia_para_subir(cacador));
    printf("%s: %d PV | Ataque: %d\n", inimigo->nome, inimigo->vida, inimigo->ataque);
    puts("1. Atacar | 2. Inventario | 3. Abandonar partida");

    int escolha;
    if (!ler_opcao(1, 3, &escolha) || escolha == 3) {
        jogo_abandonar(jogo);
    } else if (escolha == 1) {
        jogo_atacar(jogo);
    } else {
        int posicao = escolher_item(&cacador->inventario);
        if (posicao == 0) {
            return;
        }
        if (posicao < 0) {
            jogo_abandonar(jogo);
        } else {
            jogo_usar_item(jogo, (size_t)posicao);
        }
    }

    puts(jogo->mensagem);
}

static int jogar(const char *nome)
{
    Jogo jogo;
    if (!jogo_iniciar(&jogo)) {
        puts(jogo.mensagem);
        jogo_limpar(&jogo);
        return -1;
    }

    printf("\nCacador: %s\n", nome);
    puts(jogo.mensagem);
    const char *masmorra_anterior = NULL;
    const char *inimigo_anterior = NULL;

    while (jogo.estado == JOGO_EM_COMBATE) {
        const char *masmorra = jogo_nome_masmorra(&jogo);
        const Inimigo *inimigo = jogo_inimigo_atual(&jogo);
        if (masmorra != masmorra_anterior) {
            printf("\n=== %s ===\n", masmorra);
            masmorra_anterior = masmorra;
        }
        if (inimigo->nome != inimigo_anterior) {
            printf("\n%s: %s\n", inimigo->chefe ? "Chefe" : "Inimigo", inimigo->nome);
            inimigo_anterior = inimigo->nome;
        }
        jogar_turno(&jogo, inimigo);
    }

    int pontos = -1;
    if (jogo.estado == JOGO_VITORIA || jogo.estado == JOGO_DERROTA) {
        pontos = jogo.cacador.pontos;
        printf("Pontuacao final: %d\n", pontos);
    }
    jogo_limpar(&jogo);
    return pontos;
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
