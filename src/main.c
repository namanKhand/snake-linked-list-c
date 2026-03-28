#include <windows.h>
#include <stdio.h>
#include "game.h"
#include "render.h"
#include "input.h"
#include "snake.h"

/*
 * main.c — fixed-timestep game loop.
 *
 * Systems concept: we decouple simulation speed from CPU speed using
 * GetTickCount64() (a monotonic millisecond timer backed by the Windows
 * HAL). The simulation only advances when a full tick period has elapsed.
 * The CPU sleeps for 10ms between polls so we don't busy-spin.
 *
 * Loop structure:
 *
 *   last_tick = now()
 *   loop:
 *     poll input                        (non-blocking, every iteration)
 *     elapsed = now() - last_tick
 *     if elapsed >= tick_ms:
 *         game_update()                 (simulation step)
 *         render_board() + render_panel()
 *         last_tick = now()
 *     Sleep(10)                         (yield CPU, ~100Hz poll rate)
 */

int main(void) {
    render_init();

    Game *g = game_create();
    if (!g) {
        fprintf(stderr, "Failed to allocate game\n");
        return 1;
    }

    /* Draw the initial frame before the first tick fires */
    render_board(g->snake, g->food_x, g->food_y, g->score, g->level, g->bg_index);
    render_panel(g->snake);

    ULONGLONG last_tick = GetTickCount64();

    while (1) {

        /* ── quit ─────────────────────────────────────────────────── */
        if (input_key_pressed(VK_ESCAPE)) break;

        /* ── game over screen ─────────────────────────────────────── */
        if (g->state == STATE_DEAD) {
            render_game_over(g->score);

            if (input_key_pressed('R')) {
                game_reset(g);
                render_board(g->snake, g->food_x, g->food_y,
                             g->score, g->level, g->bg_index);
                render_panel(g->snake);
                last_tick = GetTickCount64();
            }

            Sleep(10);
            continue;
        }

        /* ── input ────────────────────────────────────────────────── */
        /*
         * Direction changes take effect on the *next* tick so the snake
         * can't reverse into itself in the same frame the key was pressed.
         */
        g->snake->dir = input_poll_direction(g->snake->dir);

        /* ── tick ─────────────────────────────────────────────────── */
        ULONGLONG now     = GetTickCount64();
        ULONGLONG elapsed = now - last_tick;

        if (elapsed >= (ULONGLONG)game_tick_ms(g)) {
            game_update(g);
            last_tick = now;

            /* Redraw on every tick — not on every loop iteration */
            render_board(g->snake, g->food_x, g->food_y,
                         g->score, g->level, g->bg_index);
            render_panel(g->snake);

            if (g->state == STATE_DEAD)
                render_game_over(g->score);
        }

        /* Yield CPU — keeps us at ~100 input polls per second */
        Sleep(10);
    }

    game_free(g);
    render_cleanup();
    return 0;
}
