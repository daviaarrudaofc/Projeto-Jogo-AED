#include "personagem.h"

typedef struct {
    Vector2 origem;
    float unidade;
    int direcao;
} PinturaPersonagem;

static const Color contorno = {8, 10, 22, 255};
static const Color lavanda = {220, 197, 255, 255};
static const Color roxo_claro = {165, 121, 255, 255};
static const float intervalo_passo = 0.1f;

static float limitar(float valor, float minimo, float maximo)
{
    if (valor < minimo) {
        return minimo;
    }
    if (valor > maximo) {
        return maximo;
    }
    return valor;
}

void personagem_reiniciar(Personagem *personagem, Vector2 posicao, int direcao)
{
    *personagem = (Personagem){.posicao = posicao, .direcao = direcao < 0 ? -1 : 1};
}

void personagem_mover(Personagem *personagem, Vector2 eixo, float segundos, Rectangle limites)
{
    const float velocidade = 255.0f;
    Vector2 anterior = personagem->posicao;
    float distancia = velocidade * segundos;
    if (eixo.x != 0 && eixo.y != 0) {
        distancia *= 0.7071f;
    }

    personagem->posicao.x = limitar(anterior.x + eixo.x * distancia,
                                    limites.x, limites.x + limites.width);
    personagem->posicao.y = limitar(anterior.y + eixo.y * distancia,
                                    limites.y, limites.y + limites.height);
    if (eixo.x != 0) {
        personagem->direcao = eixo.x < 0 ? -1 : 1;
    }

    personagem->andando = personagem->posicao.x != anterior.x ||
                         personagem->posicao.y != anterior.y;
    if (personagem->andando) {
        personagem->fase += segundos;
        while (personagem->fase >= intervalo_passo * 4) {
            personagem->fase -= intervalo_passo * 4;
        }
    } else {
        personagem->fase = 0;
    }
}

static void bloco(const PinturaPersonagem *pintura, float esquerda, float topo,
                  float largura, float altura, Color cor)
{
    if (pintura->direcao < 0) {
        esquerda = -esquerda - largura;
    }
    Rectangle area = {
        pintura->origem.x + esquerda * pintura->unidade,
        pintura->origem.y + topo * pintura->unidade,
        largura * pintura->unidade,
        altura * pintura->unidade
    };
    DrawRectangleRec(area, cor);
}

static void pernas(const PinturaPersonagem *pintura, float passada,
                   Color tecido, Color joelho, Color sapato)
{
    float perna_esquerda = -4.0f - passada;
    float perna_direita = 1.0f + passada;
    bloco(pintura, perna_esquerda, -7, 3, 6, tecido);
    bloco(pintura, perna_direita, -7, 3, 6, tecido);
    bloco(pintura, perna_direita + 1, -6, 2, 2, joelho);
    bloco(pintura, perna_esquerda, -2, 4, 2, sapato);
    bloco(pintura, perna_direita, -2, 4, 2, sapato);
}

static void lamina(const PinturaPersonagem *pintura, float esquerda, float topo,
                   int tamanho, PosePersonagem pose, Color brilho)
{
    float passo_x = pose == POSE_DEFESA ? 0.0f : 1.0f;

    bloco(pintura, esquerda - 2, topo + 1, 3, 2, contorno);
    for (int trecho = 0; trecho < tamanho; ++trecho) {
        float ponta_x = esquerda + passo_x * trecho;
        float ponta_y = topo - trecho;
        if (pose == POSE_ATAQUE) {
            ponta_y = topo - trecho / 2;
        }
        bloco(pintura, ponta_x, ponta_y, 2, 2, roxo_claro);
        bloco(pintura, ponta_x, ponta_y, 1, 1, brilho);
    }
}

static void desenhar_cacador(const PinturaPersonagem *pintura, float passada,
                             PosePersonagem pose, float impulso)
{
    const Color cabelo = {17, 18, 32, 255};
    const Color mecha = {54, 56, 91, 255};
    const Color pele = {213, 172, 158, 255};
    const Color casaco = {29, 37, 66, 255};
    const Color gola = {71, 87, 135, 255};
    const Color tecido = {40, 47, 75, 255};
    float braco_x = 5 + impulso * 3;
    float braco_y = pose == POSE_DEFESA ? -15 : -10;

    pernas(pintura, passada, tecido, gola, contorno);
    bloco(pintura, -6, -15, 11, 8, contorno);
    bloco(pintura, -5, -15, 10, 7, casaco);
    bloco(pintura, -6 - passada, -9, 4, 5, casaco);
    bloco(pintura, 2 + passada, -9, 4, 4, casaco);
    bloco(pintura, -4, -14, 2, 5, gola);
    bloco(pintura, 0, -14, 1, 6, mecha);
    bloco(pintura, 2, -15, 3, 2, gola);
    bloco(pintura, 4, braco_y, braco_x - 2, 3, tecido);
    bloco(pintura, braco_x, braco_y, 2, 2, pele);

    bloco(pintura, -4, -21, 8, 7, contorno);
    bloco(pintura, -2, -20, 6, 6, pele);
    bloco(pintura, -4, -22, 8, 3, cabelo);
    bloco(pintura, -5, -21, 3, 5, cabelo);
    bloco(pintura, -2, -21, 5, 1, mecha);
    bloco(pintura, 1, -20, 4, 2, cabelo);
    bloco(pintura, -1, -18, 3, 2, cabelo);
    bloco(pintura, 2, -17, 2, 1, lavanda);
    lamina(pintura, braco_x + 2, braco_y, 4, pose, lavanda);
}

