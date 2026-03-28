#ifndef RENDER_H
#define RENDER_H

/*
 * render.h — Windows Console API drawing layer.
 *
 * Systems concept: instead of printf(), we control the screen buffer
 * directly. SetConsoleCursorPosition lets us jump to any (x,y) cell,
 * SetConsoleTextAttribute sets foreground/background color, and hiding
 * the cursor prevents flicker during redraws.
 *
 * The right-side panel prints each node's actual heap address (%p),
 * making the linked list's memory layout visible while you play.
 */

#include "snake.h"

/* Board dimensions in cells (not counting the border wall) */
#define BOARD_W  38
#define BOARD_H  20

/* Column where the linked-list panel begins */
#define PANEL_COL  (BOARD_W + 5)

/*
 * Windows console color attributes — low nibble = foreground color.
 * 0=black 1=blue 2=green 3=cyan 4=red 5=magenta 6=yellow 7=gray
 * Add 8 for bright variants (e.g. 0x0A = bright green).
 */
#define COL_DEFAULT   0x07   /* gray on black   */
#define COL_HEAD      0x0A   /* bright green    */
#define COL_BODY      0x02   /* dark green      */
#define COL_TAIL      0x06   /* dark yellow     */
#define COL_FOOD      0x0C   /* bright red      */
#define COL_WALL      0x08   /* dark gray       */
#define COL_HDR       0x0B   /* bright cyan     */
#define COL_ADDR      0x0D   /* bright magenta  */
#define COL_ARROW     0x0E   /* bright yellow   */
#define COL_SCORE     0x0F   /* bright white    */

/* Set up the console handle, hide cursor, resize window */
void render_init(void);

/*
 * Redraw the full game board each tick.
 * Overwrites every interior cell to avoid clearing the screen
 * (system("cls") causes flicker — this approach does not).
 */
void render_board(const Snake *s, int food_x, int food_y,
                  int score, int level);

/*
 * Draw the live linked-list panel on the right side.
 * Shows each node's heap address, (x,y) coords, and pointer arrows.
 */
void render_panel(const Snake *s);

/* Game-over overlay centered on the board */
void render_game_over(int score);

/* Restore default colors on exit */
void render_cleanup(void);

#endif /* RENDER_H */
