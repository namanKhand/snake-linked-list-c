#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "render.h"
#include "snake.h"

/*
 * render.c — Windows Console API renderer.
 *
 * Color scheme: black background, white text, blue accents. Nothing else.
 *
 * UI layout:
 *
 *   +------- SCORE: 4   LEVEL: 1 --------+   |== LINKED LIST ==|
 *   |                                    |   | [ABCD1200] HEAD |
 *   |          ooo>                      |   |   (19, 11)      |
 *   |             *                      |   |       |         |
 *   |                                    |   | [ABCD1220]      |
 *   +------------------------------------+   |   (18, 11)      |
 *   WASD/Arrows: move   R: restart  ESC: quit|       |         |
 *                                            |     NULL        |
 *                                            |=================|
 *                                            | malloc:  47     |
 *                                            | free:    43     |
 *                                            | live:     4     |
 *                                            |=================|
 */

static HANDLE hcon;

/* ── low-level console primitives ───────────────────────────────────── */

static void goto_xy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hcon, c);
}

static void set_color(WORD attr) {
    SetConsoleTextAttribute(hcon, attr);
}

static void print_at(int x, int y, WORD color, const char *s) {
    goto_xy(x, y);
    set_color(color);
    printf("%s", s);
}

/* ── init ────────────────────────────────────────────────────────────── */

void render_init(void) {
    hcon = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO ci = { 1, FALSE };
    SetConsoleCursorInfo(hcon, &ci);

    COORD bufSize = { 120, 42 };
    SetConsoleScreenBufferSize(hcon, bufSize);

    SMALL_RECT winRect = { 0, 0, 119, 37 };
    SetConsoleWindowInfo(hcon, TRUE, &winRect);

    SetConsoleTitleA("Snake — Live Linked List Visualizer");

    /* Black background across the whole buffer */
    COORD origin = { 0, 0 };
    DWORD written;
    FillConsoleOutputCharacter(hcon, ' ', 120 * 42, origin, &written);
    FillConsoleOutputAttribute(hcon, COL_BG, 120 * 42, origin, &written);
}

/* ── board ───────────────────────────────────────────────────────────── */

static char head_glyph(Direction d) {
    switch (d) {
        case DIR_UP:    return '^';
        case DIR_DOWN:  return 'v';
        case DIR_LEFT:  return '<';
        case DIR_RIGHT: return '>';
    }
    return 'O';
}

static void draw_border(int score, int level) {
    char buf[64];

    /* Top: +--- SCORE: N  LEVEL: N ---+ */
    print_at(0, 0, COL_WALL, "+");
    int slen = snprintf(buf, sizeof(buf), " SCORE: %-4d  LEVEL: %-2d ", score, level);
    int ldash = (BOARD_W - slen) / 2;
    for (int x = 1; x <= ldash; x++)
        print_at(x, 0, COL_WALL, "-");
    print_at(ldash + 1, 0, COL_SCORE, buf);
    for (int x = ldash + 1 + slen; x <= BOARD_W; x++)
        print_at(x, 0, COL_WALL, "-");
    print_at(BOARD_W + 1, 0, COL_WALL, "+");

    /* Bottom */
    print_at(0, BOARD_H + 1, COL_WALL, "+");
    for (int x = 1; x <= BOARD_W; x++)
        print_at(x, BOARD_H + 1, COL_WALL, "-");
    print_at(BOARD_W + 1, BOARD_H + 1, COL_WALL, "+");

    /* Sides */
    for (int y = 1; y <= BOARD_H; y++) {
        print_at(0,           y, COL_WALL, "|");
        print_at(BOARD_W + 1, y, COL_WALL, "|");
    }
}

static void clear_interior(void) {
    for (int y = 1; y <= BOARD_H; y++) {
        goto_xy(1, y);
        set_color(COL_BG);
        printf("%*s", BOARD_W, "");
    }
}

static void draw_controls(void) {
    print_at(0, BOARD_H + 2, COL_BODY,
             "  WASD / Arrows: move    R: restart    ESC: quit  ");
}

/*
 * bg_index is accepted but ignored — color scheme is fixed.
 * The parameter is kept so the call site in main.c doesn't need changing.
 */
void render_board(const Snake *s, int food_x, int food_y,
                  int score, int level, int bg_index) {
    (void)bg_index;

    clear_interior();
    draw_border(score, level);
    draw_controls();

    /* Food — bright white * */
    print_at(food_x + 1, food_y + 1, COL_FOOD, "*");

    /* Body: draw tail→head so head glyph always wins */
    const Node *n = s->tail;
    while (n && n != s->head) {
        WORD col = (n == s->tail) ? COL_TAIL : COL_BODY;
        print_at(n->x + 1, n->y + 1, col, "o");
        n = n->prev;
    }

    /* Head — directional glyph, bright white */
    if (s->head) {
        char hg[2] = { head_glyph(s->dir), '\0' };
        print_at(s->head->x + 1, s->head->y + 1, COL_HEAD, hg);
    }
}

