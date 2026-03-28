#include <stdlib.h>
#include "snake.h"

/*
 * snake.c — doubly-linked list operations for the snake body.
 *
 * The key insight: a snake move is just a pointer operation at each end.
 * No shifting, no copying — O(1) regardless of snake length.
 */

/* ── create ──────────────────────────────────────────────────────────── */

/*
 * Build the initial snake pointing right.
 * We push from left to right so the rightmost node ends up as the head.
 *
 *   push (x-2, y)  →  push (x-1, y)  →  push (x, y)
 *   result: HEAD=(x,y), TAIL=(x-2,y)
 */
Snake *snake_create(int x, int y, int init_len) {
    Snake *s = malloc(sizeof(Snake));
    if (!s) return NULL;

    s->head   = NULL;
    s->tail   = NULL;
    s->length = 0;
    s->dir    = DIR_RIGHT;

    for (int i = init_len - 1; i >= 0; i--) {
        if (!snake_push_head(s, x - i, y)) {
            snake_free(s);
            return NULL;
        }
    }
    return s;
}

/* ── push_head ───────────────────────────────────────────────────────── */

/*
 * Allocate a new node and wire it as the new head.
 *
 * Before:  [old_head] <-> ... <-> [tail]
 * After:   [new_node] <-> [old_head] <-> ... <-> [tail]
 *           ^ s->head
 */
int snake_push_head(Snake *s, int nx, int ny) {
    Node *node = malloc(sizeof(Node));
    if (!node) return 0;

    node->x    = nx;
    node->y    = ny;
    node->prev = NULL;        /* new head has no predecessor */
    node->next = s->head;

    if (s->head)
        s->head->prev = node; /* old head now points back to new head */
    else
        s->tail = node;       /* first node: it's both head and tail */

    s->head = node;
    s->length++;
    return 1;
}

/* ── pop_tail ────────────────────────────────────────────────────────── */

/*
 * Unlink the tail node and free it.
 *
 * Before:  [head] <-> ... <-> [prev] <-> [tail]
 * After:   [head] <-> ... <-> [prev]
 *                              ^ s->tail, next=NULL
 */
void snake_pop_tail(Snake *s) {
    if (!s->tail) return;

    Node *old = s->tail;

    if (old->prev) {
        old->prev->next = NULL;  /* sever the backward link */
        s->tail = old->prev;
    } else {
        /* snake had exactly one node */
        s->head = NULL;
        s->tail = NULL;
    }

    free(old);   /* give the segment back to the heap */
    s->length--;
}

/* ── self collision ──────────────────────────────────────────────────── */

int snake_self_collides(const Snake *s, int x, int y) {
    const Node *n = s->head;
    while (n) {
        if (n->x == x && n->y == y) return 1;
        n = n->next;
    }
    return 0;
}

/* ── free ────────────────────────────────────────────────────────────── */

void snake_free(Snake *s) {
    Node *cur = s->head;
    while (cur) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    free(s);
}
