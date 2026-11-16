# Solo Leveling: Dungeon Quest.

Jogo em C para a atividade de AED. Você controla um caçador, atravessa duas masmorras e enfrenta a Estátua Divina. Durante o combate por turnos, você pode atacar ou usar uma poção de cura. Derrotar inimigos rende experiência e pontos.

## Como jogar no Windows

Abra o PowerShell na pasta do projeto com o GCC do MinGW disponível. Para jogar na interface gráfica, compile e abra a janela:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic interface.c tela.c jogo.c inventario.c fila.c ordenacao.c ranking.c -mwindows -lgdi32 -o dungeon_quest_gui.exe
.\dungeon_quest_gui.exe
```

Digite o nome do caçador e escolha **Iniciar caça**. Durante a partida, acompanhe os pontos de vida e a experiência na tela. Para se curar, abra o **Inventário**, selecione uma poção e clique em **Usar poção**. É possível navegar pelos controles com `Tab` e acionar o controle em destaque com `Enter`.

### Versão de terminal

Para jogar pelo menu de texto:

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic main.c jogo.c entrada.c inventario.c fila.c ordenacao.c ranking.c -o dungeon_quest.exe
.\dungeon_quest.exe
```

Escolha **1. Iniciar jogo** e digite o nome do caçador. Durante um combate, **1** ataca, **2** abre o inventário e **3** abandona a partida. Usar uma poção consome o turno. O inimigo contra-ataca se ainda estiver vivo. No menu principal, **2** exibe as dez melhores pontuações da sessão.

## Módulos do projeto

| Arquivo | Responsabilidade |
| --- | --- |
| `main.c` | Menu e controles da versão de terminal |
| `entrada.c` | Leitura de nomes e validação das opções do menu |
| `interface.c` | Janela Win32, controles e navegação entre telas |
| `tela.c` | Desenho das telas, barras e botões da interface gráfica |
| `jogo.c` | Regras compartilhadas de masmorras, combate, cura, experiência, pontuação e fim da partida |
| `inventario.c` | Lista simplesmente encadeada de itens com funções para iniciar, adicionar, buscar, remover, contar e limpar |
| `fila.c` | Fila de inimigos, com consulta ao primeiro e remoção após a derrota |
| `ordenacao.c` | QuickSort recursivo com o primeiro elemento como pivô e varredura do fim para o início, conforme a Aula 15 |
| `ranking.c` | Vetor de pontuações em ordem decrescente |

A lista controla quais poções o caçador pode usar. Cada poção consumida sai do inventário. Antes de entrar na fila, os inimigos de cada masmorra são ordenados pelo QuickSort em ordem crescente de vida inicial mais ataque. O combate segue nessa ordem até que a fila fique vazia, o caçador seja derrotado ou a partida seja abandonada. O ranking usa o mesmo algoritmo com um comparador de pontuação decrescente.

O ranking de cada versão dura enquanto aquela instância do jogo está aberta.
