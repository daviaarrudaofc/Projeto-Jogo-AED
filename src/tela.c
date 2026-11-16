#include "tela.h"

#include <stdio.h>
#include <string.h>

static const Color fundo = {8, 12, 23, 255};
static const Color painel_escuro = {17, 23, 42, 255};
static const Color campo = {25, 30, 52, 255};
static const Color borda = {56, 61, 95, 255};
static const Color texto = {241, 241, 255, 255};
static const Color suave = {167, 178, 206, 255};
static const Color portal = {179, 150, 252, 255};
static const Color vida = {98, 229, 188, 255};
static const Color dano = {255, 126, 158, 255};
static const Color alerta = {255, 204, 128, 255};

static float parte_fracionaria(float valor)
{
    return valor - (int)valor;
}

static float subir_e_descer(float progresso)
{
    if (progresso < 0.5f) {
        return 2.0f * progresso;
    }
    return 2.0f * (1.0f - progresso);
}

static void painel(Rectangle area, Color interior, Color contorno)
{
    Rectangle sombra = {area.x, area.y + 5, area.width, area.height};
    Rectangle corpo = {area.x + 1, area.y + 1, area.width - 2, area.height - 2};
    DrawRectangleRounded(sombra, 0.08f, 8, Fade(BLACK, 0.42f));
    DrawRectangleRounded(area, 0.08f, 8, contorno);
    DrawRectangleRounded(corpo, 0.08f, 8, interior);
}

static void texto_central(const char *valor, Rectangle area, int tamanho, Color cor)
{
    int largura = MeasureText(valor, tamanho);
    DrawText(valor, (int)(area.x + (area.width - largura) / 2),
             (int)(area.y + (area.height - tamanho) / 2), tamanho, cor);
}

static void texto_direita(const char *valor, int limite_direito, int linha_y, int tamanho, Color cor)
{
    DrawText(valor, limite_direito - MeasureText(valor, tamanho), linha_y, tamanho, cor);
}

static void texto_recortado(const char *valor, Rectangle area, int tamanho, Color cor)
{
    BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);
    DrawText(valor, (int)area.x, (int)area.y, tamanho, cor);
    EndScissorMode();
}

static void texto_bloco(const char *valor, Rectangle area, int tamanho, int passo, Color cor)
{
    char linha[TAMANHO_MENSAGEM_JOGO] = {0};
    size_t ocupados = 0;
    int linha_y = (int)area.y;
    int fim = (int)(area.y + area.height);

    while (*valor != '\0' && linha_y + tamanho <= fim) {
        size_t palavra = strcspn(valor, " \n");
        if (palavra == 0) {
            if (*valor == '\n') {
                DrawText(linha, (int)area.x, linha_y, tamanho, cor);
                linha_y += passo;
                ocupados = 0;
                linha[0] = '\0';
            }
            ++valor;
            continue;
        }

        size_t anterior = ocupados;
        if (ocupados + 1 >= sizeof linha) {
            DrawText(linha, (int)area.x, linha_y, tamanho, cor);
            linha_y += passo;
            ocupados = 0;
            linha[0] = '\0';
            continue;
        }
        if (ocupados != 0) {
            linha[ocupados++] = ' ';
        }
        size_t disponivel = sizeof linha - ocupados - 1;
        size_t tamanho_palavra = palavra < disponivel ? palavra : disponivel;
        memcpy(linha + ocupados, valor, tamanho_palavra);
        ocupados += tamanho_palavra;
        linha[ocupados] = '\0';

        if (anterior != 0 && (MeasureText(linha, tamanho) > area.width || tamanho_palavra < palavra)) {
            linha[anterior] = '\0';
            DrawText(linha, (int)area.x, linha_y, tamanho, cor);
            linha_y += passo;
            ocupados = 0;
            linha[0] = '\0';
        } else {
            valor += palavra;
        }
    }
    if (ocupados != 0 && linha_y + tamanho <= fim) {
        DrawText(linha, (int)area.x, linha_y, tamanho, cor);
    }
}

static void etiqueta(const char *valor, Rectangle area, Color cor)
{
    DrawRectangleRounded(area, 0.25f, 8, campo);
    DrawRectangleRounded(area, 0.25f, 8, Fade(cor, 0.13f));
    texto_central(valor, area, 13, cor);
}

