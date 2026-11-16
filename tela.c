#include "tela.h"

enum {
    FONTE_LEGENDA,
    FONTE_TEXTO,
    FONTE_FORTE,
    FONTE_SECAO,
    FONTE_TITULO,
    FONTE_HEROI,
    QUANTIDADE_FONTES
};

typedef struct {
    HDC contexto;
    HFONT fontes[QUANTIDADE_FONTES];
} Pintura;

static const COLORREF cor_fundo = RGB(10, 13, 26);
static const COLORREF cor_painel = RGB(17, 22, 40);
static const COLORREF cor_borda = RGB(39, 48, 75);
static const COLORREF cor_suave = RGB(164, 176, 208);
static const COLORREF cor_portal = RGB(175, 146, 255);
static const COLORREF cor_vida = RGB(86, 224, 181);
static const COLORREF cor_dano = RGB(247, 114, 155);

static HFONT criar_fonte(int tamanho, int peso)
{
    return CreateFontW(-tamanho, 0, 0, 0, peso, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Inter");
}

static void preencher(HDC contexto, int x, int y, int largura, int altura, COLORREF cor)
{
    RECT area = {x, y, x + largura, y + altura};
    SetDCBrushColor(contexto, cor);
    FillRect(contexto, &area, (HBRUSH)GetStockObject(DC_BRUSH));
}

static void painel(HDC contexto, int x, int y, int largura, int altura, COLORREF borda)
{
    preencher(contexto, x, y, largura, altura, borda);
    preencher(contexto, x + 1, y + 1, largura - 2, altura - 2, cor_painel);
}

static void escrever(HDC contexto, HFONT fonte, COLORREF cor, int x, int y,
                     int largura, int altura, UINT formato, const wchar_t *texto)
{
    RECT area = {x, y, x + largura, y + altura};
    HGDIOBJ anterior = NULL;
    if (fonte != NULL) {
        anterior = SelectObject(contexto, fonte);
    }
    SetBkMode(contexto, TRANSPARENT);
    SetTextColor(contexto, cor);
    DrawTextW(contexto, texto, -1, &area, DT_NOPREFIX | formato);
    if (anterior != NULL) {
        SelectObject(contexto, anterior);
    }
}

static void texto(Pintura *pintura, int fonte, COLORREF cor, int x, int y,
                  int largura, int altura, const wchar_t *conteudo)
{
    escrever(pintura->contexto, pintura->fontes[fonte], cor, x, y, largura,
             altura, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS, conteudo);
}

static void paragrafo(Pintura *pintura, int fonte, COLORREF cor, int x, int y,
                      int largura, int altura, const wchar_t *conteudo)
{
    escrever(pintura->contexto, pintura->fontes[fonte], cor, x, y, largura,
             altura, DT_WORDBREAK, conteudo);
}

static void converter_texto(const char *origem, wchar_t *destino, int capacidade)
{
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, origem, -1, destino,
                            capacidade) == 0) {
        destino[0] = L'\0';
    }
}

static void contorno(HDC contexto, const POINT *pontos, int quantidade, COLORREF cor)
{
    HGDIOBJ anterior = SelectObject(contexto, GetStockObject(DC_PEN));
    SetDCPenColor(contexto, cor);
    Polyline(contexto, pontos, quantidade);
    SelectObject(contexto, anterior);
}

static void elipse(HDC contexto, int x, int y, int largura, int altura,
                    COLORREF borda, COLORREF interior)
{
    HGDIOBJ pincel = SelectObject(contexto, GetStockObject(DC_BRUSH));
    HGDIOBJ caneta = SelectObject(contexto, GetStockObject(DC_PEN));
    SetDCBrushColor(contexto, interior);
    SetDCPenColor(contexto, borda);
    Ellipse(contexto, x, y, x + largura, y + altura);
    SelectObject(contexto, pincel);
    SelectObject(contexto, caneta);
}

