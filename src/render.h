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
 * Windows console color attributes.
 * Byte layout: 0xBF  where B = background nibble, F = foreground nibble.
 * Background: 0=black 1=blue 2=green 3=cyan 4=red 5=magenta 6=yellow 7=gray
 *             Add 8 for bright variants (e.g. 0x10 = dark navy, 0x80 = dark gray)
 * Foreground: same palette (low nibble).
 *
 * Board interior uses a dark navy (0x10) background.
 * Panel uses a dark charcoal (0x80) background.
 */

/* General */
#define COL_DEFAULT   0x07   /* gray on black              */

/* Board interior — dark navy background (high nibble = 1) */
#define COL_BG        0x10   /* black text on navy — blank cell    */
#define COL_BG_FLASH  0x50   /* kept for compat; palette used instead      */
#define COL_HEAD      0x1A   /* bright green on navy               */
#define COL_BODY      0x12   /* dark green on navy                 */
#define COL_TAIL      0x16   /* dark yellow on navy                */
#define COL_FOOD      0x1C   /* bright red on navy                 */
#define COL_WALL      0x17   /* gray on navy (top/bottom walls)    */
#define COL_SCORE     0x1F   /* bright white on navy               */

/* Panel — dark charcoal background (high nibble = 8) */
#define COL_PANEL_BG  0x80   /* blank panel cell                   */
#define COL_HDR       0x8B   /* bright cyan on charcoal            */
#define COL_ADDR      0x8D   /* bright magenta on charcoal         */
#define COL_ARROW     0x8E   /* bright yellow on charcoal          */

/* Game-over overlay colors (kept as raw values, named for clarity) */
#define COL_GAMEOVER_BG   0x4F   /* bright white on red   */
#define COL_GAMEOVER_MSG  0x0F   /* bright white on black */
#define COL_GAMEOVER_HINT 0x08   /* dark gray on black    */

/* Set up the console handle, hide cursor, resize window */
void render_init(void);

/*
 * Redraw the full game board each tick.
 * Overwrites every interior cell to avoid clearing the screen
 * (system("cls") causes flicker — this approach does not).
 */
void render_board(const Snake *s, int food_x, int food_y,
                  int score, int level, int bg_index);

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