static void fundo_tela(void)
{
    ClearBackground(fundo);
    DrawRectangleGradientV(0, 0, LARGURA_TELA, ALTURA_TELA,
                           (Color){17, 18, 40, 255}, fundo);
    Vector2 centro = {932, -64};
    for (int raio = 96; raio <= 380; raio += 47) {
        DrawCircleLinesV(centro, (float)raio, Fade(portal, 0.065f));
    }
    DrawRectangleGradientH(0, 0, LARGURA_TELA, 3, portal, Fade(portal, 0.14f));
    DrawRectangle(40, 624, 920, 1, Fade(borda, 0.65f));
}

static void cabecalho(const char *categoria, const char *titulo, const char *subtitulo)
{
    DrawText(categoria, 41, 20, 13, portal);
    DrawText(titulo, 40, 40, 37, texto);
    if (subtitulo != NULL) {
        DrawText(subtitulo, 42, 87, 17, suave);
    }
    DrawRectangle(40, 111, 920, 1, Fade(borda, 0.84f));
}

static void selo_portal(Vector2 centro, float escala, float tempo, bool ativo)
{
    const Vector2 pontos[] = {
        {-66, -53}, {-30, -89}, {4, -98}, {35, -94}, {65, -72}, {77, -32},
        {92, -10}, {77, 35}, {61, 75}, {33, 92}, {5, 102}, {-35, 86},
        {-69, 71}, {-90, 40}, {-83, -6}, {-92, -28}
    };
    float pulso = subir_e_descer(parte_fracionaria(tempo * 0.45f));
    int centro_x = (int)centro.x;
    int centro_y = (int)centro.y;
    for (int camada = 0; camada < 5; ++camada) {
        float largura = (63 - 9 * camada) * escala;
        float altura = (89 - 11 * camada) * escala;
        DrawEllipse(centro_x, centro_y, largura, altura, Fade(portal, 0.08f + camada * 0.026f));
        DrawEllipseLines(centro_x, centro_y, largura, altura, Fade(portal, 0.25f + 0.07f * camada));
    }

    Vector2 topo = {centro.x + 6 * escala, centro.y - 55 * escala};
    Vector2 dobra = {centro.x - 10 * escala, centro.y - 3 * escala};
    Vector2 quina = {centro.x + 12 * escala, centro.y - 12 * escala};
    Vector2 base = {centro.x - 9 * escala, centro.y + 53 * escala};
    Color brilho = Fade(texto, 0.66f + pulso * 0.3f);
    DrawLineEx(topo, dobra, 3 * escala, brilho);
    DrawLineEx(dobra, quina, 3 * escala, brilho);
    DrawLineEx(quina, base, 3 * escala, brilho);

    for (size_t indice = 0; indice < sizeof pontos / sizeof pontos[0]; ++indice) {
        float distancia = escala * (0.91f + 0.12f * pulso);
        Vector2 particula = {centro.x + pontos[indice].x * distancia,
                             centro.y + pontos[indice].y * distancia};
        DrawCircleV(particula, (indice % 3 == 0 ? 2.8f : 1.5f) * escala,
                    Fade(portal, (ativo ? 0.52f : 0.30f) + 0.3f * pulso));
    }
}