static void desenhar_portal(HDC contexto)
{
    const POINT arestas[][3] = {
        {{58, 565}, {262, 432}, {471, 565}},
        {{124, 565}, {262, 432}, {405, 565}},
        {{185, 565}, {262, 432}, {346, 565}}
    };
    const POINT portal_externo[] = {
        {262, 343}, {315, 388}, {324, 469}, {262, 520},
        {206, 469}, {209, 390}, {262, 343}
    };
    const POINT portal_interno[] = {
        {262, 356}, {302, 395}, {313, 462}, {262, 506},
        {221, 462}, {221, 398}, {262, 356}
    };
    const POINT fissura[] = {
        {269, 381}, {253, 425}, {270, 419}, {254, 480}
    };
    const POINT particulas[] = {
        {134, 413}, {168, 471}, {200, 379}, {213, 515}, {181, 439},
        {329, 364}, {342, 443}, {363, 476}, {388, 410}, {324, 502}
    };

    for (size_t indice = 0; indice < sizeof arestas / sizeof arestas[0]; ++indice) {
        contorno(contexto, arestas[indice], 3, RGB(36, 41, 69));
    }
    preencher(contexto, 93, 545, 351, 1, RGB(38, 42, 71));
    elipse(contexto, 130, 480, 264, 55, RGB(51, 49, 95), RGB(16, 18, 36));
    elipse(contexto, 162, 486, 200, 42, RGB(89, 71, 148), RGB(21, 21, 47));
    elipse(contexto, 198, 489, 128, 27, RGB(133, 96, 208), RGB(34, 26, 73));

    elipse(contexto, 199, 343, 126, 181, RGB(47, 40, 88), RGB(16, 18, 40));
    elipse(contexto, 208, 351, 108, 162, RGB(77, 56, 142), RGB(29, 22, 61));
    elipse(contexto, 222, 361, 80, 144, RGB(124, 82, 202), RGB(49, 30, 98));
    elipse(contexto, 236, 374, 52, 115, RGB(167, 110, 235), RGB(76, 42, 137));
    elipse(contexto, 249, 387, 26, 88, RGB(194, 153, 255), RGB(112, 66, 179));
    contorno(contexto, portal_externo, 7, RGB(110, 80, 178));
    contorno(contexto, portal_interno, 7, cor_portal);
    contorno(contexto, fissura, 4, RGB(235, 203, 255));

    for (size_t indice = 0; indice < sizeof particulas / sizeof particulas[0]; ++indice) {
        int x = particulas[indice].x;
        int y = particulas[indice].y;
        preencher(contexto, x, y, 3, 3, cor_portal);
    }
}

static void desenhar_menu(Pintura *pintura, const wchar_t *erro_nome)
{
    HDC contexto = pintura->contexto;

    texto(pintura, FONTE_HEROI, COR_TEXTO, 54, 140, 465, 71, L"Solo Leveling");
    texto(pintura, FONTE_TITULO, cor_portal, 58, 201, 450, 49, L"Dungeon Quest");
    paragrafo(pintura, FONTE_TEXTO, cor_suave, 59, 271, 378, 57,
              L"Atravesse as masmorras, evolua como ca\u00e7ador e enfrente a Est\u00e1tua Divina.");
    desenhar_portal(contexto);

    painel(contexto, 524, 155, 369, 276, cor_borda);
    preencher(contexto, 524, 155, 369, 2, cor_portal);
    texto(pintura, FONTE_FORTE, COR_TEXTO, 552, 190, 312, 25, L"Nome do ca\u00e7ador");
    painel(contexto, 551, 218, 314, 44, cor_borda);
    if (erro_nome != NULL && *erro_nome != L'\0') {
        paragrafo(pintura, FONTE_LEGENDA, cor_dano, 552, 392, 312, 35, erro_nome);
    }
}

static void cabecalho(Pintura *pintura, const wchar_t *titulo, const wchar_t *subtitulo)
{
    texto(pintura, FONTE_TITULO, COR_TEXTO, 40, 25, 840, 45, titulo);
    if (subtitulo != NULL) {
        texto(pintura, FONTE_TEXTO, cor_suave, 41, 71, 838, 22, subtitulo);
    }
    preencher(pintura->contexto, 40, 99, 840, 1, cor_borda);
}

static void barra(Pintura *pintura, const wchar_t *rotulo, int atual, int maximo,
                  int x, int y, int largura, COLORREF cor)
{
    wchar_t valores[48];
    wsprintfW(valores, L"%d / %d", atual, maximo);
    texto(pintura, FONTE_LEGENDA, COR_TEXTO, x, y, largura / 2, 20, rotulo);
    escrever(pintura->contexto, pintura->fontes[FONTE_LEGENDA], cor_suave,
             x + largura / 2, y, largura - largura / 2, 20,
             DT_RIGHT | DT_SINGLELINE | DT_VCENTER, valores);

    preencher(pintura->contexto, x, y + 23, largura, 7, COR_CAMPO);
    if (maximo > 0 && atual > 0) {
        int preenchimento = atual < maximo ? largura * atual / maximo : largura;
        preencher(pintura->contexto, x, y + 23, preenchimento, 7, cor);
    }
}

