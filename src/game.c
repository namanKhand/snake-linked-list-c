#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "snake.h"

/*
 * game.c — game state management and simulation logic.
 *
 * spawn_food() picks a random cell not occupied by the snake.
 * game_update() is the single simulation step called each tick.
 */

/* ── food ────────────────────────────────────────────────────────────── */

/*
 * Pick a random (x, y) inside the board that the snake isn't on.
 * Uses rejection sampling — fine for typical snake lengths.
 */
static void spawn_food(Game *g) {
    int x, y;
    do {
        x = rand() % BOARD_W;
        y = rand() % BOARD_H;
    } while (snake_self_collides(g->snake, x, y));

    g->food_x = x;
    g->food_y = y;
}

/* ── create / free / reset ───────────────────────────────────────────── */

Game *game_create(void) {
    srand((unsigned int)time(NULL));

    Game *g = malloc(sizeof(Game));
    if (!g) return NULL;

    /* Start near the center, heading right */
    g->snake = snake_create(BOARD_W / 2, BOARD_H / 2, 4);
    if (!g->snake) { free(g); return NULL; }

    g->score = 0;
    g->level = 1;
    g->bg_index = 0;
    g->state = STATE_PLAYING;
    spawn_food(g);
    return g;
}

void game_free(Game *g) {
    snake_free(g->snake);
    free(g);
}

void game_reset(Game *g) {
    snake_free(g->snake);
    g->snake = snake_create(BOARD_W / 2, BOARD_H / 2, 4);
    g->score = 0;
    g->level = 1;
    g->bg_index = 0;
    g->state = STATE_PLAYING;
    spawn_food(g);
}

/* ── update ──────────────────────────────────────────────────────────── */

/*
 * One simulation tick. The caller is responsible for only invoking
 * this when a full tick period has elapsed (see main.c game loop).
 *
 * Move sequence:
 *   compute next head cell
 *   → wall hit?   set DEAD, return
 *   → self hit?   set DEAD, return
 *   → food hit?   push head (grow), bump score, respawn food
 *   → otherwise:  push head + pop tail (net zero = slide forward)
 */
void game_update(Game *g) {
    if (g->state == STATE_DEAD) return;

    Snake *s = g->snake;
    int nx = s->head->x;
    int ny = s->head->y;

    /* Advance by one cell in the current direction */
    switch (s->dir) {
        case DIR_UP:    ny--; break;
        case DIR_DOWN:  ny++; break;
        case DIR_LEFT:  nx--; break;
        case DIR_RIGHT: nx++; break;
    }

    /* Wall collision — hit the border, game over */
    if (nx < 0 || nx >= BOARD_W || ny < 0 || ny >= BOARD_H) {
        g->state = STATE_DEAD;
        return;
    }

    /*
     * Self collision — check *before* pushing the new head, otherwise
     * the current tail position (about to be freed) would falsely match.
     */
    if (snake_self_collides(s, nx, ny)) {
        g->state = STATE_DEAD;
        return;
    }

    /* Food collected */
    if (nx == g->food_x && ny == g->food_y) {
        snake_push_head(s, nx, ny);   /* grow: push, no pop */
        g->score++;
        g->level = 1 + g->score / 5; /* level up every 5 points */
        g->bg_index++;             /* advance to next palette color permanently */
        spawn_food(g);
    } else {
        /* Normal move: push new head, pop old tail */
        snake_push_head(s, nx, ny);
        snake_pop_tail(s);
    }
}

/* ── timing ──────────────────────────────────────────────────────────── */

int game_tick_ms(const Game *g) {
    /* Start at 200ms, drop 15ms per level, floor at 50ms */
    int ms = 200 - (g->level - 1) * 15;
    return (ms < 50) ? 50 : ms;
}
