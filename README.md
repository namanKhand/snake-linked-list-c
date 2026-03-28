# Snake — Live Linked List Visualizer

A terminal Snake game written in pure C where the snake body **is** a
live doubly-linked list rendered on screen beside the game board.

Every move you make, you can watch nodes get `malloc`'d onto the head
and `free`'d off the tail in real time.

```
┌──────────────────────────────────────┐   === LINKED LIST ===
│  SCORE: 4   LEVEL: 1                 │
│######################################│   [0xABCD1200]  <- HEAD
│                                      │      (20, 11)
│          oooooO                      │          |
│               @                      │   [0xABCD1220]
│                                      │      (19, 11)
│######################################│          |
└──────────────────────────────────────┘   [0xABCD1240]  <- TAIL
                                              (18, 11)
                                                  |
                                               NULL
```

## Systems Concepts Demonstrated

| Concept | Where |
|---|---|
| Doubly-linked list | `snake.c` — push_head / pop_tail |
| Heap allocation / free | Every move allocates and frees a node |
| Windows Console API | `render.c` — cursor positioning, colors |
| Non-blocking I/O | `input.c` — `GetAsyncKeyState` polls hardware |
| Fixed-timestep game loop | `main.c` — `GetTickCount64` timing |
| Pointer visualization | `render.c` — prints live `%p` addresses |

## Build

Requires **GCC (MinGW)** on Windows.

```bash
make
./snake.exe
```

## Controls

| Key | Action |
|---|---|
| Arrow keys / WASD | Move |
| R | Restart after death |
| ESC | Quit |

## Project Structure

```
snake-linked-list-c/
├── src/
│   ├── main.c      # fixed-timestep game loop
│   ├── snake.h/c   # doubly-linked list (the core data structure)
│   ├── render.h/c  # Windows Console API renderer + list panel
│   ├── input.h/c   # non-blocking keyboard polling
│   └── game.h/c    # game state, collision, food logic
└── Makefile
```
