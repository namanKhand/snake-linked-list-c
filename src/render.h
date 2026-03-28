#ifndef RENDER_H
#define RENDER_H

/*
 * render.h — Windows Console API drawing layer.
 *
 * Color scheme: black background, white text, blue accents.
 * Windows console attribute byte: 0xBF (B=background, F=foreground).
 *   0 = black, 1 = blue, 7 = gray, 9 = bright blue, F = bright white
 */

#include "snake.h"

/* Board dimensions in cells (not counting the border wall) */
#define BOARD_W  38
#define BOARD_H  20

/* Column where the linked-list panel begins */
#define PANEL_COL  (BOARD_W + 5)

/* ── Color palette: black / white / blue only ── */
#define COL_DEFAULT      0x07   /* gray on black        — terminal default  */
#define COL_BG           0x00   /* black on black       — board interior    */
#define COL_WALL         0x09   /* bright blue on black — border            */
#define COL_SCORE        0x0F   /* bright white on black — score text       */
#define COL_HEAD         0x0F   /* bright white on black — snake head       */
#define COL_BODY         0x07   /* gray on black         — snake body       */
#define COL_TAIL         0x01   /* dark blue on black    — snake tail       */
#define COL_FOOD         0x0F   /* bright white on black — food             */

#define COL_PANEL_BG     0x00   /* black on black       — panel background  */
#define COL_HDR          0x09   /* bright blue on black — panel header      */
#define COL_ADDR         0x0F   /* bright white on black — node address     */
#define COL_BODY_ADDR    0x07   /* gray on black         — body node addr   */
#define COL_TAIL_ADDR    0x01   /* dark blue on black    — tail node addr   */
#define COL_ARROW        0x09   /* bright blue on black — arrows            */

#define COL_GAMEOVER_BG  0x1F   /* bright white on blue  — game over box   */
#define COL_GAMEOVER_MSG 0x0F   /* bright white on black — score line      */
#define COL_GAMEOVER_HINT 0x07  /* gray on black         — hint line       */

void render_init(void);
void render_board(const Snake *s, int food_x, int food_y,
                  int score, int level, int bg_index);
void render_panel(const Snake *s);
void render_game_over(int score);
void render_cleanup(void);

#endif /* RENDER_H */