static void desenhar_cacador(Pintura *pintura, Cacador *cacador, const wchar_t *nome)
{
    wchar_t detalhes[64];
    HDC contexto = pintura->contexto;

    painel(contexto, 40, 111, 367, 218, cor_borda);
    preencher(contexto, 40, 111, 3, 218, cor_vida);
    texto(pintura, FONTE_SECAO, COR_TEXTO, 64, 129, 318, 37, nome);
    wsprintfW(detalhes, L"N\u00edvel %d    \u2022    Ataque %d", cacador->nivel,
              cacador->ataque);
    texto(pintura, FONTE_TEXTO, cor_suave, 64, 168, 318, 27, detalhes);

    barra(pintura, L"VIDA", cacador->vida, cacador->vida_maxima, 64, 209, 319, cor_vida);
    barra(pintura, L"EXPERI\u00caNCIA", cacador->experiencia, jogo_experiencia_para_subir(cacador),
          64, 257, 319, cor_portal);

    wsprintfW(detalhes, L"%d pontos", cacador->pontos);
    texto(pintura, FONTE_LEGENDA, cor_suave, 64, 298, 318, 21, detalhes);
}

static void desenhar_inimigo(Pintura *pintura, Jogo *jogo)
{
    const Inimigo *inimigo = jogo_inimigo_atual(jogo);
    HDC contexto = pintura->contexto;
    painel(contexto, 427, 111, 453, 218, cor_borda);
    if (inimigo == NULL) {
        return;
    }

    const COLORREF cor = inimigo->chefe ? cor_dano : cor_portal;
    const wchar_t *tipo = inimigo->chefe ? L"CHEFE FINAL" : L"INIMIGO";
    wchar_t nome[80];
    wchar_t detalhes[48];
    converter_texto(inimigo->nome, nome, 80);
    wsprintfW(detalhes, L"Ataque %d", inimigo->ataque);

    preencher(contexto, 427, 111, 3, 218, cor);
    texto(pintura, FONTE_LEGENDA, cor, 451, 132, 404, 20, tipo);
    texto(pintura, FONTE_SECAO, COR_TEXTO, 451, 158, 404, 37, nome);
    texto(pintura, FONTE_TEXTO, cor_suave, 451, 199, 404, 27, detalhes);
    barra(pintura, L"VIDA", inimigo->vida, jogo->vida_maxima_inimigo,
          451, 256, 404, cor_dano);
}

static void desenhar_combate(Pintura *pintura, Jogo *jogo, const wchar_t *nome)
{
    wchar_t masmorra[96];
    wchar_t etapa[48];
    wchar_t eventos[TAMANHO_MENSAGEM_JOGO];
    converter_texto(jogo_nome_masmorra(jogo), masmorra, 96);
    wsprintfW(etapa, L"Masmorra %u de %u", (unsigned)jogo->indice_masmorra + 1,
              (unsigned)jogo_total_masmorras());

    cabecalho(pintura, masmorra, etapa);
    desenhar_cacador(pintura, &jogo->cacador, nome);
    desenhar_inimigo(pintura, jogo);

    painel(pintura->contexto, 40, 345, 840, 158, cor_borda);
    texto(pintura, FONTE_FORTE, cor_portal, 62, 353, 795, 23, L"Registro do combate");
    converter_texto(jogo->mensagem, eventos, TAMANHO_MENSAGEM_JOGO);
    paragrafo(pintura, FONTE_LEGENDA, COR_TEXTO, 62, 382, 795, 112, eventos);
}

static void desenhar_inventario(Pintura *pintura, const Jogo *jogo)
{
    cabecalho(pintura, L"Invent\u00e1rio", NULL);
    painel(pintura->contexto, 40, 111, 840, 392, cor_borda);
    texto(pintura, FONTE_FORTE, COR_TEXTO, 62, 123, 796, 28, L"Itens coletados");
    painel(pintura->contexto, 61, 155, 798, 292, cor_borda);
    if (inventario_quantidade(&jogo->cacador.inventario) == 0) {
        escrever(pintura->contexto, pintura->fontes[FONTE_TEXTO], cor_suave,
                 62, 156, 796, 290, DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                 L"Seu invent\u00e1rio est\u00e1 vazio.");
    }
    texto(pintura, FONTE_TEXTO, cor_suave, 62, 455, 796, 30,
          L"Usar uma po\u00e7\u00e3o gasta o turno.");
}