static void menu(const QuadroTela *quadro)
{
    painel((Rectangle){40, 76, 920, 486}, Fade(painel_escuro, 0.70f), borda);
    DrawRectangle(41, 77, 4, 484, Fade(portal, 0.70f));
    DrawLine(529, 101, 529, 539, Fade(borda, 0.65f));

    DrawText("Solo Leveling", 70, 117, 54, texto);
    DrawText("Dungeon Quest", 73, 179, 41, portal);
    texto_bloco("Atravesse os portais, evolua como caçador e enfrente a Estátua Divina.",
                (Rectangle){75, 244, 390, 62}, 20, 24, suave);

    for (int raio = 76; raio <= 155; raio += 26) {
        DrawCircleLinesV((Vector2){285, 425}, (float)raio, Fade(portal, 0.10f));
    }
    DrawEllipse(285, 498, 145, 21, Fade(portal, 0.10f));
    DrawEllipseLines(285, 498, 145, 21, Fade(portal, 0.42f));
    DrawEllipseLines(285, 498, 111, 13, Fade(portal, 0.44f));
    selo_portal((Vector2){285, 418}, 0.86f, quadro->tempo, true);

    painel((Rectangle){565, 169, 390, 294}, painel_escuro, borda);
    DrawRectangle(582, 172, 354, 2, Fade(portal, 0.53f));
    DrawText("Nome do caçador", 588, 210, 20, texto);
    Rectangle entrada = {588, 241, 344, 48};
    painel(entrada, campo, quadro->editando_nome ? portal : borda);
    const char *nome = quadro->nome == NULL ? "" : quadro->nome;
    if (*nome == '\0' && !quadro->editando_nome) {
        DrawText("Digite seu nome", 603, 255, 20, suave);
    } else {
        int largura = MeasureText(nome, 20);
        int inicio = largura > 303 ? 906 - largura : 603;
        BeginScissorMode(602, 247, 313, 36);
        DrawText(nome, inicio, 255, 20, texto);
        if (quadro->editando_nome && parte_fracionaria(quadro->tempo) < 0.58f) {
            DrawRectangle(inicio + largura + 3, 251, 2, 25, portal);
        }
        EndScissorMode();
    }
    if (quadro->aviso != NULL && *quadro->aviso != '\0') {
        texto_bloco(quadro->aviso, (Rectangle){589, 416, 343, 40}, 17, 19, dano);
    }
}

static void piso_mapa(void)
{
    DrawRectangleGradientV(41, 121, 918, 374, (Color){28, 36, 56, 255},
                           (Color){15, 21, 38, 255});
    for (int linha = 0; linha < 9; ++linha) {
        for (int coluna = 0; coluna < 23; ++coluna) {
            int ladrilho_x = 42 + 40 * coluna;
            int ladrilho_y = 160 + 40 * linha;
            DrawRectangle(ladrilho_x, ladrilho_y, 39, 39,
                          Fade((linha + coluna) % 2 == 0 ? WHITE : BLACK, 0.035f));
            if ((coluna * 7 + linha * 11) % 9 == 2) {
                DrawLine(ladrilho_x + 10, ladrilho_y + 6, ladrilho_x + 24, ladrilho_y + 15,
                         Fade(suave, 0.17f));
                DrawLine(ladrilho_x + 24, ladrilho_y + 15, ladrilho_x + 23, ladrilho_y + 27,
                         Fade(suave, 0.17f));
            }
        }
    }
    DrawRectangleGradientV(41, 121, 918, 62, (Color){10, 13, 29, 255},
                           (Color){28, 35, 56, 255});
    DrawRectangle(41, 160, 918, 2, Fade(portal, 0.34f));
    DrawRectangleGradientH(64, 277, 850, 37, Fade(portal, 0.02f), Fade(portal, 0.10f));
    for (int coluna_x = 64; coluna_x < 928; coluna_x += 96) {
        DrawRectangle(coluna_x, 130, 9, 34, (Color){39, 43, 73, 255});
        DrawCircleGradient(coluna_x + 4, 165, 29, Fade(portal, 0.32f), Fade(portal, 0));
        DrawCircle(coluna_x + 4, 165, 3, portal);
    }
    DrawRectangleLinesEx(AREA_MOVIMENTO, 1.0f, Fade(portal, 0.19f));
}

static void raio_interacao(Vector2 posicao, float tempo, bool perto, Color cor)
{
    float pulso = subir_e_descer(parte_fracionaria(tempo * 0.62f));
    float opacidade = perto ? 0.38f + 0.34f * pulso : 0.27f;
    DrawCircleV(posicao, DISTANCIA_INTERACAO, Fade(cor, perto ? 0.095f : 0.045f));
    DrawCircleLinesV(posicao, DISTANCIA_INTERACAO, Fade(cor, opacidade));
}

static void desenho_inscricao(Vector2 posicao, float tempo, bool perto, bool examinada)
{
    Color cor = examinada ? alerta : portal;
    raio_interacao(posicao, tempo, perto, cor);
    DrawEllipse((int)posicao.x, (int)posicao.y + 9, 37, 16, Fade(BLACK, 0.48f));
    Rectangle placa = {posicao.x - 27, posicao.y - 23, 54, 39};
    painel(placa, (Color){41, 37, 64, 255}, cor);
    Vector2 topo = {posicao.x, posicao.y - 16};
    Vector2 esquerda = {posicao.x - 10, posicao.y - 5};
    Vector2 direita = {posicao.x + 10, posicao.y - 5};
    Vector2 baixo = {posicao.x, posicao.y + 8};
    DrawLineEx(topo, esquerda, 2, cor);
    DrawLineEx(topo, direita, 2, cor);
    DrawLineEx(esquerda, baixo, 2, cor);
    DrawLineEx(direita, baixo, 2, cor);
    DrawLineEx(topo, baixo, 1, Fade(cor, 0.75f));
}

