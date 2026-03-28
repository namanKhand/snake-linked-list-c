#ifndef INPUT_H
#define INPUT_H

/*
 * input.h — non-blocking keyboard polling.
 *
 * Systems concept: GetAsyncKeyState() reads the Win32 keyboard state
 * table that the OS interrupt handler updates asynchronously. Bit 15
 * of the return value is 1 if the key is currently held down.
 *
 * This lets the game loop keep running without blocking on stdin —
 * the same technique used by real-time games and system utilities.
 */

#include "snake.h"

/*
 * Sample arrow keys and WASD. Returns a new Direction, or current
 * if no relevant key is pressed (or if the key would reverse the snake).
 */
Direction input_poll_direction(Direction current);

/*
 * Generic single-key poll. vk is a Windows Virtual-Key code (e.g. 'R',
 * VK_ESCAPE). Returns 1 if the key is currently pressed.
 */
int input_key_pressed(int vk);

#endif /* INPUT_H */