static void desenhar_ranking(Pintura *pintura, const Ranking *ranking)
{
    HDC contexto = pintura->contexto;
    cabecalho(pintura, L"Ranking da sess\u00e3o",
               L"As pontua\u00e7\u00f5es ficam dispon\u00edveis at\u00e9 fechar o jogo.");
    painel(contexto, 40, 111, 840, 392, cor_borda);
    texto(pintura, FONTE_LEGENDA, cor_suave, 64, 125, 102, 26, L"POSI\u00c7\u00c3O");
    texto(pintura, FONTE_LEGENDA, cor_suave, 166, 125, 430, 26, L"CA\u00c7ADOR");
    escrever(contexto, pintura->fontes[FONTE_LEGENDA], cor_suave, 628, 125, 224,
             26, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, L"PONTOS");

    if (ranking->quantidade == 0) {
        escrever(contexto, pintura->fontes[FONTE_TEXTO], cor_suave, 63, 166,
                 794, 305, DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                 L"Nenhuma pontua\u00e7\u00e3o registrada.");
        return;
    }

    for (int indice = 0; indice < ranking->quantidade; ++indice) {
        const Pontuacao *resultado = &ranking->resultados[indice];
        wchar_t nome[TAMANHO_NOME];
        wchar_t numero[24];
        int y = 158 + 33 * indice;

        COLORREF fundo_linha = indice == 0 ? RGB(43, 32, 80) : COR_CAMPO;
        preencher(contexto, 61, y, 798, 29, fundo_linha);

        wsprintfW(numero, L"%02d", indice + 1);
        texto(pintura, FONTE_FORTE, cor_portal, 70, y, 82, 29, numero);
        converter_texto(resultado->nome, nome, TAMANHO_NOME);
        texto(pintura, FONTE_TEXTO, COR_TEXTO, 166, y, 437, 29, nome);

        wsprintfW(numero, L"%d", resultado->pontos);
        escrever(contexto, pintura->fontes[FONTE_FORTE], COR_TEXTO, 628, y, 224,
                 29, DT_RIGHT | DT_VCENTER | DT_SINGLELINE, numero);
    }
}

static void desenhar_ajuda(Pintura *pintura)
{
    const struct {
        const wchar_t *titulo;
        const wchar_t *descricao;
    } topicos[] = {
        {L"Atacar", L"Acerte o inimigo da frente. Se ele sobreviver, vai contra-atacar."},
        {L"Usar uma po\u00e7\u00e3o", L"Abra o invent\u00e1rio para consultar seus itens. Usar uma po\u00e7\u00e3o recupera vida e consome o turno."},
        {L"Evoluir", L"Cada inimigo derrotado rende experi\u00eancia e pontos. Subir de n\u00edvel aumenta seu ataque e sua vida."},
        {L"Concluir a ca\u00e7ada", L"Supere as duas masmorras, derrote a Est\u00e1tua Divina e confira sua pontua\u00e7\u00e3o no ranking."}
    };

    cabecalho(pintura, L"Como jogar", NULL);
    painel(pintura->contexto, 40, 111, 840, 392, cor_borda);
    for (size_t indice = 0; indice < sizeof topicos / sizeof topicos[0]; ++indice) {
        int y = 134 + (int)indice * 89;
        texto(pintura, FONTE_SECAO, COR_TEXTO, 66, y, 785, 35, topicos[indice].titulo);
        paragrafo(pintura, FONTE_TEXTO, cor_suave, 67, y + 39, 778, 38,
                  topicos[indice].descricao);
        preencher(pintura->contexto, 66, y + 75, 788, 1, cor_borda);
    }
}

static void desenhar_resultado(Pintura *pintura, const Jogo *jogo, const wchar_t *nome)
{
    HDC contexto = pintura->contexto;
    COLORREF cor = jogo->estado == JOGO_VITORIA ? cor_vida : cor_dano;
    const wchar_t *resultado;
    switch (jogo->estado) {
        case JOGO_VITORIA:
            resultado = L"Vit\u00f3ria";
            break;
        case JOGO_DERROTA:
            resultado = L"Derrota";
            break;
        default:
            resultado = L"Partida interrompida";
            break;
    }

    cabecalho(pintura, resultado, nome);
    painel(contexto, 40, 111, 840, 392, cor_borda);
    preencher(contexto, 40, 111, 3, 392, cor);
    const bool tem_pontuacao = jogo->estado == JOGO_VITORIA || jogo->estado == JOGO_DERROTA;
    if (tem_pontuacao) {
        wchar_t total[48];
        wsprintfW(total, L"%d pontos", jogo->cacador.pontos);
        texto(pintura, FONTE_HEROI, cor, 66, 135, 785, 78, total);
    }

    wchar_t eventos[TAMANHO_MENSAGEM_JOGO];
    converter_texto(jogo->mensagem, eventos, TAMANHO_MENSAGEM_JOGO);
    int inicio_eventos = tem_pontuacao ? 234 : 147;
    paragrafo(pintura, FONTE_TEXTO, COR_TEXTO, 66, inicio_eventos, 785,
              470 - inicio_eventos, eventos);
}