static void explorar(const QuadroTela *quadro)
{
    bool intermediario = quadro->tela == TELA_PORTAL;
    bool examinado = intermediario && quadro->jogo->portal_examinado;
    bool perto_portao = quadro->perto == ALVO_PORTAO;
    bool perto_inscricao = quadro->perto == ALVO_INSCRICAO;
    cabecalho("EXPLORAÇÃO", intermediario ? "Passagem do templo" : "Entrada da masmorra",
              "WASD ou setas para andar   /   F para interagir");

    painel((Rectangle){40, 120, 920, 376}, painel_escuro, borda);
    BeginScissorMode(41, 121, 918, 374);
    piso_mapa();
    raio_interacao(POSICAO_PORTAO, quadro->tempo, perto_portao, portal);
    DrawEllipse((int)POSICAO_PORTAO.x, (int)POSICAO_PORTAO.y + 6, 55, 17, Fade(BLACK, 0.47f));
    Vector2 portao = {POSICAO_PORTAO.x, POSICAO_PORTAO.y - 26};
    selo_portal(portao, 0.50f, quadro->tempo, perto_portao);
    if (intermediario) {
        desenho_inscricao(POSICAO_INSCRICAO, quadro->tempo, perto_inscricao, examinado);
    }
    personagem_desenhar(quadro->personagem, VISUAL_CACADOR, POSE_NORMAL, 0.0f, 0.76f);
    EndScissorMode();

    if (perto_portao) {
        DrawText(intermediario ? "[F] Entrar no templo" : "[F] Entrar no túnel", 43, 510, 19, vida);
    } else if (perto_inscricao) {
        DrawText(examinado ? "[F] Entrar na passagem lateral" : "[F] Examinar inscrição",
                 43, 510, 19, examinado ? alerta : vida);
    } else {
        DrawText(intermediario ? "Aproxime-se do portão ou das marcas na parede."
                               : "Aproxime-se do portão para entrar na masmorra.", 43, 510, 19, suave);
    }
    if (examinado) {
        texto_bloco(quadro->jogo->mensagem, (Rectangle){43, 535, 914, 31}, 14, 15, alerta);
    }
}

static void barra(Rectangle area, int valor, int maximo, Color cor)
{
    DrawRectangleRec(area, Fade(borda, 0.69f));
    if (valor > 0 && maximo > 0) {
        area.width *= valor < maximo ? (float)valor / (float)maximo : 1.0f;
        DrawRectangleRec(area, cor);
    }
}

static void fundo_arena(const QuadroTela *quadro)
{
    Color brilho = quadro->jogo->chefe_em_furia ? dano : portal;
    DrawRectangleGradientV(41, 120, 918, 224, (Color){25, 26, 54, 255},
                           (Color){12, 17, 33, 255});
    DrawCircleGradient(500, 271, 155, Fade(brilho, 0.24f), Fade(brilho, 0));
    DrawRectangleGradientV(41, 277, 918, 65, Fade(campo, 0.1f), Fade(campo, 0.86f));
    for (int coluna_x = 66; coluna_x < 950; coluna_x += 98) {
        DrawLine(coluna_x, 343, 500 + (coluna_x - 500) / 3, 249, Fade(borda, 0.62f));
    }
    for (int linha_y = 285; linha_y <= 329; linha_y += 22) {
        DrawLine(42, linha_y, 957, linha_y, Fade(borda, 0.58f));
    }
    DrawEllipseLines(220, 330, 125, 12, Fade(vida, 0.37f));
    DrawEllipseLines(755, 330, 125, 12, Fade(brilho, 0.50f));
}

