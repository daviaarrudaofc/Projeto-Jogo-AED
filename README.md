# Solo Leveling: Dungeon Quest.

Jogo em C para a atividade de AED. Você começa como caçador de Rank E. O Sistema acompanha o avanço entre portais, e a Estátua Divina guarda o último templo. Sua rota muda se você investigar a inscrição encontrada pelo caminho.

## Como jogar no Windows

Abra o PowerShell na pasta do projeto com o GCC do MinGW disponível. Os comandos abaixo compilam e abrem a interface gráfica.

```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -I include src/interface.c src/tela.c src/jogo.c src/inventario.c src/fila.c src/ordenacao.c src/ranking.c -mwindows -lgdi32 -o dungeon_quest_gui.exe
.\dungeon_quest_gui.exe
```

Digite o nome do caçador e escolha **Iniciar caça**. No combate, use o mouse ou os atalhos abaixo. `Tab` e `Enter` também funcionam para navegar pelos controles.

| Tecla | Ação |
| --- | --- |
| `A` | Atacar. Um inimigo derrotado antes de agir perde a vez. |
| `D` | Defender. Reduz o dano normal e, diante de um golpe carregado, anula o dano e gera essência. |
| `E` | **Erga-se**. Gasta uma essência para invocar uma sombra e causar um ataque mais forte. |
| `I` | Abrir o inventário. Selecionar e usar uma poção recupera vida e consome o turno. |
| `Esc` | Fechar o inventário sem gastar o turno. |

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
| `interface.c` | Janela Win32, controles e navegação entre telas |
| `tela.c` | Desenho das telas, barras e botões da interface gráfica |
| `jogo.c` | Regras compartilhadas de combate, sombras, exploração, experiência e pontuação |
| `inventario.c` | Lista simplesmente encadeada de itens com funções para iniciar, adicionar, buscar, remover, contar e limpar |
| `fila.c` | Fila de inimigos, com consulta ao primeiro e remoção após a derrota |
| `ordenacao.c` | QuickSort recursivo com o primeiro elemento como pivô e varredura do fim para o início, conforme a Aula 15 |
| `ranking.c` | Vetor de pontuações em ordem decrescente |

A lista controla quais poções o caçador pode usar. Cada poção consumida sai do inventário. Antes de entrar na fila, os inimigos de cada masmorra são ordenados pelo QuickSort em ordem crescente de vida inicial mais ataque. O combate segue nessa ordem até que a fila fique vazia, o caçador seja derrotado ou a partida seja abandonada. O ranking usa o mesmo algoritmo com um comparador de pontuação decrescente.

O ranking de cada versão dura enquanto aquela instância do jogo está aberta.
