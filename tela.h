#ifndef TELA_H
#define TELA_H

#include <windows.h>

#include "jogo.h"
#include "ranking.h"

enum { LARGURA_TELA = 920, ALTURA_TELA = 600 };

typedef enum {
    TELA_MENU,
    TELA_COMBATE,
    TELA_INVENTARIO,
    TELA_RANKING,
    TELA_AJUDA,
    TELA_RESULTADO
} TelaJogo;

typedef enum {
    BOTAO_PRIMARIO,
    BOTAO_SECUNDARIO,
    BOTAO_PERIGO
} TipoBotao;

#define COR_CAMPO RGB(23, 29, 48)
#define COR_TEXTO RGB(232, 239, 253)

void tela_pintar(HWND janela, HDC contexto, TelaJogo tela, Jogo *jogo,
                 const Ranking *ranking, const wchar_t *nome,
                 const wchar_t *erro_nome);
void tela_desenhar_botao(const DRAWITEMSTRUCT *item, TipoBotao tipo);

#endif