static void pintar_tela(HDC contexto, const RECT *cliente, TelaJogo tela, Jogo *jogo,
                       const Ranking *ranking, const wchar_t *nome, const wchar_t *erro_nome)
{
    Pintura pintura = {contexto, {
        criar_fonte(15, FW_NORMAL), criar_fonte(17, FW_NORMAL),
        criar_fonte(17, FW_SEMIBOLD), criar_fonte(25, FW_SEMIBOLD),
        criar_fonte(35, FW_BOLD), criar_fonte(55, FW_BOLD)
    }};

    preencher(contexto, 0, 0, cliente->right, cliente->bottom, cor_fundo);
    preencher(contexto, 0, 0, cliente->right, 3, RGB(109, 75, 199));
    switch (tela) {
        case TELA_MENU:
            desenhar_menu(&pintura, erro_nome);
            break;
        case TELA_COMBATE:
            desenhar_combate(&pintura, jogo, nome);
            break;
        case TELA_INVENTARIO:
            desenhar_inventario(&pintura, jogo);
            break;
        case TELA_RANKING:
            desenhar_ranking(&pintura, ranking);
            break;
        case TELA_AJUDA:
            desenhar_ajuda(&pintura);
            break;
        case TELA_RESULTADO:
            desenhar_resultado(&pintura, jogo, nome);
            break;
    }

    for (int indice = 0; indice < QUANTIDADE_FONTES; ++indice) {
        if (pintura.fontes[indice] != NULL) {
            DeleteObject(pintura.fontes[indice]);
        }
    }
}

void tela_pintar(HWND janela, HDC contexto, TelaJogo tela, Jogo *jogo,
                 const Ranking *ranking, const wchar_t *nome, const wchar_t *erro_nome)
{
    RECT cliente;
    if (!GetClientRect(janela, &cliente) || cliente.right <= 0 || cliente.bottom <= 0) {
        return;
    }

    HDC buffer = CreateCompatibleDC(contexto);
    HBITMAP imagem = CreateCompatibleBitmap(contexto, cliente.right, cliente.bottom);
    if (buffer != NULL && imagem != NULL) {
        HGDIOBJ anterior = SelectObject(buffer, imagem);
        pintar_tela(buffer, &cliente, tela, jogo, ranking, nome, erro_nome);
        BitBlt(contexto, 0, 0, cliente.right, cliente.bottom, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, anterior);
    } else {
        pintar_tela(contexto, &cliente, tela, jogo, ranking, nome, erro_nome);
    }

    if (imagem != NULL) {
        DeleteObject(imagem);
    }
    if (buffer != NULL) {
        DeleteDC(buffer);
    }
}

void tela_desenhar_botao(const DRAWITEMSTRUCT *item, TipoBotao tipo)
{
    HDC contexto = item->hDC;
    RECT area = item->rcItem;
    int largura = area.right - area.left;
    int altura = area.bottom - area.top;
    COLORREF fundo = COR_CAMPO;
    COLORREF borda = cor_borda;
    COLORREF cor = COR_TEXTO;

    if (tipo == BOTAO_PRIMARIO) {
        fundo = RGB(109, 75, 199);
        borda = cor_portal;
    } else if (tipo == BOTAO_PERIGO) {
        fundo = RGB(48, 28, 52);
        borda = RGB(106, 58, 91);
        cor = RGB(255, 193, 212);
    }

    if (item->itemState & ODS_DISABLED) {
        fundo = cor_painel;
        borda = cor_borda;
        cor = RGB(121, 135, 171);
    } else if (item->itemState & ODS_SELECTED) {
        fundo = RGB(53, 49, 92);
    }

    preencher(contexto, area.left, area.top, largura, altura, borda);
    preencher(contexto, area.left + 1, area.top + 1, largura - 2, altura - 2, fundo);

    wchar_t rotulo[64];
    GetWindowTextW(item->hwndItem, rotulo, 64);
    HFONT fonte = criar_fonte(16, FW_SEMIBOLD);
    escrever(contexto, fonte, cor, area.left + 6, area.top, largura - 12, altura,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, rotulo);
    if (fonte != NULL) {
        DeleteObject(fonte);
    }

    if (item->itemState & ODS_FOCUS) {
        InflateRect(&area, -4, -4);
        DrawFocusRect(contexto, &area);
    }
}
