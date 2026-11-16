#include "jogo.h"

#include "ordenacao.h"

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

static void anotar_evento(Jogo *jogo, const char *texto)
{
    size_t espaco = sizeof jogo->mensagem - strlen(jogo->mensagem) - 1;
    strncat(jogo->mensagem, texto, espaco);
}

static void iniciar_turnos(Jogo *jogo, const Inimigo *inimigo)
{
    jogo->vida_maxima_inimigo = inimigo->vida;
    jogo->turno_inimigo = 0;
    jogo->chefe_em_furia = false;
}

static bool preparar_fila(Jogo *jogo)
{
    Masmorra masmorra = masmorras[jogo->indice_masmorra];
    quicksort_inimigos(masmorra.inimigos, 0, masmorra.quantidade - 1);

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
    Jogo novo = {0};
    novo.cacador.vida = 75;
    novo.cacador.vida_maxima = 75;
    novo.cacador.ataque = 15;
    novo.cacador.nivel = 1;
    novo.cacador.essencia = 1;
    novo.estado = JOGO_EM_COMBATE;
    *jogo = novo;
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
        char texto[TAMANHO_MENSAGEM_JOGO];
        snprintf(texto, sizeof texto, "%s causou %d de dano%s.", inimigo->nome,
                 dano, defendendo ? " apos a defesa" : "");
        anotar_evento(jogo, texto);
    }

    if (cacador->vida == 0) {
        anotar_evento(jogo, "\nDerrota! Seus pontos de vida acabaram.");
        encerrar_partida(jogo, JOGO_DERROTA);
    } else if (inimigo->chefe && inimigo->vida <= jogo->vida_maxima_inimigo / 2 &&
               !jogo->chefe_em_furia) {
        jogo->chefe_em_furia = true;
        anotar_evento(jogo, "\nA Estatua entrou em furia. Seus golpes ficaram mais fortes.");
    }
}

static void receber_recompensa(Jogo *jogo, const Inimigo *inimigo)
{
    Cacador *cacador = &jogo->cacador;
    char texto[TAMANHO_MENSAGEM_JOGO];
    cacador->experiencia += inimigo->experiencia;
    cacador->pontos += inimigo->pontos;
    snprintf(texto, sizeof texto, "\n%s foi derrotado. +%d XP e +%d pontos.",
             inimigo->nome, inimigo->experiencia, inimigo->pontos);
    anotar_evento(jogo, texto);

    while (cacador->experiencia >= jogo_experiencia_para_subir(cacador)) {
        cacador->experiencia -= jogo_experiencia_para_subir(cacador);
        ++cacador->nivel;
        cacador->vida_maxima += 12;
        cacador->vida += 12;
        cacador->ataque += 4;
        snprintf(texto, sizeof texto, "\nNivel %d alcancado! +12 PV e +4 de ataque.",
                 cacador->nivel);
        anotar_evento(jogo, texto);
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
    char texto[TAMANHO_MENSAGEM_JOGO];

    cacador->pontos += 100;
    snprintf(texto, sizeof texto, "\nMasmorra concluida! +100 pontos. Total: %d",
             cacador->pontos);
    anotar_evento(jogo, texto);

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

    jogo->estado = JOGO_PORTAL;
    jogo->vida_maxima_inimigo = 0;
    anotar_evento(jogo, "\nSistema: ha uma inscricao ao lado do portal do Templo.");
}

static void aplicar_golpe(Jogo *jogo, const char *origem, int ataque)
{
    Inimigo *inimigo = fila_frente(&jogo->fila);
    char texto[TAMANHO_MENSAGEM_JOGO];
    int dano = ataque < inimigo->vida ? ataque : inimigo->vida;
    inimigo->vida -= dano;
    snprintf(texto, sizeof texto, "%s causou %d de dano.", origem, dano);
    anotar_evento(jogo, texto);

    if (inimigo->vida > 0) {
        anotar_evento(jogo, "\n");
        contra_atacar(jogo, inimigo, false);
        return;
    }

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
    char texto[TAMANHO_MENSAGEM_JOGO];
    snprintf(texto, sizeof texto, "Voce usou %s e recuperou %d PV.\n", item.nome, cura);
    anotar_evento(jogo, texto);
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
    char texto[TAMANHO_MENSAGEM_JOGO];
    snprintf(texto, sizeof texto, "Voce entrou em %s.", masmorras[destino].nome);
    anotar_evento(jogo, texto);
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
