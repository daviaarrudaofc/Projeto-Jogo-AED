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

typedef enum {
    TUNEL_DO_PORTAL,
    MASMORRA_DUPLA,
    TEMPLO_DA_ESTATUA
} Etapa;

static const Item pocao = {"Pocao de cura", 30};

static const Masmorra masmorras[] = {
    {"Tunel do Portal", {
        {"Orc do Tunel", 33, 8, 20, 160, false},
        {"Goblin do Portal", 20, 6, 15, 100, false}
    }, 2},
    {"Masmorra Dupla", {
        {"Cavaleiro Vermelho", 58, 11, 35, 340, false}
    }, 1},
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

static void iniciar_turnos(Jogo *jogo, const Inimigo *inimigo)
{
    jogo->vida_maxima_inimigo = inimigo->vida;
    jogo->turno_inimigo = 0;
    jogo->chefe_em_furia = false;
}

static bool preparar_fila(Jogo *jogo)
{
    /* Uso uma cópia pra ordenar os inimigos sem mexer no cadastro da masmorra. */
    Masmorra masmorra = masmorras[jogo->indice_masmorra];
    quicksort(masmorra.inimigos, masmorra.quantidade,
              sizeof masmorra.inimigos[0], comparar_inimigos);

    for (int indice = 0; indice < masmorra.quantidade; ++indice) {
        if (!fila_enfileirar(&jogo->fila, masmorra.inimigos[indice])) {
            return false;
        }
    }

    iniciar_turnos(jogo, fila_frente(&jogo->fila));
    return true;
}

static bool partida_ativa(const Jogo *jogo)
{
    return jogo->estado == JOGO_EM_COMBATE || jogo->estado == JOGO_PORTAL;
}

void jogo_limpar(Jogo *jogo)
{
    fila_limpar(&jogo->fila);
    inventario_limpar(&jogo->cacador.inventario);
    jogo->vida_maxima_inimigo = 0;
    if (partida_ativa(jogo)) {
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
        .cacador = {.vida = 75, .vida_maxima = 75, .ataque = 15, .nivel = 1,
                    .essencia = 1},
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

    anotar_evento(jogo, "Sistema: cacador Rank E. Voce recebeu uma pocao de cura.");
    return true;
}

const Inimigo *jogo_inimigo_atual(const Jogo *jogo)
{
    return fila_frente(&jogo->fila);
}

const char *jogo_nome_masmorra(const Jogo *jogo)
{
    return masmorras[jogo->indice_masmorra].nome;
}

int jogo_experiencia_para_subir(const Cacador *cacador)
{
    return cacador->nivel * 30;
}

bool jogo_golpe_carregado(const Jogo *jogo)
{
    return jogo->estado == JOGO_EM_COMBATE && jogo->turno_inimigo % 2 == 1;
}

int jogo_dano_previsto(const Jogo *jogo)
{
    const Inimigo *inimigo = jogo_inimigo_atual(jogo);
    if (inimigo == NULL) {
        return 0;
    }

    int dano = inimigo->ataque + (jogo->chefe_em_furia ? 4 : 0);
    return jogo_golpe_carregado(jogo) ? dano * 2 : dano;
}

int jogo_dano_sombra(const Jogo *jogo)
{
    return jogo->cacador.ataque + (jogo->cacador.tem_igris ? 18 : 10);
}

static void recuperar_essencia(Jogo *jogo)
{
    if (jogo->cacador.essencia < ESSENCIA_MAXIMA) {
        ++jogo->cacador.essencia;
        anotar_evento(jogo, " +1 essencia.");
    }
}

static void contra_atacar(Jogo *jogo, const Inimigo *inimigo, bool defendendo)
{
    Cacador *cacador = &jogo->cacador;
    /* Pego a previsão antes de avançar o turno pra aplicar o golpe que apareceu na tela. */
    bool carregado = jogo_golpe_carregado(jogo);
    int dano = jogo_dano_previsto(jogo);
    ++jogo->turno_inimigo;

    if (defendendo && carregado) {
        anotar_evento(jogo, "O golpe carregado foi anulado.");
        recuperar_essencia(jogo);
    } else {
        if (defendendo) {
            dano = (dano + 1) / 2;
        }
        dano = dano < cacador->vida ? dano : cacador->vida;
        cacador->vida -= dano;
        anotar_evento(jogo, "%s causou %d de dano%s.", inimigo->nome, dano,
                      defendendo ? " apos a defesa" : "");
    }

    if (cacador->vida == 0) {
        anotar_evento(jogo, "\nDerrota! Seus pontos de vida acabaram.");
        encerrar_partida(jogo, JOGO_DERROTA);
    } else if (inimigo->chefe && inimigo->vida <= jogo->vida_maxima_inimigo / 2 &&
               !jogo->chefe_em_furia) {
        /* A fúria começa depois da resposta atual, então muda o dano do próximo golpe. */
        jogo->chefe_em_furia = true;
        anotar_evento(jogo, "\nA Estatua entrou em furia. Seus golpes ficaram mais fortes.");
    }
}

static void receber_recompensa(Jogo *jogo, const Inimigo *inimigo)
{
    Cacador *cacador = &jogo->cacador;
    cacador->experiencia += inimigo->experiencia;
    cacador->pontos += inimigo->pontos;
    anotar_evento(jogo, "\n%s foi derrotado. +%d XP e +%d pontos.",
                  inimigo->nome, inimigo->experiencia, inimigo->pontos);

    /* Gasto só o XP necessário pra subir. O que sobra já conta pro próximo nível. */
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

static void entrar_masmorra(Jogo *jogo, Etapa etapa)
{
    jogo->indice_masmorra = etapa;
    if (preparar_fila(jogo)) {
        jogo->estado = JOGO_EM_COMBATE;
    } else {
        anotar_evento(jogo, "\nMemoria insuficiente para preparar a masmorra.");
        encerrar_partida(jogo, JOGO_ERRO);
    }
}

static void concluir_masmorra(Jogo *jogo)
{
    Cacador *cacador = &jogo->cacador;

    cacador->pontos += 100;
    anotar_evento(jogo, "\nMasmorra concluida! +100 pontos. Total: %d", cacador->pontos);

    if (jogo->indice_masmorra == TEMPLO_DA_ESTATUA) {
        anotar_evento(jogo, "\nVitoria! Voce derrotou o chefe final.");
        encerrar_partida(jogo, JOGO_VITORIA);
        return;
    }

    if (jogo->indice_masmorra == MASMORRA_DUPLA) {
        cacador->tem_igris = true;
        anotar_evento(jogo, "\nErga-se! A Sombra de Igris agora luta ao seu lado.");
        recuperar_essencia(jogo);
        entrar_masmorra(jogo, TEMPLO_DA_ESTATUA);
        return;
    }

    if (inventario_adicionar(&cacador->inventario, pocao)) {
        anotar_evento(jogo, "\nVoce coletou uma pocao de cura.");
    } else {
        anotar_evento(jogo, "\nMemoria insuficiente para guardar a pocao encontrada.");
    }

    /* A fila já ficou vazia. O jogo espera a escolha da rota antes de voltar ao combate. */
    jogo->estado = JOGO_PORTAL;
    jogo->vida_maxima_inimigo = 0;
    anotar_evento(jogo, "\nSistema: ha uma inscricao ao lado do portal do Templo.");
}

static void aplicar_golpe(Jogo *jogo, const char *origem, int ataque)
{
    Inimigo *inimigo = fila_frente(&jogo->fila);
    int dano = ataque < inimigo->vida ? ataque : inimigo->vida;
    inimigo->vida -= dano;
    anotar_evento(jogo, "%s causou %d de dano.", origem, dano);

    if (inimigo->vida > 0) {
        anotar_evento(jogo, "\n");
        contra_atacar(jogo, inimigo, false);
        return;
    }

    /* Pego a recompensa primeiro porque tirar o inimigo da fila libera a memória do nó. */
    receber_recompensa(jogo, inimigo);
    fila_desenfileirar(&jogo->fila);
    const Inimigo *proximo = jogo_inimigo_atual(jogo);
    if (proximo == NULL) {
        concluir_masmorra(jogo);
    } else {
        iniciar_turnos(jogo, proximo);
    }
}

void jogo_atacar(Jogo *jogo)
{
    if (jogo->estado == JOGO_EM_COMBATE) {
        jogo->mensagem[0] = '\0';
        aplicar_golpe(jogo, "Voce", jogo->cacador.ataque);
    }
}

void jogo_defender(Jogo *jogo)
{
    if (jogo->estado == JOGO_EM_COMBATE) {
        jogo->mensagem[0] = '\0';
        contra_atacar(jogo, jogo_inimigo_atual(jogo), true);
    }
}

void jogo_invocar_sombra(Jogo *jogo)
{
    if (jogo->estado != JOGO_EM_COMBATE) {
        return;
    }

    jogo->mensagem[0] = '\0';
    if (jogo->cacador.essencia == 0) {
        anotar_evento(jogo, "Sem essencia. Defenda um golpe carregado para recuperar uma carga.");
        return;
    }

    --jogo->cacador.essencia;
    const char *origem = jogo->cacador.tem_igris ? "Erga-se! Igris" : "Erga-se! Sua sombra";
    aplicar_golpe(jogo, origem, jogo_dano_sombra(jogo));
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
    anotar_evento(jogo, "Voce usou %s e recuperou %d PV.\n", item.nome, cura);
    contra_atacar(jogo, jogo_inimigo_atual(jogo), false);
}

void jogo_examinar_portal(Jogo *jogo)
{
    if (jogo->estado == JOGO_PORTAL) {
        jogo->portal_examinado = true;
        jogo->mensagem[0] = '\0';
        anotar_evento(jogo, "A inscricao revela uma entrada para a Masmorra Dupla.\n"
                     "A sombra do Cavaleiro Vermelho pode ser extraida. "
                     "Voce tera de vence-lo primeiro.");
    }
}

void jogo_escolher_portal(Jogo *jogo, bool investigar)
{
    if (jogo->estado != JOGO_PORTAL) {
        return;
    }

    jogo->mensagem[0] = '\0';
    if (investigar && !jogo->portal_examinado) {
        anotar_evento(jogo, "Examine a inscricao primeiro.");
        return;
    }

    Etapa destino = investigar ? MASMORRA_DUPLA : TEMPLO_DA_ESTATUA;
    anotar_evento(jogo, "Voce entrou em %s.", masmorras[destino].nome);
    entrar_masmorra(jogo, destino);
}

void jogo_abandonar(Jogo *jogo)
{
    if (partida_ativa(jogo)) {
        jogo->mensagem[0] = '\0';
        anotar_evento(jogo, "Partida interrompida.");
        encerrar_partida(jogo, JOGO_ABANDONADO);
    }
}