static void desenhar_monstro(const PinturaPersonagem *pintura, float passada, float impulso)
{
    const Color pele = {46, 101, 105, 255};
    const Color escama = {82, 152, 130, 255};
    const Color escuro = {24, 53, 66, 255};
    const Color osso = {197, 208, 167, 255};
    const Color olho = {255, 188, 105, 255};
    float garra_x = 6 + impulso * 4;

    pernas(pintura, passada, escuro, pele, contorno);
    bloco(pintura, -7, -14, 13, 9, escuro);
    bloco(pintura, -7, -13, 10, 6, pele);
    bloco(pintura, -8, -12, 3, 7, escuro);
    bloco(pintura, -5, -8, 6, 2, escama);
    bloco(pintura, 3, -12, garra_x - 1, 4, pele);
    bloco(pintura, garra_x, -9, 3, 3, escuro);
    bloco(pintura, garra_x + 2, -9, 2, 1, osso);
    bloco(pintura, garra_x + 2, -7, 2, 1, osso);

    bloco(pintura, -4, -18, 10, 7, pele);
    bloco(pintura, 4, -15, 4, 4, escuro);
    bloco(pintura, -5, -20, 3, 5, escama);
    bloco(pintura, -6, -21, 2, 4, osso);
    bloco(pintura, 3, -19, 2, 2, osso);
    bloco(pintura, -1, -17, 5, 1, escama);
    bloco(pintura, 2, -16, 3, 1, olho);
    bloco(pintura, 5, -12, 1, 2, osso);
    bloco(pintura, 7, -12, 1, 2, osso);
}

static void desenhar_cavaleiro(const PinturaPersonagem *pintura, float passada,
                               PosePersonagem pose, float impulso)
{
    const Color armadura = {107, 35, 65, 255};
    const Color chama = {225, 78, 104, 255};
    const Color escuro = {60, 25, 53, 255};
    const Color viseira = {255, 173, 128, 255};
    float braco_x = 6 + impulso * 3;
    float braco_y = pose == POSE_DEFESA ? -15 : -11;

    pernas(pintura, passada, armadura, chama, contorno);
    bloco(pintura, -7, -15, 5, 11, escuro);
    bloco(pintura, -8, -14, 3, 9, armadura);
    bloco(pintura, -5, -16, 10, 10, armadura);
    bloco(pintura, -8, -16, 5, 4, armadura);
    bloco(pintura, 3, -16, 5, 4, armadura);
    bloco(pintura, -7, -16, 4, 1, chama);
    bloco(pintura, 4, -16, 4, 1, chama);
    bloco(pintura, -2, -13, 6, 2, escuro);
    bloco(pintura, -2, -10, 5, 1, chama);
    bloco(pintura, 5, braco_y, braco_x - 1, 3, armadura);
    bloco(pintura, braco_x + 1, braco_y, 2, 2, chama);

    bloco(pintura, -4, -21, 8, 6, armadura);
    bloco(pintura, -3, -22, 6, 1, chama);
    bloco(pintura, -5, -23, 3, 3, chama);
    bloco(pintura, -7, -24, 4, 2, chama);
    bloco(pintura, -2, -19, 7, 2, contorno);
    bloco(pintura, 1, -19, 3, 1, viseira);
    bloco(pintura, 0, -17, 3, 1, escuro);
    lamina(pintura, braco_x + 3, braco_y, 7, pose, viseira);
}

