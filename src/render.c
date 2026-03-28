#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "render.h"
#include "snake.h"

/*
 * render.c — Windows Console API renderer.
 *
 * All drawing goes through three primitives:
 *   goto_xy(x, y)      — move cursor to screen cell (x, y)
 *   set_color(attr)    — set foreground/background color word
 *   print_at(...)      — combined goto + color + printf
 *
 * UI layout:
 *
 *   +---------- SCORE: 4   LEVEL: 1 ----------+   |== LINKED LIST ==|
 *   |                                          |   | [0xABCD1200]    |
 *   |         oooO>                            |   |   HEAD (19,11)  |
 *   |              *                           |   |       |         |
 *   |                                          |   | [0xABCD1220]    |
 *   +------------------------------------------+   | (18,11)         |
 *   WASD / Arrows: move   ESC: quit   R: restart   |       |         |
 *                                                   | NULL            |
 *                                                   |================ |
 *                                                   | malloc: 47      |
 *                                                   | free:   43      |
 *                                                   | live:    4      |
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

    COORD origin = { 0, 0 };
    DWORD written;
    FillConsoleOutputCharacter(hcon, ' ', 120 * 42, origin, &written);
    FillConsoleOutputAttribute(hcon, COL_DEFAULT, 120 * 42, origin, &written);
}

/* ── board ───────────────────────────────────────────────────────────── */

/*
 * Draw the border as a box: +---+ corners, - top/bottom, | sides.
 * The top bar shows score and level centered in the wall.
 */
static void draw_border(int score, int level, WORD bg) {
    char buf[64];
    WORD wall  = (WORD)((bg & 0xF0) | 0x07);   /* gray fg on current bg  */
    WORD score_col = (WORD)((bg & 0xF0) | 0x0F); /* bright white on bg    */

    /* ── top bar ── */
    print_at(0, 0, wall, "+");

    /* Left dashes up to the score text */
    int score_len = snprintf(buf, sizeof(buf),
                             " SCORE: %-4d  LEVEL: %-2d ", score, level);
    int left_dashes = (BOARD_W - score_len) / 2;
    for (int x = 1; x <= left_dashes; x++)
        print_at(x, 0, wall, "-");

    print_at(left_dashes + 1, 0, score_col, buf);

    for (int x = left_dashes + 1 + score_len; x <= BOARD_W; x++)
        print_at(x, 0, wall, "-");
    print_at(BOARD_W + 1, 0, wall, "+");

    /* ── bottom bar ── */
    print_at(0, BOARD_H + 1, wall, "+");
    for (int x = 1; x <= BOARD_W; x++)
        print_at(x, BOARD_H + 1, wall, "-");
    print_at(BOARD_W + 1, BOARD_H + 1, wall, "+");

    /* ── side walls ── */
    for (int y = 1; y <= BOARD_H; y++) {
        print_at(0,           y, wall, "|");
        print_at(BOARD_W + 1, y, wall, "|");
    }
}

/*
 * Flood-fill the board interior with the current palette background.
 * Writing spaces with the background color attribute paints the cells.
 */
static void clear_interior(WORD bg) {
    for (int y = 1; y <= BOARD_H; y++) {
        goto_xy(1, y);
        set_color(bg);
        printf("%*s", BOARD_W, "");
    }
}

/*
 * Controls hint row directly below the board.
 * Drawn once per tick so it persists and doesn't flicker.
 */
static void draw_controls(void) {
    print_at(0, BOARD_H + 2, 0x08,
             "  WASD / Arrows: move    R: restart    ESC: quit  ");
}

/*
 * Palette of board backgrounds that cycle every time food is eaten.
 * High nibble = Windows console background color index.
 */
static const WORD bg_palette[] = {
    0x10,  /* navy          — start            */
    0x20,  /* dark green                        */
    0x40,  /* dark red                          */
    0x50,  /* dark magenta                      */
    0x60,  /* dark yellow/olive                 */
    0x30,  /* dark cyan                         */
    0x80,  /* dark gray                         */
};
#define NUM_BG_COLORS  (int)(sizeof(bg_palette) / sizeof(bg_palette[0]))

/*
 * Return a direction glyph for the snake head so it reads like an arrow.
 *   DIR_UP → ^   DIR_DOWN → v   DIR_LEFT → <   DIR_RIGHT → >
 */
static char head_glyph(Direction d) {
    switch (d) {
        case DIR_UP:    return '^';
        case DIR_DOWN:  return 'v';
        case DIR_LEFT:  return '<';
        case DIR_RIGHT: return '>';
    }
    return 'O';
}

void render_board(const Snake *s, int food_x, int food_y,
                  int score, int level, int bg_index) {
    WORD bg         = bg_palette[bg_index % NUM_BG_COLORS];
    WORD snake_col  = (WORD)((bg & 0xF0) | 0x0F); /* bright white on bg */
    WORD food_col   = (WORD)((bg & 0xF0) | 0x0C); /* bright red on bg   */

    clear_interior(bg);
    draw_border(score, level, bg);
    draw_controls();

    /* Food */
    print_at(food_x + 1, food_y + 1, food_col, "*");

    /* Body: draw tail→head so head always wins if there's an overlap */
    char body[2] = { 'o', '\0' };
    const Node *n = s->tail;
    while (n && n != s->head) {
        print_at(n->x + 1, n->y + 1, snake_col, body);
        n = n->prev;
    }

    /* Head — directional arrow glyph */
    if (s->head) {
        char hg[2] = { head_glyph(s->dir), '\0' };
        print_at(s->head->x + 1, s->head->y + 1, snake_col, hg);
    }
}

