# Solo Leveling: Dungeon Quest.

Jogo em C para a atividade de AED, inspirado no anime de ação *Solo Leveling*. Você começa como caçador de Rank E. O Sistema acompanha o avanço entre portais, e a Estátua Divina guarda o último templo. Sua rota muda se você investigar a inscrição encontrada pelo caminho.

## Como jogar no Windows

A interface gráfica usa [Raylib 5.5 para Windows de 32 bits](https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win32_mingw-w64.zip) e um compilador MinGW-w64 compatível, como o do [w64devkit de 32 bits](https://github.com/skeeto/w64devkit/releases/download/v2.11.0/w64devkit-x86-2.11.0.7z.exe). Extraia os dois pacotes e ajuste os caminhos abaixo para as pastas que contêm `bin/gcc.exe` e `include/raylib.h`, respectivamente. Execute os comandos no PowerShell, na pasta do jogo.

```powershell
$devkit = 'C:\Ferramentas\w64devkit'
$raylib = 'C:\Ferramentas\raylib-5.5_win32_mingw-w64'
& "$devkit\bin\gcc.exe" -std=c11 -Wall -Wextra -Wpedantic -I include "-I$raylib\include" src/interface.c src/tela.c src/personagem.c src/jogo.c src/inventario.c src/fila.c src/ordenacao.c src/ranking.c "-L$raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows -o dungeon_quest_gui.exe
.\dungeon_quest_gui.exe
```

Digite o nome do caçador e escolha **Iniciar caça**. Use `WASD` ou as setas para andar pela sala. Aproxime-se de um portal ou de uma inscrição e pressione `F` para interagir. No combate, use o mouse ou os atalhos abaixo. `Tab` e `Enter` também funcionam para navegar pelos controles.

| Tecla | Ação |
| --- | --- |
| `A` | Atacar. Um inimigo derrotado antes de agir perde a vez. |
| `D` | Defender. Reduz o dano normal e, diante de um golpe carregado, anula o dano e gera essência. |
| `E` | **Erga-se**. Gasta uma essência para invocar uma sombra e causar um ataque mais forte. |
| `I` | Abrir o inventário. Selecionar e usar uma poção recupera vida e consome o turno. |
| `Esc` | Fechar o inventário sem gastar o turno. |
| `F11` | Alternar entre tela cheia e janela. |

O aviso de combate informa quando vem um golpe carregado. Se o inimigo estiver quase derrotado, você pode finalizá-lo. Se ele for sobreviver ao ataque, defender pode evitar o golpe e preparar a próxima invocação. Fora do combate, examinar a inscrição revela uma rota opcional com risco e recompensa próprios.

### Versão de terminal

Os comandos abaixo compilam e abrem o menu de texto.

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -I include src/main.c src/jogo.c src/entrada.c src/inventario.c src/fila.c src/ordenacao.c src/ranking.c -o dungeon_quest.exe
.\dungeon_quest.exe
```

No menu principal, **1** inicia e **2** mostra as dez melhores pontuações da sessão. No combate, **1** ataca, **2** defende, **3** invoca uma sombra, **4** abre o inventário e **5** recua.

## Módulos do projeto

Os arquivos `.c` estão em `src/`. Os cabeçalhos `.h` em `include/` declaram as estruturas e funções usadas entre os módulos.

| Arquivo | Responsabilidade |
| --- | --- |
| `main.c` | Menu e controles da versão de terminal |
| `entrada.c` | Leitura de nomes e validação das opções do menu |
| `interface.c` | Entrada por mouse e teclado, navegação e interação com os portais pela Raylib |
| `tela.c` | Cenários, telas, barras e botões desenhados com Raylib |
| `personagem.c` | Movimento e animação do caçador e desenho das criaturas |
| `jogo.c` | Regras compartilhadas de combate, sombras, exploração, experiência e pontuação |
| `inventario.c` | Lista simplesmente encadeada de itens com funções para iniciar, adicionar, buscar, remover, contar e limpar |
| `fila.c` | Fila de inimigos, com consulta ao primeiro e remoção após a derrota |
| `ordenacao.c` | QuickSort recursivo com o primeiro elemento como pivô e varredura do fim para o início, conforme a Aula 15 |
| `ranking.c` | Vetor de pontuações em ordem decrescente |

A lista controla quais poções o caçador pode usar. Cada poção consumida sai do inventário. Antes de entrar na fila, os inimigos de cada masmorra são ordenados pelo QuickSort em ordem crescente de vida inicial mais ataque. O combate segue nessa ordem até que a fila fique vazia, o caçador seja derrotado ou a partida seja abandonada. No ranking, o QuickSort compara os pontos e coloca as maiores pontuações primeiro.

O QuickSort foi escolhido por ter custo médio de O(n log n) e reorganizar os elementos no próprio vetor. A implementação usa a pilha das chamadas recursivas para tratar os intervalos dos dois lados do pivô.

## Funções principais

| Funções | O que fazem na partida |
| --- | --- |
| `jogo_iniciar`, `jogo_escolher_portal` | Preparam a partida e escolhem a próxima masmorra. |
| `jogo_atacar`, `jogo_defender`, `jogo_invocar_sombra` | Resolvem as ações e a resposta do inimigo conforme as regras de turno. |
| `inventario_iniciar`, `inventario_adicionar`, `inventario_buscar`, `inventario_remover`, `inventario_quantidade`, `inventario_limpar` | Criam a lista, guardam e consultam poções, retiram itens usados e liberam os nós ao fim. |
| `jogo_usar_item`, `receber_recompensa` | Aplicam cura, adicionam XP e pontos e tratam a passagem de nível. |
| `contra_atacar`, `concluir_masmorra`, `encerrar_partida` | Detectam a derrota ou a vitória e concluem a partida. |
| `ranking_registrar`, `quicksort_pontuacoes`, `quicksort_inimigos` | Registram os resultados da sessão, ordenam as pontuações e definem a ordem dos combates. |

Os inimigos seguem regras programadas, sem IA ou API externa. O ranking de cada versão dura enquanto aquela instância do jogo está aberta.

O movimento por teclas e pelo tempo entre quadros foi estudado no (https://github.com/Mircio-Ferreira/monitoria_ray_lib_pif/blob/main/src/player.c).