static void desenhar_estatua(const PinturaPersonagem *pintura, float passada, float impulso)
{
    const Color pedra = {123, 134, 164, 255};
    const Color luz = {175, 182, 205, 255};
    const Color rachadura = {64, 72, 106, 255};
    const Color olhar = {251, 137, 193, 255};
    float punho_x = 7 + impulso * 4;

    pernas(pintura, passada, rachadura, pedra, contorno);
    bloco(pintura, -8, -16, 16, 10, rachadura);
    bloco(pintura, -7, -16, 14, 7, pedra);
    bloco(pintura, -9, -16, 4, 5, luz);
    bloco(pintura, -9, -11, 4, 4, pedra);
    bloco(pintura, 5, -14, punho_x - 2, 4, pedra);
    bloco(pintura, punho_x, -12, 4, 4, luz);
    bloco(pintura, -3, -9, 7, 1, luz);
    bloco(pintura, 0, -15, 1, 5, rachadura);
    bloco(pintura, 1, -13, 2, 1, rachadura);

    bloco(pintura, -6, -22, 12, 6, pedra);
    bloco(pintura, -5, -24, 10, 3, luz);
    bloco(pintura, -6, -25, 2, 2, luz);
    bloco(pintura, 0, -25, 2, 2, luz);
    bloco(pintura, 4, -25, 2, 2, luz);
    bloco(pintura, -3, -20, 3, 1, olhar);
    bloco(pintura, 2, -20, 3, 1, olhar);
    bloco(pintura, 0, -19, 2, 1, luz);
    bloco(pintura, -2, -18, 7, 2, rachadura);
    bloco(pintura, -1, -17, 5, 1, luz);
}

static void desenhar_sombra(const PinturaPersonagem *pintura, PosePersonagem pose,
                            float impulso)
{
    const Color aura = {137, 82, 251, 175};
    const Color manto = {46, 26, 96, 255};
    const Color corpo = {25, 18, 59, 255};
    float braco_x = 5 + impulso * 4;

    bloco(pintura, -8, -14, 2, 5, aura);
    bloco(pintura, -6, -17, 12, 11, manto);
    bloco(pintura, -5, -15, 10, 6, corpo);
    bloco(pintura, -5, -8, 8, 3, aura);
    bloco(pintura, -3, -5, 5, 2, manto);
    bloco(pintura, -4, -3, 2, 1, aura);
    bloco(pintura, 2, -2, 2, 1, aura);
    bloco(pintura, 4, -13, braco_x - 1, 3, manto);

    bloco(pintura, -5, -19, 10, 6, aura);
    bloco(pintura, -4, -21, 8, 6, manto);
    bloco(pintura, -2, -18, 7, 3, corpo);
    bloco(pintura, 0, -17, 4, 1, lavanda);
    lamina(pintura, braco_x + 2, -13, 5, pose, lavanda);
}

void personagem_desenhar(const Personagem *personagem, VisualPersonagem visual,
                         PosePersonagem pose, float progresso, float escala)
{
    float curva = limitar(progresso, 0.0f, 1.0f) * 2;
    if (curva > 1.0f) {
        curva = 2.0f - curva;
    }
    float impulso = pose == POSE_ATAQUE ? curva : 0.0f;
    float recuo = pose == POSE_FERIDO ? curva : 0.0f;
    float passada = 0.0f;
    if (personagem->andando) {
        if (personagem->fase < intervalo_passo) {
            passada = 2.0f;
        } else if (personagem->fase >= intervalo_passo * 2 &&
                   personagem->fase < intervalo_passo * 3) {
            passada = -2.0f;
        }
    }
    PinturaPersonagem pintura = {personagem->posicao, 3.0f * escala, personagem->direcao};

    DrawEllipse((int)personagem->posicao.x, (int)personagem->posicao.y,
                24.0f * escala, 6.0f * escala, Fade(contorno, 0.64f));
    pintura.origem.x += personagem->direcao * (10.0f * impulso - 6.0f * recuo) * escala;

    switch (visual) {
        case VISUAL_CACADOR:
            desenhar_cacador(&pintura, passada, pose, impulso);
            break;
        case VISUAL_MONSTRO:
            desenhar_monstro(&pintura, passada, impulso);
            break;
        case VISUAL_CAVALEIRO:
            desenhar_cavaleiro(&pintura, passada, pose, impulso);
            break;
        case VISUAL_ESTATUA:
            desenhar_estatua(&pintura, passada, impulso);
            break;
        case VISUAL_SOMBRA:
            desenhar_sombra(&pintura, pose, impulso);
            break;
    }

    if (pose == POSE_DEFESA) {
        Color brilho = Fade(lavanda, 0.55f + 0.45f * curva);
        bloco(&pintura, 10, -15, 2, 8, brilho);
        bloco(&pintura, 8, -16, 2, 2, brilho);
        bloco(&pintura, 8, -8, 2, 2, brilho);
    } else if (recuo > 0.0f) {
        Color impacto = Fade((Color){255, 133, 169, 255}, recuo);
        bloco(&pintura, 8, -18, 3, 1, impacto);
        bloco(&pintura, 9, -20, 1, 5, impacto);
        bloco(&pintura, 12, -14, 2, 1, impacto);
    }
}