static VisualPersonagem visual_inimigo(const Inimigo *inimigo)
{
    if (inimigo->chefe) {
        return VISUAL_ESTATUA;
    }
    if (strcmp(inimigo->nome, "Cavaleiro Vermelho") == 0 || strstr(inimigo->nome, "Sentinela") != NULL) {
        return VISUAL_CAVALEIRO;
    }
    return VISUAL_MONSTRO;
}

static void particulas_turno(Vector2 centro, Color cor, float progresso, int quantidade)
{
    const Vector2 direcoes[] = {
        {1, 0}, {1, 0.5f}, {0.7f, 0.7f}, {0.5f, 1}, {0, 1}, {-0.5f, 1},
        {-0.7f, 0.7f}, {-1, 0.5f}, {-1, 0}, {-1, -0.5f}, {-0.7f, -0.7f},
        {-0.5f, -1}, {0, -1}, {0.5f, -1}, {0.7f, -0.7f}, {1, -0.5f}
    };
    const int total_direcoes = sizeof direcoes / sizeof direcoes[0];
    for (int indice = 0; indice < quantidade; ++indice) {
        Vector2 direcao = direcoes[indice * total_direcoes / quantidade];
        float raio = 18.0f + (indice % 3 * 9 + 47) * progresso;
        Vector2 posicao = {centro.x + direcao.x * raio, centro.y + direcao.y * raio};
        DrawCircleV(posicao, 2.0f + indice % 3, Fade(cor, 1.0f - progresso));
    }
}

static void atores_combate(const QuadroTela *quadro, const Inimigo *inimigo)
{
    float progresso = quadro->progresso_efeito;
    float impacto = subir_e_descer(progresso);
    bool atingido = progresso >= 0.35f && progresso <= 0.76f;
    Personagem cacador = {{220, 333}, quadro->tempo, 1, false};
    Personagem adversario = {{755, 333}, quadro->tempo + 0.37f, -1, false};
    PosePersonagem pose_cacador = POSE_NORMAL;
    PosePersonagem pose_adversario = POSE_NORMAL;

    switch (quadro->efeito) {
        case EFEITO_ATAQUE:
            pose_cacador = POSE_ATAQUE;
            cacador.posicao.x += impacto * 365;
            pose_adversario = atingido ? POSE_FERIDO : POSE_NORMAL;
            break;
        case EFEITO_DEFESA:
            pose_cacador = POSE_DEFESA;
            pose_adversario = POSE_ATAQUE;
            adversario.posicao.x -= impacto * 370;
            break;
        case EFEITO_SOMBRA:
            pose_cacador = POSE_ATAQUE;
            pose_adversario = atingido ? POSE_FERIDO : POSE_NORMAL;
            break;
        case EFEITO_POCAO:
        case EFEITO_NENHUM:
            break;
    }

    personagem_desenhar(&cacador, VISUAL_CACADOR, pose_cacador, progresso, 1.55f);
    personagem_desenhar(&adversario, visual_inimigo(inimigo), pose_adversario, progresso, 1.57f);

    Vector2 ponto_cacador = {220, 269};
    Vector2 ponto_inimigo = {754, 276};
    if (quadro->efeito == EFEITO_ATAQUE) {
        DrawRing((Vector2){cacador.posicao.x + 17, 281}, 45, 50, -61, 67, 26,
                 Fade(texto, impacto * 0.70f));
        if (atingido) {
            particulas_turno(ponto_inimigo, dano, progresso, 10);
        }
    } else if (quadro->efeito == EFEITO_DEFESA) {
        DrawCircleV(ponto_cacador, 55, Fade(vida, impacto * 0.15f));
        DrawRing(ponto_cacador, 54, 57, -76, 75, 30, Fade(vida, impacto));
        particulas_turno(ponto_cacador, vida, progresso, 8);
    } else if (quadro->efeito == EFEITO_SOMBRA) {
        Personagem sombra = {{596 + progresso * 72, 333}, quadro->tempo, 1, false};
        personagem_desenhar(&sombra, VISUAL_SOMBRA, POSE_ATAQUE, progresso, 1.44f);
        DrawEllipseLines((int)sombra.posicao.x, 331, 53, 14, Fade(portal, impacto));
        if (atingido) {
            particulas_turno(ponto_inimigo, portal, progresso, 14);
        }
    } else if (quadro->efeito == EFEITO_POCAO) {
        DrawCircleGradient(220, 269, 61, Fade(vida, impacto * 0.36f), Fade(vida, 0));
        for (int indice = 0; indice < 5; ++indice) {
            int particula_x = 172 + indice * 23;
            int particula_y = 304 - (int)(progresso * 67) - indice % 2 * 16;
            DrawRectangle(particula_x, particula_y + 4, 12, 4, Fade(vida, impacto));
            DrawRectangle(particula_x + 4, particula_y, 4, 12, Fade(vida, impacto));
        }
    }
}

