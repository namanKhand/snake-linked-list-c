# snake-linked-list-c

A terminal Snake game written in C where the snake body is a live doubly-linked list — you can watch nodes get allocated and freed in real time as you play.

Built as a systems programming project to show how classic data structures actually work at the memory level.

---

## How it works

Every body segment is a heap-allocated `Node`. Moving the snake is just pointer manipulation at both ends of the list — O(1) per tick, no copying. The right side of the screen shows the list live, with real heap addresses, so you can see `malloc` and `free` happening as the snake moves.

```
+-------- SCORE: 3   LEVEL: 1 ---------+   |== LINKED LIST ==|
|                                      |   | [A1B2C300] HEAD |
|           ooo>                       |   |   (22, 10)      |
|              *                       |   |       |         |
|                                      |   | [A1B2C320]      |
+--------------------------------------+   |   (21, 10)      |
  WASD / Arrows: move   R: restart         |       |         |
                                           |     NULL        |
                                           |=================|
                                           | malloc:  18     |
                                           | free:    15     |
                                           | live:     3     |
                                           |=================|
```

## Systems concepts

- **Doubly-linked list** — push to head, pop from tail, O(1) per move
- **Heap memory** — every node is `malloc`'d and `free`'d live
- **Windows Console API** — direct cursor control, no flicker
- **Non-blocking input** — `GetAsyncKeyState` polls hardware state
- **Fixed-timestep loop** — `GetTickCount64` keeps speed consistent

## Build

Requires GCC (MinGW / MSYS2) on Windows.

```bash
make
./snake.exe
```

Or compile directly:

```bash
gcc -std=c99 -Isrc src/main.c src/snake.c src/render.c src/input.c src/game.c -o snake.exe
```

## Controls

| Key | Action |
|---|---|
| Arrow keys / WASD | Move |
| R | Restart |
| ESC | Quit |

## Structure

```
src/
  snake.h / snake.c   — doubly-linked list (the core)
  render.h / render.c — Windows Console API renderer + list panel
  input.h / input.c   — non-blocking keyboard polling
  game.h  / game.c    — game state, collision, food logic
  main.c              — fixed-timestep game loop
```
