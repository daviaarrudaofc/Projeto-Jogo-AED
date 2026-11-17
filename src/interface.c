#include "tela.h"

#include <stdio.h>
#include <string.h>

const Rectangle AREA_MOVIMENTO = {64, 180, 868, 302};
const Vector2 POSICAO_PORTAO = {800, 290};
const Vector2 POSICAO_INSCRICAO = {244, 280};

static const Rectangle CAMPO_NOME = {588, 241, 344, 48};
static const float DURACAO_ACAO = 0.28f;

typedef enum {
    SEM_COMANDO, INICIAR, MOSTRAR_RANKING, MOSTRAR_AJUDA, ATACAR, DEFENDER,
    INVOCAR, ABRIR_INVENTARIO, USAR_ITEM, RECUAR, VOLTAR, VOLTAR_COMBATE
} Comando;

typedef struct {
    Comando comando;
    size_t posicao_item;
} Acao;

enum { LIMITE_BOTOES = 8 };

typedef struct {
    TelaJogo tela;
    TelaJogo retorno_ranking;
    Jogo jogo;
    Ranking ranking;
    Personagem personagem;
    AlvoExploracao perto;
    char nome[TAMANHO_NOME];
    const char *aviso;
    BotaoTela botoes[LIMITE_BOTOES];
    Acao acoes[LIMITE_BOTOES];
    char nomes_itens[LIMITE_BOTOES - 1][64];
    size_t quantidade_botoes;
    int foco;
    EfeitoTela efeito;
    float tempo_acao;
    Acao acao_pendente;
} Interface;

static void ir_para(Interface *interface, TelaJogo tela)
{
    interface->tela = tela;
    interface->foco = -1;
    interface->aviso = NULL;
    interface->perto = ALVO_NENHUM;

    if (tela == TELA_ENTRADA || tela == TELA_PORTAL) {
        Vector2 ponto_inicial = tela == TELA_ENTRADA ? (Vector2){134, 407} : (Vector2){484, 407};
        personagem_reiniciar(&interface->personagem, ponto_inicial, 1);
    }
}

static void voltar_para_menu(Interface *interface)
{
    jogo_limpar(&interface->jogo);
    ir_para(interface, TELA_MENU);
}

static void atualizar_partida(Interface *interface)
{
    switch (interface->jogo.estado) {
        case JOGO_EM_COMBATE:
            if (interface->tela != TELA_COMBATE) {
                ir_para(interface, TELA_COMBATE);
            }
            return;
        case JOGO_PORTAL:
            ir_para(interface, TELA_PORTAL);
            return;
        case JOGO_VITORIA:
        case JOGO_DERROTA:
            ranking_registrar(&interface->ranking, interface->nome, interface->jogo.cacador.pontos);
            break;
        case JOGO_ABANDONADO:
        case JOGO_ERRO:
            break;
    }
    ir_para(interface, TELA_RESULTADO);
}

static void iniciar_partida(Interface *interface)
{
    char primeira_letra;
    if (sscanf(interface->nome, " %c", &primeira_letra) != 1) {
        interface->foco = -1;
        interface->aviso = "Digite o nome do caçador.";
        return;
    }

    if (jogo_iniciar(&interface->jogo)) {
        ir_para(interface, TELA_ENTRADA);
    } else {
        atualizar_partida(interface);
    }
}

static void adicionar_botao(Interface *interface, Rectangle area, const char *texto,
                            TipoBotao tipo, bool habilitado, Comando comando, size_t posicao)
{
    size_t indice = interface->quantidade_botoes++;
    interface->botoes[indice] = (BotaoTela){area, texto, tipo, habilitado,
                                          habilitado && interface->foco == (int)indice};
    interface->acoes[indice] = (Acao){comando, posicao};
}

static void adicionar_itens(Interface *interface)
{
    const Inventario *inventario = &interface->jogo.cacador.inventario;
    size_t total = inventario_quantidade(inventario);
    for (size_t posicao = 1; posicao <= total && posicao < LIMITE_BOTOES; ++posicao) {
        Item item;
        if (inventario_buscar(inventario, posicao, &item)) {
            char *texto = interface->nomes_itens[posicao - 1];
            snprintf(texto, sizeof interface->nomes_itens[0], "%s  |  +%d PV", item.nome, item.cura);
            Rectangle area = {60, 142 + 53 * (float)(posicao - 1), 880, 42};
            adicionar_botao(interface, area, texto, BOTAO_SECUNDARIO, true, USAR_ITEM, posicao);
        }
    }
}

