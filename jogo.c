#include "jogo.h"

#include "ordenacao.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *nome;
    Inimigo inimigos[3];
    int quantidade;
} Masmorra;

static const Item pocao = {"Pocao de cura", 30};

static const Masmorra masmorras[] = {
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

static void anotar_evento(Jogo *jogo, const char *formato, ...)
{
    size_t tamanho = strlen(jogo->mensagem);
    va_list argumentos;
    va_start(argumentos, formato);
    vsnprintf(jogo->mensagem + tamanho, sizeof jogo->mensagem - tamanho,
              formato, argumentos);
    va_end(argumentos);
}

static int comparar_inimigos(const void *primeiro, const void *segundo)
{
    const Inimigo *inimigo_a = primeiro;
    const Inimigo *inimigo_b = segundo;
    int poder_a = inimigo_a->vida + inimigo_a->ataque;
    int poder_b = inimigo_b->vida + inimigo_b->ataque;

    return poder_a - poder_b;
}

static bool preparar_fila(Jogo *jogo)
{
    /* Ordena uma copia. Cada partida comeca com inimigos de vida cheia. */
    Masmorra masmorra = masmorras[jogo->indice_masmorra];
    quicksort(masmorra.inimigos, masmorra.quantidade,
              sizeof masmorra.inimigos[0], comparar_inimigos);

    for (int indice = 0; indice < masmorra.quantidade; ++indice) {
        if (!fila_enfileirar(&jogo->fila, masmorra.inimigos[indice])) {
            return false;
        }
    }

    jogo->vida_maxima_inimigo = fila_frente(&jogo->fila)->vida;
    return true;
}

void jogo_limpar(Jogo *jogo)
{
    fila_limpar(&jogo->fila);
    inventario_limpar(&jogo->cacador.inventario);
    jogo->vida_maxima_inimigo = 0;
    if (jogo->estado == JOGO_EM_COMBATE) {
        jogo->estado = JOGO_ABANDONADO;
    }
}

static void encerrar_partida(Jogo *jogo, EstadoJogo estado)
{
    jogo->estado = estado;
    jogo_limpar(jogo);
}

bool jogo_iniciar(Jogo *jogo)
{
    *jogo = (Jogo){
        .cacador = {.vida = 75, .vida_maxima = 75, .ataque = 15, .nivel = 1},
        .estado = JOGO_EM_COMBATE
    };
    fila_iniciar(&jogo->fila);
    inventario_iniciar(&jogo->cacador.inventario);

    if (!inventario_adicionar(&jogo->cacador.inventario, pocao) ||
        !preparar_fila(jogo)) {
        anotar_evento(jogo, "Memoria insuficiente para iniciar a partida.");
        encerrar_partida(jogo, JOGO_ERRO);
        return false;
    }

    anotar_evento(jogo, "Voce recebeu uma pocao de cura antes de entrar no portal.");
    return true;
}

const Inimigo *jogo_inimigo_atual(Jogo *jogo)
{
    return fila_frente(&jogo->fila);
}

const char *jogo_nome_masmorra(const Jogo *jogo)
{
    return masmorras[jogo->indice_masmorra].nome;
}

size_t jogo_total_masmorras(void)
{
    return sizeof masmorras / sizeof masmorras[0];
}

int jogo_experiencia_para_subir(const Cacador *cacador)
{
    return cacador->nivel * 30;
}

static void contra_atacar(Jogo *jogo, const Inimigo *inimigo)
{
    Cacador *cacador = &jogo->cacador;
    int dano = inimigo->ataque < cacador->vida ? inimigo->ataque : cacador->vida;
    cacador->vida -= dano;
    anotar_evento(jogo, "\n%s causou %d de dano.", inimigo->nome, dano);

    if (cacador->vida == 0) {
        anotar_evento(jogo, "\nDerrota! Seus pontos de vida acabaram.");
        encerrar_partida(jogo, JOGO_DERROTA);
    }
}

static void receber_recompensa(Jogo *jogo, const Inimigo *inimigo)
{
    Cacador *cacador = &jogo->cacador;
    cacador->experiencia += inimigo->experiencia;
    cacador->pontos += inimigo->pontos;
    anotar_evento(jogo, "\n%s foi derrotado. +%d XP e +%d pontos.",
                  inimigo->nome, inimigo->experiencia, inimigo->pontos);

    while (cacador->experiencia >= jogo_experiencia_para_subir(cacador)) {
        cacador->experiencia -= jogo_experiencia_para_subir(cacador);
        ++cacador->nivel;
        cacador->vida_maxima += 12;
        cacador->vida += 12;
        cacador->ataque += 4;
        anotar_evento(jogo, "\nNivel %d alcancado! +12 PV e +4 de ataque.",
                      cacador->nivel);
    }
}

static void concluir_masmorra(Jogo *jogo)
{
    Cacador *cacador = &jogo->cacador;

    cacador->pontos += 100;
    anotar_evento(jogo, "\nMasmorra concluida! +100 pontos. Total: %d", cacador->pontos);

    if (jogo->indice_masmorra + 1 == jogo_total_masmorras()) {
        anotar_evento(jogo, "\nVitoria! Voce derrotou o chefe final.");
        encerrar_partida(jogo, JOGO_VITORIA);
        return;
    }

    if (inventario_adicionar(&cacador->inventario, pocao)) {
        anotar_evento(jogo, "\nVoce coletou uma pocao de cura.");
    } else {
        anotar_evento(jogo, "\nMemoria insuficiente para guardar a pocao encontrada.");
    }

    ++jogo->indice_masmorra;
    if (!preparar_fila(jogo)) {
        anotar_evento(jogo, "\nMemoria insuficiente para preparar a masmorra.");
        encerrar_partida(jogo, JOGO_ERRO);
    }
}

void jogo_atacar(Jogo *jogo)
{
    if (jogo->estado != JOGO_EM_COMBATE) {
        return;
    }

    Inimigo *inimigo = fila_frente(&jogo->fila);
    int ataque = jogo->cacador.ataque;
    int dano = ataque < inimigo->vida ? ataque : inimigo->vida;
    inimigo->vida -= dano;
    jogo->mensagem[0] = '\0';
    anotar_evento(jogo, "Voce causou %d de dano.", dano);

    if (inimigo->vida > 0) {
        contra_atacar(jogo, inimigo);
        return;
    }

    receber_recompensa(jogo, inimigo);
    fila_desenfileirar(&jogo->fila);
    const Inimigo *proximo = jogo_inimigo_atual(jogo);
    if (proximo == NULL) {
        concluir_masmorra(jogo);
    } else {
        jogo->vida_maxima_inimigo = proximo->vida;
    }
}

void jogo_usar_item(Jogo *jogo, size_t posicao)
{
    if (jogo->estado != JOGO_EM_COMBATE) {
        return;
    }

    Cacador *cacador = &jogo->cacador;
    jogo->mensagem[0] = '\0';
    size_t quantidade = inventario_quantidade(&cacador->inventario);
    if (quantidade == 0) {
        anotar_evento(jogo, "O inventario esta vazio.");
        return;
    }
    if (cacador->vida == cacador->vida_maxima) {
        anotar_evento(jogo, "Sua vida ja esta cheia.");
        return;
    }

    Item item;
    if (!inventario_remover(&cacador->inventario, posicao, &item)) {
        anotar_evento(jogo, "Item indisponivel.");
        return;
    }

    int espaco_para_cura = cacador->vida_maxima - cacador->vida;
    int cura = item.cura < espaco_para_cura ? item.cura : espaco_para_cura;
    cacador->vida += cura;
    anotar_evento(jogo, "Voce usou %s e recuperou %d PV.", item.nome, cura);
    contra_atacar(jogo, jogo_inimigo_atual(jogo));
}

void jogo_abandonar(Jogo *jogo)
{
    if (jogo->estado == JOGO_EM_COMBATE) {
        jogo->mensagem[0] = '\0';
        anotar_evento(jogo, "Partida interrompida.");
        encerrar_partida(jogo, JOGO_ABANDONADO);
    }
}