static void hud_combate(const QuadroTela *quadro)
{
    const Jogo *jogo = quadro->jogo;
    const Cacador *cacador = &jogo->cacador;
    char valor[68];

    painel((Rectangle){40, 352, 272, 80}, painel_escuro, borda);
    snprintf(valor, sizeof valor, "Nível %d  /  Ataque %d  /  %d pts",
             cacador->nivel, cacador->ataque, cacador->pontos);
    DrawText(valor, 54, 367, 16, texto);
    int proximo_nivel = jogo_experiencia_para_subir(cacador);
    snprintf(valor, sizeof valor, "XP %d / %d", cacador->experiencia, proximo_nivel);
    DrawText(valor, 54, 391, 14, suave);
    barra((Rectangle){54, 414, 243, 5}, cacador->experiencia, proximo_nivel, portal);

    painel((Rectangle){325, 352, 228, 80}, painel_escuro, borda);
    snprintf(valor, sizeof valor, "ESSÊNCIA  %d / %d", cacador->essencia, ESSENCIA_MAXIMA);
    DrawText(valor, 339, 367, 14, texto);
    for (int indice = 0; indice < ESSENCIA_MAXIMA; ++indice) {
        int largura = 199 / ESSENCIA_MAXIMA;
        DrawRectangle(339 + indice * largura, 390, largura - 4, 6,
                      indice < cacador->essencia ? portal : borda);
    }
    if (cacador->essencia > 0) {
        snprintf(valor, sizeof valor, "%s: %d de dano", cacador->tem_igris ? "Igris" : "Sombra",
                 jogo_dano_sombra(jogo));
    } else {
        snprintf(valor, sizeof valor, "%s sem essência", cacador->tem_igris ? "Igris" : "Sombra");
    }
    DrawText(valor, 339, 407, 14, cacador->essencia > 0 ? portal : suave);

    bool carregado = jogo_golpe_carregado(jogo);
    Color cor_aviso = carregado ? alerta : suave;
    Color fundo_aviso = carregado ? (Color){61, 39, 41, 255} : painel_escuro;
    painel((Rectangle){566, 352, 394, 80}, fundo_aviso, carregado ? alerta : borda);
    DrawRectangle(567, 360, 3, 64, carregado ? alerta : borda);
    DrawText(carregado ? "GOLPE CARREGADO" : "PRÓXIMO GOLPE", 581, 367, 16, cor_aviso);
    snprintf(valor, sizeof valor, "%d de dano", jogo_dano_previsto(jogo));
    texto_direita(valor, 943, 366, 18, carregado ? alerta : texto);
    const char *dica = "Defender reduz o dano pela metade.";
    if (carregado) {
        dica = cacador->essencia < ESSENCIA_MAXIMA ? "Defender anula o dano e recupera 1 essência."
                                                  : "Defender anula o dano.";
    }
    texto_bloco(dica, (Rectangle){581, 398, 360, 28}, 14, 16, texto);
}