static void montar_botoes(Interface *interface)
{
    interface->quantidade_botoes = 0;
    const Rectangle primeiro = {40, 568, 246, 46};
    const Rectangle segundo = {300, 568, 246, 46};
    const Rectangle ultimo = {760, 568, 200, 46};

    switch (interface->tela) {
        case TELA_MENU:
            adicionar_botao(interface, (Rectangle){588, 302, 344, 48}, "Iniciar caça",
                            BOTAO_PRIMARIO, true, INICIAR, 0);
            adicionar_botao(interface, (Rectangle){588, 364, 168, 40}, "Ranking",
                            BOTAO_SECUNDARIO, true, MOSTRAR_RANKING, 0);
            adicionar_botao(interface, (Rectangle){764, 364, 168, 40}, "Como jogar",
                            BOTAO_SECUNDARIO, true, MOSTRAR_AJUDA, 0);
            break;
        case TELA_ENTRADA:
        case TELA_PORTAL:
            adicionar_botao(interface, ultimo, "Recuar", BOTAO_PERIGO, true, RECUAR, 0);
            break;
        case TELA_COMBATE: {
            const char *nomes[] = {"A  Atacar", "D  Defender", "E  Erga-se", "I  Inventário", "Recuar"};
            const Comando comandos[] = {ATACAR, DEFENDER, INVOCAR, ABRIR_INVENTARIO, RECUAR};
            const TipoBotao tipos[] = {BOTAO_PRIMARIO, BOTAO_SECUNDARIO, BOTAO_SECUNDARIO,
                                       BOTAO_SECUNDARIO, BOTAO_PERIGO};
            for (size_t indice = 0; indice < sizeof comandos / sizeof comandos[0]; ++indice) {
                Rectangle area = {40 + 186 * (float)indice, 568, 176, 46};
                bool habilitado = interface->acao_pendente.comando == SEM_COMANDO &&
                    (comandos[indice] != INVOCAR || interface->jogo.cacador.essencia > 0);
                adicionar_botao(interface, area, nomes[indice], tipos[indice], habilitado,
                                comandos[indice], 0);
            }
            break;
        }
        case TELA_INVENTARIO:
            adicionar_itens(interface);
            adicionar_botao(interface, primeiro, "Voltar ao combate", BOTAO_SECUNDARIO,
                            true, VOLTAR_COMBATE, 0);
            break;
        case TELA_RANKING:
        case TELA_AJUDA:
            adicionar_botao(interface, primeiro, "Voltar", BOTAO_SECUNDARIO, true, VOLTAR, 0);
            break;
        case TELA_RESULTADO:
            adicionar_botao(interface, primeiro, "Voltar ao menu", BOTAO_PRIMARIO, true, VOLTAR, 0);
            adicionar_botao(interface, segundo, "Ver ranking", BOTAO_SECUNDARIO, true, MOSTRAR_RANKING, 0);
            break;
    }
}

static void preparar_acao(Interface *interface, Acao acao, EfeitoTela efeito)
{
    if (interface->tela == TELA_INVENTARIO) {
        ir_para(interface, TELA_COMBATE);
    }
    interface->acao_pendente = acao;
    interface->efeito = efeito;
    interface->tempo_acao = 0;
}

static void executar_acao(Interface *interface, Acao acao)
{
    switch (acao.comando) {
        case INICIAR:
            iniciar_partida(interface);
            break;
        case MOSTRAR_RANKING:
            interface->retorno_ranking = interface->tela;
            ir_para(interface, TELA_RANKING);
            break;
        case MOSTRAR_AJUDA:
            ir_para(interface, TELA_AJUDA);
            break;
        case ATACAR:
            preparar_acao(interface, acao, EFEITO_ATAQUE);
            break;
        case DEFENDER:
            preparar_acao(interface, acao, EFEITO_DEFESA);
            break;
        case INVOCAR:
            if (interface->jogo.cacador.essencia == 0) {
                jogo_invocar_sombra(&interface->jogo);
            } else {
                preparar_acao(interface, acao, EFEITO_SOMBRA);
            }
            break;
        case ABRIR_INVENTARIO:
            ir_para(interface, TELA_INVENTARIO);
            break;
        case USAR_ITEM:
            preparar_acao(interface, acao, EFEITO_POCAO);
            break;
        case RECUAR:
            jogo_abandonar(&interface->jogo);
            atualizar_partida(interface);
            break;
        case VOLTAR:
            if (interface->tela == TELA_RANKING) {
                ir_para(interface, interface->retorno_ranking);
            } else {
                voltar_para_menu(interface);
            }
            break;
        case VOLTAR_COMBATE:
            ir_para(interface, TELA_COMBATE);
            break;
        case SEM_COMANDO:
            break;
    }
}