/* ── linked list panel ───────────────────────────────────────────────── */

/*
 * Panel layout (right of the board):
 *
 *   |== LINKED LIST ==|
 *   | [0xABCD1200]    |   <- HEAD (bright green)
 *   |    (19, 11)     |
 *   |       |         |
 *   | [0xABCD1220]    |
 *   |    (18, 11)     |
 *   |       |         |
 *   |    ...+N more   |   (if list is long)
 *   |       |         |
 *   |     NULL        |
 *   |=================|
 *   | malloc:  47     |
 *   | free:    43     |
 *   | live:     4     |
 *   |=================|
 */
#define MAX_PANEL_NODES  7
#define PANEL_W          24   /* panel total width including borders */
#define PANEL_INNER      (PANEL_W - 3)  /* "| " + content + "|" */

static void panel_line(int col, int row, WORD color, const char *text) {
    char buf[64];
    /* PANEL_INNER = PANEL_W - 3 = 21; use literal so gcc can verify size */
    snprintf(buf, sizeof(buf), "| %-21.21s|", text);
    print_at(col, row, color, buf);
}

static void panel_divider(int col, int row, WORD color) {
    char buf[PANEL_W + 4];
    buf[0] = '|';
    for (int i = 1; i < PANEL_W - 1; i++) buf[i] = '=';
    buf[PANEL_W - 1] = '|';
    buf[PANEL_W] = '\0';
    print_at(col, row, color, buf);
}

void render_panel(const Snake *s) {
    int col = PANEL_COL;
    int row = 0;

    /* Clear panel area */
    for (int r = 0; r < BOARD_H + 8; r++) {
        goto_xy(col, r);
        set_color(COL_PANEL_BG);
        printf("%-*s", PANEL_W, "");
    }

    char text[64];

    /* Header */
    {
        /* Center "LINKED LIST" in the panel */
        const char *title = "LINKED LIST";
        int pad = (PANEL_W - 2 - (int)strlen(title) - 2) / 2;
        char hdr[64];
        snprintf(hdr, sizeof(hdr), "|%.*s== %s ==%.*s|",
                 pad, "          ", title, pad, "          ");
        print_at(col, row++, COL_HDR, hdr);
    }

    /* Nodes */
    const Node *n = s->head;
    int shown = 0;

    while (n && shown < MAX_PANEL_NODES) {
        unsigned int addr = (unsigned int)((uintptr_t)n & 0xFFFFFFFF);

        const char *role = "";
        if      (n == s->head && n == s->tail) role = " H+T";
        else if (n == s->head)                 role = " HEAD";
        else if (n == s->tail)                 role = " TAIL";

        WORD addr_col  = (n == s->head) ? COL_HEAD :
                         (n == s->tail) ? COL_TAIL : COL_ADDR;
        WORD coord_col = (n == s->head) ? COL_HEAD :
                         (n == s->tail) ? COL_TAIL : COL_BODY;

        /* Address line */
        snprintf(text, sizeof(text), "[%08X]%s", addr, role);
        panel_line(col, row++, addr_col, text);

        /* Coordinates */
        snprintf(text, sizeof(text), "  (%2d, %2d)", n->x, n->y);
        panel_line(col, row++, coord_col, text);

        /* Arrow to next node */
        if (n->next)
            panel_line(col, row++, COL_ARROW, "      |");

        n = n->next;
        shown++;
    }

    /* Overflow */
    if (n) {
        snprintf(text, sizeof(text), "  ...+%d more", s->length - shown);
        panel_line(col, row++, COL_ARROW, text);
    }

    /* NULL terminator */
    panel_line(col, row++, COL_ARROW, "      |");
    panel_line(col, row++, COL_ADDR,  "    NULL");

    /* Memory stats divider */
    panel_divider(col, row++, COL_HDR);

    snprintf(text, sizeof(text), "malloc:  %d", s->total_allocs);
    panel_line(col, row++, COL_HEAD, text);

    snprintf(text, sizeof(text), "free:    %d", s->total_frees);
    panel_line(col, row++, COL_TAIL, text);

    snprintf(text, sizeof(text), "live:    %d", s->length);
    panel_line(col, row++, COL_ADDR, text);

    panel_divider(col, row, COL_HDR);
}

/* ── game over ───────────────────────────────────────────────────────── */

void render_game_over(int score) {
    /* Draw a centered box overlay */
    int cx = (BOARD_W / 2) - 10;
    int cy = (BOARD_H / 2) - 2;

    WORD box  = COL_GAMEOVER_BG;
    WORD msg  = COL_GAMEOVER_MSG;
    WORD hint = COL_GAMEOVER_HINT;

    char score_buf[32];
    snprintf(score_buf, sizeof(score_buf), "   Final score:  %-4d  ", score);

    print_at(cx, cy,     box,  "+--------------------+");
    print_at(cx, cy + 1, box,  "|                    |");
    print_at(cx, cy + 1, box,  "|  ** GAME OVER **   |");
    print_at(cx, cy + 2, msg,  score_buf);
    print_at(cx, cy + 3, hint, "|  R=restart ESC=quit|");
    print_at(cx, cy + 4, box,  "+--------------------+");

    set_color(COL_DEFAULT);
}

/* ── cleanup ─────────────────────────────────────────────────────────── */

void render_cleanup(void) {
    set_color(COL_DEFAULT);
    CONSOLE_CURSOR_INFO ci = { 10, TRUE };
    SetConsoleCursorInfo(hcon, &ci);
    goto_xy(0, BOARD_H + 5);
}
