#include "tela.h"

#include <wchar.h>

enum {
    ID_NOME = 100,
    ID_LISTA,
    ID_INICIAR,
    ID_RANKING,
    ID_AJUDA,
    ID_ATACAR,
    ID_DEFENDER,
    ID_SOMBRA,
    ID_INVENTARIO,
    ID_ABANDONAR,
    ID_SEGUIR_TEMPLO,
    ID_PASSAGEM,
    ID_RECUAR_PORTAL,
    ID_USAR,
    ID_VOLTAR_COMBATE,
    ID_VOLTAR_RANKING,
    ID_VOLTAR_AJUDA,
    ID_VOLTAR_MENU,
    ID_RANKING_RESULTADO,
    ID_TIMER_EFEITO = 1,
    DURACAO_EFEITO = 260
};

typedef struct {
    int id;
    TelaJogo tela;
    TipoBotao tipo;
    const wchar_t *texto;
    int x, y, largura, altura;
} ConfigBotao;

static const ConfigBotao configuracoes[] = {
    {ID_INICIAR, TELA_MENU, BOTAO_PRIMARIO, L"Iniciar ca\u00e7a", 552, 275, 312, 48},
    {ID_RANKING, TELA_MENU, BOTAO_SECUNDARIO, L"Ranking", 552, 335, 150, 42},
    {ID_AJUDA, TELA_MENU, BOTAO_SECUNDARIO, L"Como jogar", 714, 335, 150, 42},
    {ID_ATACAR, TELA_COMBATE, BOTAO_PRIMARIO, L"A \u00b7 Atacar", 40, 518, 160, 46},
    {ID_DEFENDER, TELA_COMBATE, BOTAO_SECUNDARIO, L"D \u00b7 Defender", 210, 518, 160, 46},
    {ID_SOMBRA, TELA_COMBATE, BOTAO_SECUNDARIO, L"E \u00b7 Erga-se", 380, 518, 160, 46},
    {ID_INVENTARIO, TELA_COMBATE, BOTAO_SECUNDARIO, L"I \u00b7 Invent\u00e1rio", 550, 518, 160, 46},
    {ID_ABANDONAR, TELA_COMBATE, BOTAO_PERIGO, L"Recuar", 720, 518, 160, 46},
    {ID_SEGUIR_TEMPLO, TELA_PORTAL, BOTAO_SECUNDARIO, L"Seguir ao templo", 40, 518, 270, 46},
    {ID_PASSAGEM, TELA_PORTAL, BOTAO_PRIMARIO, L"Examinar inscri\u00e7\u00e3o", 324, 518, 270, 46},
    {ID_RECUAR_PORTAL, TELA_PORTAL, BOTAO_PERIGO, L"Recuar", 610, 518, 270, 46},
    {ID_USAR, TELA_INVENTARIO, BOTAO_PRIMARIO, L"Usar po\u00e7\u00e3o", 40, 518, 270, 46},
    {ID_VOLTAR_COMBATE, TELA_INVENTARIO, BOTAO_SECUNDARIO, L"Voltar ao combate", 324, 518, 270, 46},
    {ID_VOLTAR_RANKING, TELA_RANKING, BOTAO_SECUNDARIO, L"Voltar", 40, 518, 270, 46},
    {ID_VOLTAR_AJUDA, TELA_AJUDA, BOTAO_SECUNDARIO, L"Voltar ao menu", 40, 518, 270, 46},
    {ID_VOLTAR_MENU, TELA_RESULTADO, BOTAO_PRIMARIO, L"Voltar ao menu", 40, 518, 270, 46},
    {ID_RANKING_RESULTADO, TELA_RESULTADO, BOTAO_SECUNDARIO, L"Ver ranking", 324, 518, 270, 46}
};

enum { QUANTIDADE_BOTOES = sizeof configuracoes / sizeof configuracoes[0] };

typedef struct {
    HWND janela;
    HWND campo_nome;
    HWND lista_itens;
    HWND botoes[QUANTIDADE_BOTOES];
    HFONT fonte;
    HBRUSH fundo_campo;
    TelaJogo tela;
    TelaJogo voltar_do_ranking;
    Jogo jogo;
    bool partida_iniciada;
    Ranking ranking;
    wchar_t nome[TAMANHO_NOME];
    char nome_utf8[TAMANHO_NOME];
    const wchar_t *erro_nome;
    DWORD inicio_efeito;
    bool efeito_ativo;
} Interface;