static void atualizar_acao(Interface *interface, float segundos)
{
    Acao acao = interface->acao_pendente;
    if (acao.comando == SEM_COMANDO) {
        return;
    }

    interface->tempo_acao += segundos;
    if (interface->tempo_acao < DURACAO_ACAO) {
        return;
    }

    switch (acao.comando) {
        case ATACAR: jogo_atacar(&interface->jogo); break;
        case DEFENDER: jogo_defender(&interface->jogo); break;
        case INVOCAR: jogo_invocar_sombra(&interface->jogo); break;
        case USAR_ITEM: jogo_usar_item(&interface->jogo, acao.posicao_item); break;
        default: break;
    }

    interface->acao_pendente = (Acao){SEM_COMANDO, 0};
    interface->efeito = EFEITO_NENHUM;
    interface->tempo_acao = 0;
    atualizar_partida(interface);
}

static void ler_nome(Interface *interface)
{
    size_t tamanho = strlen(interface->nome);
    if (tamanho > 0 && IsKeyPressed(KEY_BACKSPACE)) {
        int bytes_letra;
        GetCodepointPrevious(interface->nome + tamanho, &bytes_letra);
        tamanho -= (size_t)bytes_letra;
        interface->nome[tamanho] = '\0';
        interface->aviso = NULL;
    }

    for (int letra = GetCharPressed(); letra != 0; letra = GetCharPressed()) {
        int bytes;
        const char *texto = CodepointToUTF8(letra, &bytes);
        if (letra >= 32 && tamanho + (size_t)bytes < sizeof interface->nome) {
            memcpy(interface->nome + tamanho, texto, (size_t)bytes);
            tamanho += (size_t)bytes;
            interface->nome[tamanho] = '\0';
            interface->aviso = NULL;
        }
    }
}

static AlvoExploracao localizar_alvo(const Interface *interface)
{
    Vector2 posicao = interface->personagem.posicao;
    if (CheckCollisionPointCircle(posicao, POSICAO_PORTAO, DISTANCIA_INTERACAO)) {
        return ALVO_PORTAO;
    }
    if (interface->tela == TELA_PORTAL &&
        CheckCollisionPointCircle(posicao, POSICAO_INSCRICAO, DISTANCIA_INTERACAO)) {
        return ALVO_INSCRICAO;
    }
    return ALVO_NENHUM;
}

static void interagir(Interface *interface)
{
    if (interface->perto == ALVO_NENHUM) {
        interface->aviso = "Chegue mais perto para interagir.";
    } else if (interface->tela == TELA_ENTRADA) {
        ir_para(interface, TELA_COMBATE);
    } else if (interface->perto == ALVO_PORTAO) {
        jogo_escolher_portal(&interface->jogo, false);
        atualizar_partida(interface);
    } else if (!interface->jogo.portal_examinado) {
        jogo_examinar_portal(&interface->jogo);
    } else {
        jogo_escolher_portal(&interface->jogo, true);
        atualizar_partida(interface);
    }
}

static void explorar(Interface *interface, float segundos)
{
    int direita = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
    int esquerda = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
    int baixo = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    int cima = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
    Vector2 eixo = {(float)(direita - esquerda), (float)(baixo - cima)};
    personagem_mover(&interface->personagem, eixo, segundos, AREA_MOVIMENTO);
    interface->perto = localizar_alvo(interface);
    if (interface->perto != ALVO_NENHUM) {
        interface->aviso = NULL;
    }
    if (IsKeyPressed(KEY_F)) {
        interagir(interface);
    }
}

static bool processar_mouse(Interface *interface)
{
    Vector2 mouse = GetMousePosition();
    for (size_t indice = 0; indice < interface->quantidade_botoes; ++indice) {
        BotaoTela *botao = &interface->botoes[indice];
        if (botao->habilitado && CheckCollisionPointRec(mouse, botao->area)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                interface->foco = (int)indice;
                executar_acao(interface, interface->acoes[indice]);
                return true;
            }
            break;
        }
    }
    if (interface->tela == TELA_MENU && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mouse, CAMPO_NOME)) {
        interface->foco = -1;
    }
    return false;
}

static void avancar_foco(Interface *interface)
{
    int total = (int)interface->quantidade_botoes;
    int proximo = interface->foco;
    for (int tentativa = 0; tentativa <= total; ++tentativa) {
        ++proximo;
        if (proximo >= total) {
            if (interface->tela == TELA_MENU) {
                interface->foco = -1;
                return;
            }
            proximo = 0;
        }
        if (interface->botoes[proximo].habilitado) {
            interface->foco = proximo;
            return;
        }
    }
}

