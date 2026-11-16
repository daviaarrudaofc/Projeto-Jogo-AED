#include "entrada.h"
#include "jogo.h"
#include "ranking.h"

#include <stdio.h>

static bool ler_nome(char nome[TAMANHO_NOME])
{
    while (ler_texto("Nome do cacador: ", nome, TAMANHO_NOME)) {
        char primeira_letra;
        if (sscanf(nome, " %c", &primeira_letra) == 1) {
            return true;
        }

        puts("Digite um nome para iniciar.");
    }

    return false;
}

static void exibir_controles(void)
{
    puts("\nNas masmorras, enfrente um inimigo por vez para avancar.");
    puts("Leia a intencao do inimigo antes de escolher sua acao.");
    puts("Defender reduz o dano normal.");
    puts("Contra um golpe carregado, a defesa anula o dano e recupera 1 essencia.");
    puts("Erga-se consome 1 essencia para atacar junto com uma sombra.");
    puts("Usar uma pocao gasta o turno. A consulta ao inventario e livre.");
    puts("Entre as masmorras, examine a inscricao do portal para descobrir uma passagem opcional.");
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
    printf("Essencia: %d/%d | Pontos: %d\n",
           cacador->essencia, ESSENCIA_MAXIMA, cacador->pontos);
    printf("%s: %d/%d PV\n", inimigo->nome, inimigo->vida, jogo->vida_maxima_inimigo);
    printf("Intencao inimiga: %s (%d de dano)\n",
           jogo_golpe_carregado(jogo) ? "golpe carregado" : "ataque",
           jogo_dano_previsto(jogo));
    puts("1. Atacar | 2. Defender");
    printf("3. Erga-se (%s, %d de dano)\n", cacador->tem_igris ? "Igris" : "Sombra",
           jogo_dano_sombra(jogo));
    puts("4. Inventario | 5. Recuar");

    int escolha;
    if (!ler_opcao(1, 5, &escolha)) {
        escolha = 5;
    }

    switch (escolha) {
        case 1:
            jogo_atacar(jogo);
            break;
        case 2:
            jogo_defender(jogo);
            break;
        case 3:
            jogo_invocar_sombra(jogo);
            break;
        case 4: {
            int posicao = escolher_item(&cacador->inventario);
            if (posicao == 0) {
                return;
            }
            if (posicao < 0) {
                jogo_abandonar(jogo);
            } else {
                jogo_usar_item(jogo, (size_t)posicao);
            }
            break;
        }
        case 5:
            jogo_abandonar(jogo);
            break;
    }

    puts(jogo->mensagem);
}

static void jogar_portal(Jogo *jogo)
{
    puts("\n=== Portal ===");
    puts("1. Seguir para o Templo");
    puts(jogo->portal_examinado ? "2. Entrar na passagem opcional" : "2. Examinar inscricao");
    puts("3. Recuar");

    int escolha;
    if (!ler_opcao(1, 3, &escolha) || escolha == 3) {
        jogo_abandonar(jogo);
    } else if (escolha == 2 && !jogo->portal_examinado) {
        jogo_examinar_portal(jogo);
    } else {
        jogo_escolher_portal(jogo, escolha == 2);
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

    while (jogo.estado == JOGO_EM_COMBATE || jogo.estado == JOGO_PORTAL) {
        if (jogo.estado == JOGO_PORTAL) {
            jogar_portal(&jogo);
            continue;
        }

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
