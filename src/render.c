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
 *   print_at(...)      — combined goto + color + puts
 *
 * The right panel renders the snake's doubly-linked list live:
 * each node shows its heap address so you can watch memory
 * get allocated and freed as the snake moves.
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
    /* printf is fine here — we control position explicitly */
    printf("%s", s);
}

/* ── init ────────────────────────────────────────────────────────────── */

void render_init(void) {
    hcon = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Hide the blinking cursor — cleaner animation */
    CONSOLE_CURSOR_INFO ci = { 1, FALSE };
    SetConsoleCursorInfo(hcon, &ci);

    /* Grow the console buffer so the panel fits beside the board */
    COORD bufSize = { 120, 40 };
    SetConsoleScreenBufferSize(hcon, bufSize);

    SMALL_RECT winRect = { 0, 0, 119, 35 };
    SetConsoleWindowInfo(hcon, TRUE, &winRect);

    SetConsoleTitleA("Snake — Live Linked List Visualizer");

    /* Fill the board region with the navy background */
    COORD origin = { 0, 0 };
    DWORD written;
    FillConsoleOutputCharacter(hcon, ' ', 120 * 40, origin, &written);
    /* Paint the whole buffer with the default color first, then we'll
     * let render_board / render_panel overwrite with their backgrounds. */
    FillConsoleOutputAttribute(hcon, COL_DEFAULT, 120 * 40, origin, &written);
}

/* ── board ───────────────────────────────────────────────────────────── */

/*
 * Draw the border walls using '#' characters.
 * Board cells are offset by 1 so (0,0) game coord → screen (1,1).
 */
static void draw_border(int score, int level) {
    char buf[64];

    /* Top wall with score embedded */
    snprintf(buf, sizeof(buf), " SCORE:%-4d LEVEL:%-2d ", score, level);
    print_at(0, 0, COL_WALL, "#");
    print_at(1, 0, COL_SCORE, buf);

    /* Pad remaining top wall cells */
    int used = (int)strlen(buf) + 1;
    for (int x = used; x <= BOARD_W + 1; x++)
        print_at(x, 0, COL_WALL, "#");

    /* Bottom wall */
    for (int x = 0; x <= BOARD_W + 1; x++)
        print_at(x, BOARD_H + 1, COL_WALL, "#");

    /* Side walls */
    for (int y = 1; y <= BOARD_H; y++) {
        print_at(0,           y, COL_WALL, "#");
        print_at(BOARD_W + 1, y, COL_WALL, "#");
    }
}

/*
 * Clear the board interior by overwriting every cell with a space.
 * This avoids system("cls") which causes visible flicker.
 */
static void clear_interior(void) {
    for (int y = 1; y <= BOARD_H; y++) {
        goto_xy(1, y);
        set_color(COL_BG);          /* dark navy background */
        printf("%*s", BOARD_W, ""); /* flood-fill row with navy */
    }
}

void render_board(const Snake *s, int food_x, int food_y,
                  int score, int level) {
    clear_interior();
    draw_border(score, level);

    /* Food — bright red '@' */
    print_at(food_x + 1, food_y + 1, COL_FOOD, "@");

    /*
     * Draw body from tail toward head so the head glyph always wins
     * if two segments overlap (shouldn't happen in valid play, but safe).
     */
    const Node *n = s->tail;
    while (n && n != s->head) {
        /* Tail node gets a lighter color so it stands out */
        WORD color = (n == s->tail) ? COL_TAIL : COL_BODY;
        print_at(n->x + 1, n->y + 1, color, "o");
        n = n->prev;
    }

    /* Head — bright green 'O', visually distinct */
    if (s->head)
        print_at(s->head->x + 1, s->head->y + 1, COL_HEAD, "O");
}

/* ── linked list panel ───────────────────────────────────────────────── */

/*
 * Render the doubly-linked list as a vertical diagram beside the board.
 *
 * Each node prints as:
 *   [0x1A2B3C4D]          ← heap address (lower 32 bits for readability)
 *      (x, y)  <HEAD      ← coordinates + optional role label
 *        |                ← arrow to next node
 *
 * We cap the display at MAX_ROWS rows. For long snakes a "+N more"
 * line replaces the overflow.
 */
#define MAX_PANEL_NODES  6

void render_panel(const Snake *s) {
    int col = PANEL_COL;
    int row = 0;

    /* Clear the panel area with the charcoal background */
    for (int r = 0; r < (BOARD_H + 2); r++) {
        goto_xy(col, r);
        set_color(COL_PANEL_BG);
        printf("%-25s", ""); /* 25-wide charcoal strip */
    }

    char line[64];

    print_at(col, row++, COL_HDR,   "=== LINKED LIST ===");
    print_at(col, row++, COL_ARROW, "  (live heap memory)");
    row++;  /* blank line */

    const Node *n = s->head;
    int shown = 0;

    while (n && shown < MAX_PANEL_NODES) {
        /*
         * Print the node's heap address.
         * We mask to 32 bits so it fits on one line cleanly.
         * The full pointer is still unique within a run.
         */
        unsigned int addr = (unsigned int)((uintptr_t)n & 0xFFFFFFFF);
        snprintf(line, sizeof(line), "[0x%08X]", addr);

        WORD addr_color = (n == s->head) ? COL_HEAD :
                          (n == s->tail) ? COL_TAIL : COL_ADDR;
        print_at(col, row++, addr_color, line);

        /* Coordinates + role label */
        const char *role = "";
        if (n == s->head && n == s->tail) role = "  <- HEAD+TAIL";
        else if (n == s->head)             role = "  <- HEAD";
        else if (n == s->tail)             role = "  <- TAIL";

        snprintf(line, sizeof(line), "   (%2d,%2d)%s", n->x, n->y, role);
        WORD coord_color = (n == s->head) ? COL_HEAD :
                           (n == s->tail) ? COL_TAIL : COL_BODY;
        print_at(col, row++, coord_color, line);

        /* Downward arrow — omit after the last shown node */
        if (n->next)
            print_at(col + 4, row++, COL_ARROW, "|");

        n = n->next;
        shown++;
    }

    /* Show overflow count if list is longer than panel */
    if (n) {
        snprintf(line, sizeof(line), "   ... +%d more", s->length - shown);
        print_at(col, row++, COL_ARROW, line);
    }

    /* NULL terminator — every linked list ends here */
    print_at(col + 4, row++, COL_ADDR, "|");
    print_at(col + 2, row,   COL_ADDR, "NULL");
}

/* ── game over ───────────────────────────────────────────────────────── */

void render_game_over(int score) {
    int cx = (BOARD_W / 2) - 8;
    int cy = BOARD_H / 2;
    print_at(cx, cy - 1, COL_GAMEOVER_BG,   "                       ");
    print_at(cx, cy - 1, COL_GAMEOVER_BG,   "   *** GAME OVER ***   ");
    char buf[32];
    snprintf(buf, sizeof(buf), "    Final score: %-4d  ", score);
    print_at(cx, cy,     COL_GAMEOVER_MSG,  buf);
    print_at(cx, cy + 1, COL_GAMEOVER_HINT, "  R = restart  ESC = quit  ");
    set_color(COL_DEFAULT);
}

/* ── cleanup ─────────────────────────────────────────────────────────── */

void render_cleanup(void) {
    /* Restore default colors and show cursor again */
    set_color(COL_DEFAULT);
    CONSOLE_CURSOR_INFO ci = { 10, TRUE };
    SetConsoleCursorInfo(hcon, &ci);
    goto_xy(0, BOARD_H + 4);
}
