#ifndef GAME_H
#define GAME_H

/*
 * game.h — top-level game state.
 *
 * Owns the snake, food position, score, and level.
 * All simulation mutation goes through game_update() which advances
 * exactly one tick — keeping logic and rendering cleanly separated.
 */

#include "snake.h"

/* Must match BOARD_W / BOARD_H in render.h */
#define BOARD_W  38
#define BOARD_H  20

typedef enum { STATE_PLAYING, STATE_DEAD } GameState;

typedef struct {
    Snake     *snake;
    int        food_x;
    int        food_y;
    int        score;
    int        level;        /* speed tier: 1 = slow, increases every 5 points */
    GameState  state;
    int        bg_index;     /* increments on each eat; selects persistent bg color */
} Game;

/* Allocate and initialise a new game (snake near center, random food) */
Game *game_create(void);

/* Free the snake and game struct */
void  game_free(Game *g);

/*
 * Advance one simulation tick:
 *   1. Compute next head position from current direction
 *   2. Check wall collision → STATE_DEAD
 *   3. Check self-collision → STATE_DEAD
 *   4. Grow snake if food eaten, else move (push + pop)
 */
void  game_update(Game *g);

/* Reset to a fresh game without reallocating */
void  game_reset(Game *g);

/*
 * Milliseconds per tick. Decreases as level rises so the game gets
 * faster. Capped at 50ms (~20 ticks/sec max).
 */
int   game_tick_ms(const Game *g);

#endif /* GAME_H */