static bool criar_controles(Interface *aplicacao, HINSTANCE instancia)
{
    aplicacao->fundo_campo = CreateSolidBrush(COR_CAMPO);
    aplicacao->fonte = CreateFontW(-20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    if (aplicacao->fundo_campo == NULL || aplicacao->fonte == NULL) {
        return false;
    }

    HWND janela = aplicacao->janela;
    aplicacao->campo_nome = CreateWindowExW(0, L"EDIT", L"",
        WS_CHILD | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL,
        552, 219, 312, 36, janela, (HMENU)(INT_PTR)ID_NOME, instancia, NULL);
    aplicacao->lista_itens = CreateWindowExW(0, L"LISTBOX", L"",
        WS_CHILD | WS_TABSTOP | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
        62, 156, 796, 290, janela, (HMENU)(INT_PTR)ID_LISTA, instancia, NULL);
    if (aplicacao->campo_nome == NULL || aplicacao->lista_itens == NULL) {
        return false;
    }

    SendMessageW(aplicacao->campo_nome, EM_SETLIMITTEXT, TAMANHO_NOME - 1, 0);
    SendMessageW(aplicacao->campo_nome, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                 MAKELPARAM(10, 10));
    SendMessageW(aplicacao->campo_nome, WM_SETFONT, (WPARAM)aplicacao->fonte, FALSE);
    SendMessageW(aplicacao->lista_itens, WM_SETFONT, (WPARAM)aplicacao->fonte, FALSE);

    for (int indice = 0; indice < QUANTIDADE_BOTOES; ++indice) {
        const ConfigBotao *config = &configuracoes[indice];
        aplicacao->botoes[indice] = CreateWindowExW(0, L"BUTTON", config->texto,
            WS_CHILD | WS_TABSTOP | BS_OWNERDRAW, config->x, config->y,
            config->largura, config->altura, janela, (HMENU)(INT_PTR)config->id,
            instancia, NULL);
        if (aplicacao->botoes[indice] == NULL) {
            return false;
        }
    }

    return true;
}

static void mostrar_tela(Interface *aplicacao, TelaJogo tela)
{
    HWND foco_anterior = GetFocus();
    bool manter_foco = false;
    aplicacao->tela = tela;
    bool tem_itens = tela == TELA_INVENTARIO &&
                     inventario_quantidade(&aplicacao->jogo.cacador.inventario) > 0;
    HWND foco = NULL;

    ShowWindow(aplicacao->campo_nome, tela == TELA_MENU ? SW_SHOW : SW_HIDE);
    ShowWindow(aplicacao->lista_itens, tem_itens ? SW_SHOW : SW_HIDE);

    if (tela == TELA_MENU) {
        foco = aplicacao->campo_nome;
    } else if (tem_itens) {
        foco = aplicacao->lista_itens;
    }
    manter_foco = foco != NULL && foco == foco_anterior;

    for (int indice = 0; indice < QUANTIDADE_BOTOES; ++indice) {
        const ConfigBotao *config = &configuracoes[indice];
        HWND botao = aplicacao->botoes[indice];
        bool visivel = config->tela == tela;
        bool habilitado = (config->id != ID_USAR || tem_itens) &&
                          (config->id != ID_SOMBRA || aplicacao->jogo.cacador.essencia > 0);

        if (visivel && config->id == ID_PASSAGEM) {
            SetWindowTextW(botao, aplicacao->jogo.portal_examinado ?
                           L"Entrar na passagem" : L"Examinar inscri\u00e7\u00e3o");
        }

        EnableWindow(botao, habilitado);
        ShowWindow(botao, visivel ? SW_SHOW : SW_HIDE);
        if (visivel && habilitado) {
            if (foco == NULL) {
                foco = botao;
            }
            if (botao == foco_anterior) {
                manter_foco = true;
            }
        }
    }

    if (!manter_foco) {
        SetFocus(foco);
    }
    InvalidateRect(aplicacao->janela, NULL, FALSE);
}

static void atualizar_partida(Interface *aplicacao)
{
    EstadoJogo estado = aplicacao->jogo.estado;
    if (estado == JOGO_EM_COMBATE) {
        mostrar_tela(aplicacao, TELA_COMBATE);
        return;
    }
    if (estado == JOGO_PORTAL) {
        mostrar_tela(aplicacao, TELA_PORTAL);
        return;
    }

    if (estado == JOGO_VITORIA || estado == JOGO_DERROTA) {
        ranking_registrar(&aplicacao->ranking, aplicacao->nome_utf8,
                         aplicacao->jogo.cacador.pontos);
    }
    mostrar_tela(aplicacao, TELA_RESULTADO);
}

static unsigned efeito_restante(const Interface *aplicacao)
{
    if (!aplicacao->efeito_ativo) {
        return 0;
    }

    DWORD decorrido = GetTickCount() - aplicacao->inicio_efeito;
    return decorrido < DURACAO_EFEITO ? (unsigned)(DURACAO_EFEITO - decorrido) : 0;
}

static void mostrar_acao_de_combate(Interface *aplicacao)
{
    if (aplicacao->jogo.estado == JOGO_EM_COMBATE) {
        aplicacao->inicio_efeito = GetTickCount();
        aplicacao->efeito_ativo = SetTimer(aplicacao->janela, ID_TIMER_EFEITO, 20, NULL) != 0;
    }
    atualizar_partida(aplicacao);
}

static void iniciar_partida(Interface *aplicacao)
{
    wchar_t nome[TAMANHO_NOME] = {0};
    GetWindowTextW(aplicacao->campo_nome, nome, TAMANHO_NOME);
    const wchar_t *comeco = nome + wcsspn(nome, L" \t");
    if (*comeco == L'\0') {
        aplicacao->erro_nome = L"Digite um nome para seu ca\u00e7ador.";
    } else if (WideCharToMultiByte(CP_UTF8, 0, comeco, -1, aplicacao->nome_utf8,
                                   TAMANHO_NOME, NULL, NULL) == 0) {
        aplicacao->erro_nome = L"Escolha um nome mais curto.";
    } else {
        wcscpy(aplicacao->nome, comeco);
        aplicacao->erro_nome = NULL;
        aplicacao->partida_iniciada = true;
        jogo_iniciar(&aplicacao->jogo);
        atualizar_partida(aplicacao);
        return;
    }

    SetFocus(aplicacao->campo_nome);
    InvalidateRect(aplicacao->janela, NULL, FALSE);
}

static void voltar_ao_menu(Interface *aplicacao)
{
    if (aplicacao->partida_iniciada) {
        jogo_limpar(&aplicacao->jogo);
        aplicacao->partida_iniciada = false;
    }
    mostrar_tela(aplicacao, TELA_MENU);
}

static void abrir_inventario(Interface *aplicacao)
{
    HWND lista = aplicacao->lista_itens;
    SendMessageW(lista, LB_RESETCONTENT, 0, 0);

    const Inventario *inventario = &aplicacao->jogo.cacador.inventario;
    size_t quantidade = inventario_quantidade(inventario);
    for (size_t posicao = 1; posicao <= quantidade; ++posicao) {
        Item item;
        if (inventario_buscar(inventario, posicao, &item)) {
            wchar_t nome_item[48] = {0};
            MultiByteToWideChar(CP_UTF8, 0, item.nome, -1, nome_item,
                                sizeof nome_item / sizeof nome_item[0]);
            wchar_t texto[96];
            _snwprintf(texto, sizeof texto / sizeof texto[0],
                       L"  %u. %ls  (+%d PV)", (unsigned)posicao, nome_item, item.cura);
            SendMessageW(lista, LB_ADDSTRING, 0, (LPARAM)texto);
        }
    }

    SendMessageW(lista, LB_SETCURSEL, 0, 0);
    mostrar_tela(aplicacao, TELA_INVENTARIO);
}

static void usar_pocao(Interface *aplicacao)
{
    LRESULT selecionado = SendMessageW(aplicacao->lista_itens, LB_GETCURSEL, 0, 0);
    if (selecionado != LB_ERR) {
        jogo_usar_item(&aplicacao->jogo, (size_t)selecionado + 1);
        mostrar_acao_de_combate(aplicacao);
    }
}

static const ConfigBotao *buscar_botao(int id)
{
    for (int indice = 0; indice < QUANTIDADE_BOTOES; ++indice) {
        if (configuracoes[indice].id == id) {
            return &configuracoes[indice];
        }
    }
    return NULL;
}

static void acionar_botao(Interface *aplicacao, int id)
{
    switch (id) {
        case ID_INICIAR:
            iniciar_partida(aplicacao);
            break;
        case ID_RANKING:
        case ID_RANKING_RESULTADO:
            aplicacao->voltar_do_ranking = aplicacao->tela;
            mostrar_tela(aplicacao, TELA_RANKING);
            break;
        case ID_AJUDA:
            mostrar_tela(aplicacao, TELA_AJUDA);
            break;
        case ID_ATACAR:
            jogo_atacar(&aplicacao->jogo);
            mostrar_acao_de_combate(aplicacao);
            break;
        case ID_DEFENDER:
            jogo_defender(&aplicacao->jogo);
            mostrar_acao_de_combate(aplicacao);
            break;
        case ID_SOMBRA:
            jogo_invocar_sombra(&aplicacao->jogo);
            mostrar_acao_de_combate(aplicacao);
            break;
        case ID_INVENTARIO:
            abrir_inventario(aplicacao);
            break;
        case ID_ABANDONAR:
        case ID_RECUAR_PORTAL:
            jogo_abandonar(&aplicacao->jogo);
            atualizar_partida(aplicacao);
            break;
        case ID_SEGUIR_TEMPLO:
            jogo_escolher_portal(&aplicacao->jogo, false);
            atualizar_partida(aplicacao);
            break;
        case ID_PASSAGEM:
            if (aplicacao->jogo.portal_examinado) {
                jogo_escolher_portal(&aplicacao->jogo, true);
            } else {
                jogo_examinar_portal(&aplicacao->jogo);
            }
            atualizar_partida(aplicacao);
            break;
        case ID_USAR:
            usar_pocao(aplicacao);
            break;
        case ID_VOLTAR_COMBATE:
            mostrar_tela(aplicacao, TELA_COMBATE);
            break;
        case ID_VOLTAR_RANKING:
            mostrar_tela(aplicacao, aplicacao->voltar_do_ranking);
            break;
        case ID_VOLTAR_AJUDA:
        case ID_VOLTAR_MENU:
            voltar_ao_menu(aplicacao);
            break;
    }
}

static void tratar_comando(Interface *aplicacao, int id, int aviso)
{
    if (id == ID_NOME && aviso == EN_CHANGE && aplicacao->erro_nome != NULL) {
        aplicacao->erro_nome = NULL;
        InvalidateRect(aplicacao->janela, NULL, FALSE);
    } else if (id == ID_LISTA && aviso == LBN_DBLCLK && aplicacao->tela == TELA_INVENTARIO) {
        usar_pocao(aplicacao);
    } else if (aviso == BN_CLICKED) {
        const ConfigBotao *config = buscar_botao(id);
        if (config != NULL && config->tela == aplicacao->tela) {
            acionar_botao(aplicacao, id);
        }
    }
}

static LRESULT CALLBACK processar_janela(HWND janela, UINT mensagem, WPARAM parametro,
                                        LPARAM dados)
{
    if (mensagem == WM_NCCREATE) {
        const CREATESTRUCTW *criacao = (const CREATESTRUCTW *)dados;
        Interface *aplicacao = criacao->lpCreateParams;
        aplicacao->janela = janela;
        SetWindowLongPtrW(janela, GWLP_USERDATA, (LONG_PTR)aplicacao);
        return DefWindowProcW(janela, mensagem, parametro, dados);
    }

    Interface *aplicacao = (Interface *)GetWindowLongPtrW(janela, GWLP_USERDATA);
    if (aplicacao == NULL) {
        return DefWindowProcW(janela, mensagem, parametro, dados);
    }

    switch (mensagem) {
        case WM_CREATE: {
            const CREATESTRUCTW *criacao = (const CREATESTRUCTW *)dados;
            return criar_controles(aplicacao, criacao->hInstance) ? 0 : -1;
        }
        case WM_COMMAND:
            tratar_comando(aplicacao, LOWORD(parametro), HIWORD(parametro));
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT pintura;
            HDC contexto = BeginPaint(janela, &pintura);
            tela_pintar(janela, contexto, aplicacao->tela, &aplicacao->jogo,
                        &aplicacao->ranking, aplicacao->nome, aplicacao->erro_nome,
                        efeito_restante(aplicacao));
            EndPaint(janela, &pintura);
            return 0;
        }
        case WM_TIMER:
            if (parametro == ID_TIMER_EFEITO) {
                if (efeito_restante(aplicacao) == 0 || aplicacao->tela != TELA_COMBATE) {
                    KillTimer(janela, ID_TIMER_EFEITO);
                    aplicacao->efeito_ativo = false;
                }
                if (aplicacao->tela == TELA_COMBATE) {
                    InvalidateRect(janela, NULL, FALSE);
                }
                return 0;
            }
            break;
        case WM_ERASEBKGND:
            return 1;
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC contexto = (HDC)parametro;
            SetTextColor(contexto, COR_TEXTO);
            SetBkColor(contexto, COR_CAMPO);
            return (LRESULT)aplicacao->fundo_campo;
        }
        case WM_DRAWITEM: {
            const DRAWITEMSTRUCT *item = (const DRAWITEMSTRUCT *)dados;
            const ConfigBotao *config = buscar_botao((int)item->CtlID);
            if (config != NULL) {
                tela_desenhar_botao(item, config->tipo);
                return TRUE;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(janela, mensagem, parametro, dados);
}

static bool tratar_atalho(Interface *aplicacao, const MSG *mensagem)
{
    if (mensagem->message != WM_KEYDOWN) {
        return false;
    }

    bool repeticao = (mensagem->lParam & (1L << 30)) != 0;
    if (aplicacao->tela == TELA_COMBATE) {
        int id = 0;
        switch (mensagem->wParam) {
            case 'A': id = ID_ATACAR; break;
            case 'D': id = ID_DEFENDER; break;
            case 'E': id = ID_SOMBRA; break;
            case 'I': id = ID_INVENTARIO; break;
        }
        if (id != 0) {
            if (!repeticao) {
                acionar_botao(aplicacao, id);
            }
            return true;
        }
    }

    if (mensagem->wParam == VK_ESCAPE && aplicacao->tela == TELA_INVENTARIO) {
        if (!repeticao) {
            mostrar_tela(aplicacao, TELA_COMBATE);
        }
        return true;
    }
    if (mensagem->wParam != VK_RETURN) {
        return false;
    }

    /* Um Enter mantido pressionado nao deve consumir varios turnos. */
    if (repeticao) {
        return true;
    }

    if (mensagem->hwnd == aplicacao->campo_nome && aplicacao->tela == TELA_MENU) {
        iniciar_partida(aplicacao);
        return true;
    }
    if (mensagem->hwnd == aplicacao->lista_itens && aplicacao->tela == TELA_INVENTARIO) {
        usar_pocao(aplicacao);
        return true;
    }

    const ConfigBotao *config = buscar_botao(GetDlgCtrlID(mensagem->hwnd));
    if (config != NULL && config->tela == aplicacao->tela) {
        SendMessageW(mensagem->hwnd, BM_CLICK, 0, 0);
        return true;
    }
    return false;
}

int WINAPI WinMain(HINSTANCE instancia, HINSTANCE anterior, LPSTR linha_comando, int modo)
{
    (void)anterior;
    (void)linha_comando;

    const wchar_t classe[] = L"DungeonQuestJanela";
    WNDCLASSW configuracao = {0};
    configuracao.lpfnWndProc = processar_janela;
    configuracao.hInstance = instancia;
    configuracao.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    configuracao.lpszClassName = classe;
    if (!RegisterClassW(&configuracao)) {
        MessageBoxW(NULL, L"A janela do jogo n\u00e3o p\u00f4de ser registrada.",
                    L"Dungeon Quest", MB_OK | MB_ICONERROR);
        return 1;
    }

    Interface aplicacao = {0};
    DWORD estilo = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    RECT tamanho = {0, 0, LARGURA_TELA, ALTURA_TELA};
    AdjustWindowRect(&tamanho, estilo, FALSE);

    HWND janela = CreateWindowExW(WS_EX_CONTROLPARENT, classe, L"Solo Leveling: Dungeon Quest",
        estilo, CW_USEDEFAULT, CW_USEDEFAULT, tamanho.right - tamanho.left,
        tamanho.bottom - tamanho.top, NULL, NULL, instancia, &aplicacao);

    int saida = 1;
    if (janela == NULL) {
        MessageBoxW(NULL, L"A janela do jogo n\u00e3o p\u00f4de ser aberta.",
                    L"Dungeon Quest", MB_OK | MB_ICONERROR);
    } else {
        ShowWindow(janela, modo);
        mostrar_tela(&aplicacao, TELA_MENU);

        MSG mensagem;
        BOOL estado;
        while ((estado = GetMessageW(&mensagem, NULL, 0, 0)) > 0) {
            if (!tratar_atalho(&aplicacao, &mensagem) && !IsDialogMessageW(janela, &mensagem)) {
                TranslateMessage(&mensagem);
                DispatchMessageW(&mensagem);
            }
        }
        if (estado == 0) {
            saida = (int)mensagem.wParam;
        }
    }

    if (aplicacao.partida_iniciada) {
        jogo_limpar(&aplicacao.jogo);
    }
    if (aplicacao.fonte != NULL) {
        DeleteObject(aplicacao.fonte);
    }
    if (aplicacao.fundo_campo != NULL) {
        DeleteObject(aplicacao.fundo_campo);
    }
    return saida;
}