static bool processar_teclado(Interface *interface)
{
    if (IsKeyPressed(KEY_TAB)) {
        avancar_foco(interface);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (interface->tela == TELA_INVENTARIO) {
            ir_para(interface, TELA_COMBATE);
        } else if (interface->tela == TELA_RANKING || interface->tela == TELA_AJUDA) {
            executar_acao(interface, (Acao){VOLTAR, 0});
        }
        return true;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        int foco = interface->foco;
        if (foco >= 0 && foco < (int)interface->quantidade_botoes && interface->botoes[foco].habilitado) {
            executar_acao(interface, interface->acoes[foco]);
            return true;
        }
        if (interface->tela == TELA_MENU) {
            iniciar_partida(interface);
            return true;
        }
    }
    if (interface->tela != TELA_COMBATE) {
        return false;
    }

    Comando comando = SEM_COMANDO;
    if (IsKeyPressed(KEY_A)) comando = ATACAR;
    else if (IsKeyPressed(KEY_D)) comando = DEFENDER;
    else if (IsKeyPressed(KEY_E)) comando = INVOCAR;
    else if (IsKeyPressed(KEY_I)) comando = ABRIR_INVENTARIO;
    executar_acao(interface, (Acao){comando, 0});
    return comando != SEM_COMANDO;
}

static void atualizar_interface(Interface *interface, float segundos)
{
    if (interface->acao_pendente.comando != SEM_COMANDO) {
        atualizar_acao(interface, segundos);
        return;
    }

    montar_botoes(interface);
    if (processar_mouse(interface) || processar_teclado(interface)) {
        return;
    }
    if (interface->tela == TELA_MENU && interface->foco == -1) {
        ler_nome(interface);
    } else if (interface->tela == TELA_ENTRADA || interface->tela == TELA_PORTAL) {
        explorar(interface, segundos);
    }
}

static void desenhar_interface(Interface *interface, float tempo)
{
    montar_botoes(interface);
    Vector2 mouse = GetMousePosition();
    for (size_t indice = 0; indice < interface->quantidade_botoes; ++indice) {
        BotaoTela *botao = &interface->botoes[indice];
        if (botao->habilitado && CheckCollisionPointRec(mouse, botao->area)) {
            botao->selecionado = true;
        }
    }

    QuadroTela quadro = {
        .tela = interface->tela,
        .jogo = &interface->jogo,
        .ranking = &interface->ranking,
        .nome = interface->nome,
        .aviso = interface->aviso,
        .editando_nome = interface->foco == -1,
        .personagem = &interface->personagem,
        .perto = interface->perto,
        .efeito = interface->efeito,
        .progresso_efeito = interface->tempo_acao / DURACAO_ACAO,
        .tempo = tempo,
        .botoes = interface->botoes,
        .quantidade_botoes = interface->quantidade_botoes
    };
    tela_desenhar(&quadro);
}

static Rectangle ajustar_tela(void)
{
    int largura_janela = GetScreenWidth();
    int altura_janela = GetScreenHeight();
    float escala = (float)largura_janela / LARGURA_TELA;
    float escala_vertical = (float)altura_janela / ALTURA_TELA;
    if (escala_vertical < escala) {
        escala = escala_vertical;
    }
    if (escala <= 0.0f) {
        escala = 1.0f;
    }

    float largura_jogo = LARGURA_TELA * escala;
    float altura_jogo = ALTURA_TELA * escala;
    int margem_x = (int)((largura_janela - largura_jogo) / 2);
    int margem_y = (int)((altura_janela - altura_jogo) / 2);
    SetMouseOffset(-margem_x, -margem_y);
    SetMouseScale(1.0f / escala, 1.0f / escala);
    return (Rectangle){margem_x, margem_y, largura_jogo, altura_jogo};
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Solo Leveling: Dungeon Quest");
    if (!IsWindowReady()) {
        return 1;
    }

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    RenderTexture2D imagem_jogo = LoadRenderTexture(LARGURA_TELA, ALTURA_TELA);
    if (!IsRenderTextureValid(imagem_jogo)) {
        CloseWindow();
        return 1;
    }
    Rectangle imagem_origem = {0, 0, LARGURA_TELA, -ALTURA_TELA};

    Interface interface = {0};
    ir_para(&interface, TELA_MENU);
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F11)) {
            ToggleBorderlessWindowed();
        }
        Rectangle area = ajustar_tela();
        atualizar_interface(&interface, GetFrameTime());
        BeginTextureMode(imagem_jogo);
        desenhar_interface(&interface, (float)GetTime());
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(imagem_jogo.texture, imagem_origem, area, (Vector2){0, 0}, 0, WHITE);
        EndDrawing();
    }

    jogo_limpar(&interface.jogo);
    UnloadRenderTexture(imagem_jogo);
    CloseWindow();
    return 0;
}
