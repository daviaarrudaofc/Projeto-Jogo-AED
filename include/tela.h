#ifndef TELA_H
#define TELA_H

#include "jogo.h"
#include "personagem.h"
#include "ranking.h"

enum { LARGURA_TELA = 1000, ALTURA_TELA = 640 };

typedef enum {
    TELA_MENU,
    TELA_ENTRADA,
    TELA_COMBATE,
    TELA_PORTAL,
    TELA_INVENTARIO,
    TELA_RANKING,
    TELA_AJUDA,
    TELA_RESULTADO
} TelaJogo;

typedef enum { BOTAO_PRIMARIO, BOTAO_SECUNDARIO, BOTAO_PERIGO } TipoBotao;
typedef enum { ALVO_NENHUM, ALVO_PORTAO, ALVO_INSCRICAO } AlvoExploracao;
typedef enum { EFEITO_NENHUM, EFEITO_ATAQUE, EFEITO_DEFESA, EFEITO_SOMBRA,
               EFEITO_POCAO } EfeitoTela;

typedef struct {
    Rectangle area;
    const char *texto;
    TipoBotao tipo;
    bool habilitado;
    bool selecionado;
} BotaoTela;

typedef struct {
    TelaJogo tela;
    const Jogo *jogo;
    const Ranking *ranking;
    const char *nome;
    const char *aviso;
    bool editando_nome;
    const Personagem *personagem;
    AlvoExploracao perto;
    EfeitoTela efeito;
    float progresso_efeito;
    float tempo;
    const BotaoTela *botoes;
    size_t quantidade_botoes;
} QuadroTela;

extern const Rectangle AREA_MOVIMENTO;
extern const Vector2 POSICAO_PORTAO;
extern const Vector2 POSICAO_INSCRICAO;
enum { DISTANCIA_INTERACAO = 72 };

void tela_desenhar(const QuadroTela *quadro);

#endif
