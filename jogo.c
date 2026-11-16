#include "jogo.h"

#include "entrada.h"
#include "fila.h"
#include "inventario.h"
#include "ordenacao.h"

#include <stdio.h>

typedef struct {
    int vida;
    int vida_maxima;
    int ataque;
    int nivel;
    int experiencia;
    int pontos;
    Inventario inventario;
} Cacador;

typedef struct {
    const char *nome;
    Inimigo inimigos[3];
    int quantidade;
} Masmorra;

typedef enum {
    CACA_EM_CURSO,
    CACADOR_DERROTADO,
    CACA_INTERROMPIDA
} ResultadoCombate;

typedef enum {
    ITEM_CANCELADO,
    ITEM_USADO,
    INVENTARIO_INTERROMPIDO
} ResultadoItem;

static const Item pocao = {"Pocao de cura", 30};

static int comparar_inimigos(const void *primeiro, const void *segundo)
{
    const Inimigo *inimigo_a = primeiro;
    const Inimigo *inimigo_b = segundo;
    int poder_a = inimigo_a->vida + inimigo_a->ataque;
    int poder_b = inimigo_b->vida + inimigo_b->ataque;

    return poder_a - poder_b;
}

static bool preparar_fila(Fila *fila, Masmorra *masmorra)
{
    /* Ordena por poder antes de definir a sequencia de combate na fila. */
    quicksort(masmorra->inimigos, masmorra->quantidade,
              sizeof masmorra->inimigos[0], comparar_inimigos);

    for (int indice = 0; indice < masmorra->quantidade; ++indice) {
        if (!fila_enfileirar(fila, masmorra->inimigos[indice])) {
            return false;
        }
    }

    return true;
}

static ResultadoItem usar_item(Cacador *cacador)
{
    size_t quantidade = inventario_quantidade(&cacador->inventario);
    if (quantidade == 0) {
        puts("O inventario esta vazio.");
        return ITEM_CANCELADO;
    }
    puts("\nInventario");
    for (size_t posicao = 1; posicao <= quantidade; ++posicao) {
        Item item;
        if (inventario_buscar(&cacador->inventario, posicao, &item)) {
            printf("%zu. %s (+%d PV)\n", posicao, item.nome, item.cura);
        }
    }
    puts("0. Voltar");

    int escolha;
    if (!ler_opcao(0, (int)quantidade, &escolha)) {
        return INVENTARIO_INTERROMPIDO;
    }
    if (escolha == 0) {
        return ITEM_CANCELADO;
    }
    if (cacador->vida == cacador->vida_maxima) {
        puts("Sua vida ja esta cheia.");
        return ITEM_CANCELADO;
    }

    Item item;
    if (!inventario_remover(&cacador->inventario, (size_t)escolha, &item)) {
        return ITEM_CANCELADO;
    }

    int vida_anterior = cacador->vida;
    cacador->vida += item.cura;
    if (cacador->vida > cacador->vida_maxima) {
        cacador->vida = cacador->vida_maxima;
    }

    printf("Voce usou %s e recuperou %d PV.\n", item.nome,
           cacador->vida - vida_anterior);
    return ITEM_USADO;
}

static ResultadoCombate combater(Cacador *cacador, Inimigo *inimigo)
{
    const char *tipo = inimigo->chefe ? "Chefe" : "Inimigo";
    printf("\n%s: %s\n", tipo, inimigo->nome);

    while (inimigo->vida > 0 && cacador->vida > 0) {
        printf("\nSeus PV: %d/%d | Nivel: %d | Ataque: %d | XP: %d/%d\n",
               cacador->vida, cacador->vida_maxima, cacador->nivel,
               cacador->ataque, cacador->experiencia, cacador->nivel * 30);
        printf("%s: %d PV | Ataque: %d\n", inimigo->nome, inimigo->vida,
               inimigo->ataque);
        puts("1. Atacar | 2. Inventario | 3. Abandonar partida");

        int escolha;
        if (!ler_opcao(1, 3, &escolha) || escolha == 3) {
            return CACA_INTERROMPIDA;
        }

        if (escolha == 1) {
            int dano = cacador->ataque < inimigo->vida ? cacador->ataque : inimigo->vida;
            inimigo->vida -= dano;
            printf("Voce causou %d de dano.\n", dano);
        } else {
            ResultadoItem resultado = usar_item(cacador);
            if (resultado == INVENTARIO_INTERROMPIDO) {
                return CACA_INTERROMPIDA;
            }
            if (resultado == ITEM_CANCELADO) {
                continue;
            }
        }

        if (inimigo->vida > 0) {
            int dano = inimigo->ataque < cacador->vida ? inimigo->ataque : cacador->vida;
            cacador->vida -= dano;
            printf("%s causou %d de dano.\n", inimigo->nome, dano);
        }
    }

    return cacador->vida > 0 ? CACA_EM_CURSO : CACADOR_DERROTADO;
}