static void combate(const QuadroTela *quadro)
{
    const Jogo *jogo = quadro->jogo;
    const Inimigo *inimigo = jogo_inimigo_atual(jogo);
    cabecalho("SISTEMA / COMBATE", jogo_nome_masmorra(jogo), NULL);
    painel((Rectangle){40, 119, 920, 225}, painel_escuro, borda);
    if (inimigo == NULL) {
        return;
    }

    BeginScissorMode(41, 120, 918, 223);
    fundo_arena(quadro);
    atores_combate(quadro, inimigo);
    EndScissorMode();

    texto_recortado(quadro->nome, (Rectangle){60, 134, 245, 25}, 23, texto);
    etiqueta("RANK E", (Rectangle){303, 133, 76, 23}, portal);
    texto_direita(inimigo->nome, 940, 134, 23, texto);
    char valor[52];
    snprintf(valor, sizeof valor, "%d / %d PV", jogo->cacador.vida, jogo->cacador.vida_maxima);
    DrawText(valor, 60, 163, 15, vida);
    snprintf(valor, sizeof valor, "%d / %d PV", inimigo->vida, jogo->vida_maxima_inimigo);
    texto_direita(valor, 940, 163, 15, dano);
    barra((Rectangle){60, 183, 315, 7}, jogo->cacador.vida, jogo->cacador.vida_maxima, vida);
    barra((Rectangle){625, 183, 315, 7}, inimigo->vida, jogo->vida_maxima_inimigo, dano);
    if (jogo->chefe_em_furia) {
        etiqueta("CHEFE EM FÚRIA", (Rectangle){774, 197, 166, 24}, alerta);
    } else if (inimigo->chefe) {
        etiqueta("CHEFE", (Rectangle){856, 197, 84, 24}, dano);
    }

    hud_combate(quadro);
    painel((Rectangle){40, 441, 920, 115}, painel_escuro, borda);
    DrawText("REGISTRO DO SISTEMA", 54, 452, 13, portal);
    texto_bloco(jogo->mensagem, (Rectangle){54, 474, 891, 75}, 15, 17, texto);
}

static void inventario(const QuadroTela *quadro)
{
    cabecalho("CAÇADOR", "Inventário", "Selecione uma poção para recuperar PV.");
    painel((Rectangle){40, 120, 920, 426}, painel_escuro, borda);
    const Inventario *itens = &quadro->jogo->cacador.inventario;
    if (inventario_quantidade(itens) == 0) {
        texto_central("Seu inventário está vazio.", (Rectangle){60, 152, 880, 330}, 22, suave);
    }
    DrawText("Usar uma poção consome um turno.", 60, 514, 17, suave);
}

static void ranking(const QuadroTela *quadro)
{
    cabecalho("RESULTADOS", "Ranking da sessão", "As pontuações ficam disponíveis enquanto o jogo estiver aberto.");
    painel((Rectangle){40, 120, 920, 426}, painel_escuro, borda);
    DrawText("POSIÇÃO", 66, 137, 14, suave);
    DrawText("CAÇADOR", 186, 137, 14, suave);
    texto_direita("PONTOS", 931, 137, 14, suave);
    if (quadro->ranking->quantidade == 0) {
        texto_central("Nenhuma pontuação registrada.", (Rectangle){57, 161, 886, 330}, 22, suave);
        return;
    }

    for (int indice = 0; indice < quadro->ranking->quantidade; ++indice) {
        const Pontuacao *pontuacao = &quadro->ranking->resultados[indice];
        int linha_y = 165 + 36 * indice;
        Color cor_linha = indice == 0 ? (Color){56, 43, 94, 255} : campo;
        DrawRectangleRounded((Rectangle){58, (float)linha_y, 884, 31}, 0.10f, 8, cor_linha);
        char numero[24];
        snprintf(numero, sizeof numero, "%02d", indice + 1);
        DrawText(numero, 70, linha_y + 6, 19, portal);
        texto_recortado(pontuacao->nome, (Rectangle){186, (float)linha_y + 6, 628, 23}, 19, texto);
        snprintf(numero, sizeof numero, "%d", pontuacao->pontos);
        texto_direita(numero, 931, linha_y + 6, 19, texto);
    }
}

static void ajuda(void)
{
    const struct {
        const char *titulo;
        const char *descricao;
    } topicos[] = {
        {"Explorar", "Use WASD ou as setas para andar. Pressione F perto de um portão ou de uma inscrição."},
        {"Atacar", "Acerte o inimigo da frente. Se sobreviver, ele pode contra-atacar com o dano indicado pelo Sistema."},
        {"Defender", "Reduza o dano normal pela metade. Defender um golpe carregado anula o dano e recupera 1 essência."},
        {"Erga-se", "Use 1 essência para invocar uma sombra. O dano previsto aparece ao lado da reserva de essência."},
        {"Curar e evoluir", "Usar poção consome o turno. Derrote inimigos para ganhar XP e elevar sua vida e seu ataque."}
    };
    cabecalho("CONTROLES E COMBATE", "Como jogar", NULL);
    painel((Rectangle){40, 120, 920, 426}, painel_escuro, borda);
    for (size_t indice = 0; indice < sizeof topicos / sizeof topicos[0]; ++indice) {
        int linha_y = 142 + (int)indice * 76;
        DrawText(topicos[indice].titulo, 66, linha_y + 13, 22, portal);
        texto_bloco(topicos[indice].descricao, (Rectangle){286, (float)linha_y + 4, 638, 58}, 20, 23, texto);
        if (indice + 1 < sizeof topicos / sizeof topicos[0]) {
            DrawRectangle(66, linha_y + 62, 864, 1, Fade(borda, 0.76f));
        }
    }
}