/* ── linked list panel ───────────────────────────────────────────────── */

#define MAX_PANEL_NODES  7
#define PANEL_W          24

static void panel_line(int col, int row, WORD color, const char *text) {
    char buf[64];
    snprintf(buf, sizeof(buf), "| %-21.21s|", text);
    print_at(col, row, color, buf);
}

static void panel_divider(int col, int row) {
    char buf[PANEL_W + 2];
    buf[0] = '|';
    for (int i = 1; i < PANEL_W - 1; i++) buf[i] = '=';
    buf[PANEL_W - 1] = '|';
    buf[PANEL_W] = '\0';
    print_at(col, row, COL_HDR, buf);
}

void render_panel(const Snake *s) {
    int col = PANEL_COL;
    int row = 0;

    /* Clear panel to black */
    for (int r = 0; r < BOARD_H + 8; r++) {
        goto_xy(col, r);
        set_color(COL_PANEL_BG);
        printf("%-*s", PANEL_W, "");
    }

    char text[64];

    /* Header */
    panel_divider(col, row++);

    {
        const char *title = "LINKED LIST";
        int pad = (PANEL_W - 2 - (int)strlen(title) - 2) / 2;
        char hdr[64];
        snprintf(hdr, sizeof(hdr), "|%.*s== %s ==%.*s|",
                 pad, "          ", title, pad, "          ");
        print_at(col, row++, COL_HDR, hdr);
    }

    panel_divider(col, row++);

    /* Nodes */
    const Node *n = s->head;
    int shown = 0;

    while (n && shown < MAX_PANEL_NODES) {
        unsigned int addr = (unsigned int)((uintptr_t)n & 0xFFFFFFFF);

        const char *role = "";
        if      (n == s->head && n == s->tail) role = " H+T";
        else if (n == s->head)                 role = " HEAD";
        else if (n == s->tail)                 role = " TAIL";

        /* Address line: HEAD = bright white, TAIL = blue, body = gray */
        WORD addr_col  = (n == s->head) ? COL_ADDR :
                         (n == s->tail) ? COL_TAIL_ADDR : COL_BODY_ADDR;
        WORD coord_col = addr_col;

        snprintf(text, sizeof(text), "[%08X]%s", addr, role);
        panel_line(col, row++, addr_col, text);

        snprintf(text, sizeof(text), "  (%2d, %2d)", n->x, n->y);
        panel_line(col, row++, coord_col, text);

        if (n->next)
            panel_line(col, row++, COL_ARROW, "      |");

        n = n->next;
        shown++;
    }

    if (n) {
        snprintf(text, sizeof(text), "  ... +%d more", s->length - shown);
        panel_line(col, row++, COL_ARROW, text);
    }

    panel_line(col, row++, COL_ARROW, "      |");
    panel_line(col, row++, COL_ADDR,  "    NULL");

    /* Memory stats */
    panel_divider(col, row++);

    snprintf(text, sizeof(text), "malloc:  %d", s->total_allocs);
    panel_line(col, row++, COL_ADDR, text);

    snprintf(text, sizeof(text), "free:    %d", s->total_frees);
    panel_line(col, row++, COL_BODY_ADDR, text);

    snprintf(text, sizeof(text), "live:    %d", s->length);
    panel_line(col, row++, COL_ARROW, text);

    panel_divider(col, row);
}

/* ── game over ───────────────────────────────────────────────────────── */

void render_game_over(int score) {
    int cx = (BOARD_W / 2) - 10;
    int cy = (BOARD_H / 2) - 2;

    char score_buf[32];
    snprintf(score_buf, sizeof(score_buf), "|   Final score: %-4d   |", score);

    print_at(cx, cy,     COL_GAMEOVER_BG,   "+----------------------+");
    print_at(cx, cy + 1, COL_GAMEOVER_BG,   "|    ** GAME OVER **   |");
    print_at(cx, cy + 2, COL_GAMEOVER_MSG,  score_buf);
    print_at(cx, cy + 3, COL_GAMEOVER_HINT, "| R=restart  ESC=quit  |");
    print_at(cx, cy + 4, COL_GAMEOVER_BG,   "+----------------------+");

    set_color(COL_DEFAULT);
}

/* ── cleanup ─────────────────────────────────────────────────────────── */

void render_cleanup(void) {
    set_color(COL_DEFAULT);
    CONSOLE_CURSOR_INFO ci = { 10, TRUE };
    SetConsoleCursorInfo(hcon, &ci);
    goto_xy(0, BOARD_H + 5);
}