static void receber_recompensa(Cacador *cacador, const Inimigo *inimigo)
{
    cacador->experiencia += inimigo->experiencia;
    cacador->pontos += inimigo->pontos;
    printf("%s foi derrotado. +%d XP e +%d pontos.\n",
           inimigo->nome, inimigo->experiencia, inimigo->pontos);

    while (cacador->experiencia >= cacador->nivel * 30) {
        cacador->experiencia -= cacador->nivel * 30;
        ++cacador->nivel;
        cacador->vida_maxima += 12;
        cacador->vida += 12;
        cacador->ataque += 4;
        printf("Nivel %d alcancado! +12 PV e +4 de ataque.\n", cacador->nivel);
    }
}

static ResultadoCombate explorar_masmorra(Cacador *cacador, Masmorra *masmorra)
{
    Fila fila;
    fila_iniciar(&fila);

    if (!preparar_fila(&fila, masmorra)) {
        fila_limpar(&fila);
        puts("Memoria insuficiente para preparar a masmorra.");
        return CACA_INTERROMPIDA;
    }

    printf("\n=== %s ===\n", masmorra->nome);
    ResultadoCombate resultado = CACA_EM_CURSO;
    Inimigo *inimigo = fila_frente(&fila);

    while (inimigo != NULL) {
        resultado = combater(cacador, inimigo);
        if (resultado != CACA_EM_CURSO) {
            break;
        }

        receber_recompensa(cacador, inimigo);
        fila_desenfileirar(&fila);
        inimigo = fila_frente(&fila);
    }

    fila_limpar(&fila);
    return resultado;
}

int jogar(const char *nome)
{
    Cacador cacador = {.vida = 75, .vida_maxima = 75, .ataque = 15, .nivel = 1};
    Masmorra masmorras[] = {
        {"Tunel do Portal", {
            {"Orc do Tunel", 33, 8, 20, 160, false},
            {"Goblin do Portal", 20, 6, 15, 100, false}
        }, 2},
        {"Templo da Estatua", {
            {"Estatua Divina", 76, 15, 50, 500, true},
            {"Sentinela de Pedra", 46, 12, 30, 260, false},
            {"Cao das Sombras", 36, 10, 20, 200, false}
        }, 3}
    };
    const size_t total_masmorras = sizeof masmorras / sizeof masmorras[0];

    inventario_iniciar(&cacador.inventario);
    if (!inventario_adicionar(&cacador.inventario, pocao)) {
        puts("Memoria insuficiente para iniciar a partida.");
        return -1;
    }

    printf("\n%s, voce recebeu uma pocao de cura antes de entrar no portal.\n", nome);
    ResultadoCombate resultado = CACA_EM_CURSO;

    for (size_t indice = 0; indice < total_masmorras; ++indice) {
        resultado = explorar_masmorra(&cacador, &masmorras[indice]);
        if (resultado != CACA_EM_CURSO) {
            break;
        }

        cacador.pontos += 100;
        printf("Masmorra concluida! +100 pontos. Total: %d\n", cacador.pontos);

        if (indice + 1 < total_masmorras) {
            if (inventario_adicionar(&cacador.inventario, pocao)) {
                puts("Voce coletou uma pocao de cura.");
            } else {
                puts("Memoria insuficiente para guardar a pocao encontrada.");
            }
        }
    }

    inventario_limpar(&cacador.inventario);
    if (resultado == CACA_INTERROMPIDA) {
        puts("\nPartida interrompida.");
        return -1;
    }

    if (resultado == CACADOR_DERROTADO) {
        puts("\nDerrota! Seus pontos de vida acabaram.");
    } else {
        puts("\nVitoria! Voce derrotou o chefe final.");
    }

    printf("Pontuacao final: %d\n", cacador.pontos);
    return cacador.pontos;
}