static void resultado(const QuadroTela *quadro)
{
    const Jogo *jogo = quadro->jogo;
    EstadoJogo estado = jogo->estado;
    bool pontuavel = estado == JOGO_VITORIA || estado == JOGO_DERROTA;
    const char *titulo = "Partida interrompida";
    Color cor = alerta;
    if (estado == JOGO_VITORIA) {
        titulo = "Vitória";
        cor = vida;
    } else if (estado == JOGO_DERROTA) {
        titulo = "Derrota";
        cor = dano;
    }
    cabecalho("RESULTADO DA CAÇADA", titulo, quadro->nome);
    painel((Rectangle){40, 120, 920, 426}, painel_escuro, borda);
    DrawRectangle(41, 128, 4, 410, cor);
    int y_registro = 174;
    if (pontuavel) {
        char total[28];
        snprintf(total, sizeof total, "%d", jogo->cacador.pontos);
        DrawText(total, 68, 161, 66, cor);
        DrawText("PONTOS", 71, 230, 17, suave);
        y_registro = 297;
    }
    DrawText("REGISTRO DO SISTEMA", 70, y_registro - 24, 14, portal);
    texto_bloco(jogo->mensagem, (Rectangle){70, (float)y_registro, 854, (float)(523 - y_registro)},
                20, 25, texto);
}

static void botao_desenhar(const BotaoTela *botao, bool linha_inventario)
{
    Color cor = texto;
    Color cor_borda = borda;
    Color interior = campo;
    if (!botao->habilitado) {
        interior = painel_escuro;
        cor = Fade(suave, 0.59f);
    } else if (botao->tipo == BOTAO_PRIMARIO) {
        interior = (Color){96, 66, 169, 255};
        cor_borda = portal;
    } else if (botao->tipo == BOTAO_PERIGO) {
        interior = (Color){49, 29, 45, 255};
        cor_borda = (Color){106, 55, 84, 255};
        cor = (Color){255, 200, 216, 255};
    }

    bool selecionado = botao->habilitado && botao->selecionado;
    if (selecionado) {
        cor_borda = cor;
    }
    painel(botao->area, interior, cor_borda);
    if (selecionado) {
        Rectangle corpo = {botao->area.x + 1, botao->area.y + 1,
                           botao->area.width - 2, botao->area.height - 2};
        DrawRectangleRounded(corpo, 0.08f, 8, Fade(cor, 0.15f));
    }
    if (linha_inventario) {
        DrawText(botao->texto, (int)botao->area.x + 18,
                 (int)(botao->area.y + (botao->area.height - 19) / 2), 19, cor);
    } else {
        int tamanho = MeasureText(botao->texto, 19) + 16 > botao->area.width ? 16 : 19;
        texto_central(botao->texto, botao->area, tamanho, cor);
    }
}

void tela_desenhar(const QuadroTela *quadro)
{
    fundo_tela();
    switch (quadro->tela) {
        case TELA_MENU:
            menu(quadro);
            break;
        case TELA_ENTRADA:
        case TELA_PORTAL:
            explorar(quadro);
            break;
        case TELA_COMBATE:
            combate(quadro);
            break;
        case TELA_INVENTARIO:
            inventario(quadro);
            break;
        case TELA_RANKING:
            ranking(quadro);
            break;
        case TELA_AJUDA:
            ajuda();
            break;
        case TELA_RESULTADO:
            resultado(quadro);
            break;
    }

    for (size_t indice = 0; indice < quadro->quantidade_botoes; ++indice) {
        const BotaoTela *botao = &quadro->botoes[indice];
        bool linha_inventario = quadro->tela == TELA_INVENTARIO && botao->area.y < 550;
        botao_desenhar(botao, linha_inventario);
    }
}
