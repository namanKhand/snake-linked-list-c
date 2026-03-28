#include <windows.h>
#include "input.h"
#include "snake.h"

/*
 * input.c — hardware keyboard polling via GetAsyncKeyState.
 *
 * GetAsyncKeyState(vk) returns a SHORT where:
 *   bit 15 = key is currently down (0x8000 mask)
 *   bit 0  = key was pressed since last call (we ignore this)
 *
 * We check both arrow keys and WASD so the game works for either scheme.
 * A 180° reversal (e.g. pressing LEFT while moving RIGHT) is silently
 * ignored — you can't crash into yourself that way.
 */

Direction input_poll_direction(Direction current) {
    /* UP / W */
    if ((GetAsyncKeyState(VK_UP) & 0x8000) ||
        (GetAsyncKeyState('W')   & 0x8000))
        return (current != DIR_DOWN) ? DIR_UP : current;

    /* DOWN / S */
    if ((GetAsyncKeyState(VK_DOWN) & 0x8000) ||
        (GetAsyncKeyState('S')     & 0x8000))
        return (current != DIR_UP) ? DIR_DOWN : current;

    /* LEFT / A */
    if ((GetAsyncKeyState(VK_LEFT) & 0x8000) ||
        (GetAsyncKeyState('A')     & 0x8000))
        return (current != DIR_RIGHT) ? DIR_LEFT : current;

    /* RIGHT / D */
    if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) ||
        (GetAsyncKeyState('D')      & 0x8000))
        return (current != DIR_LEFT) ? DIR_RIGHT : current;

    return current;
}

int input_key_pressed(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) ? 1 : 0;
}
