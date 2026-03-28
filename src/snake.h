#ifndef SNAKE_H
#define SNAKE_H

/*
 * snake.h — doubly-linked list representation of the snake body.
 *
 * Systems concept: each body segment is a heap-allocated Node.
 * Moving the snake = push_head() + pop_tail()  →  O(1) per tick.
 * Eating food     = push_head() only, no pop    →  snake grows by 1.
 *
 * Memory layout of a 3-segment snake moving right:
 *
 *   HEAD                              TAIL
 *    |                                 |
 *   [x=5,y=3] <-> [x=4,y=3] <-> [x=3,y=3]
 *    prev=NULL      prev=HEAD     prev=MID
 *    next=MID       next=TAIL     next=NULL
 */

typedef struct Node {
    int          x;
    int          y;
    struct Node *next;   /* toward the tail */
    struct Node *prev;   /* toward the head */
} Node;

typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;

typedef struct {
    Node      *head;         /* front — grows here        */
    Node      *tail;         /* back  — shrinks here       */
    int        length;
    Direction  dir;
    int        total_allocs; /* lifetime malloc() calls   */
    int        total_frees;  /* lifetime free() calls     */
} Snake;

/*
 * Allocate a new snake at (x, y) with init_len segments,
 * initially pointing right. Returns NULL on alloc failure.
 */
Snake *snake_create(int x, int y, int init_len);

/*
 * Prepend a new node at (nx, ny).
 * This is how the snake grows: one malloc, wire it to the front.
 * Returns 1 on success, 0 on alloc failure.
 */
int snake_push_head(Snake *s, int nx, int ny);

/*
 * Remove the tail node and free its memory.
 * Called every tick when the snake moves without eating.
 */
void snake_pop_tail(Snake *s);

/*
 * Walk the list and return 1 if any node sits at (x, y).
 * Used for self-collision detection.
 */
int snake_self_collides(const Snake *s, int x, int y);

/* Release all nodes, then the Snake struct. */
void snake_free(Snake *s);

#endif /* SNAKE_H */
