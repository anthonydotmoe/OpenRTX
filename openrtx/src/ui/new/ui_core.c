#include <stdint.h>
#include <stdbool.h>
#include <string.h> // For NULL

#include "core/input.h"
#include "ui/ui_screen.h"

// TODO: Figure this out
#define UI_SCREEN_STACK_MAX 4

static UiScreen *screen_stack[UI_SCREEN_STACK_MAX];
static uint8_t   screen_top = 0;

void ui_push_screen(UiScreen *s) {
    if (screen_top < UI_SCREEN_STACK_MAX) {
        screen_stack[screen_top++] = s;
    }
}

void ui_pop_screen(void) {
    if (screen_top > 0) {
        screen_top--;
    }
}

UiScreen *ui_current_screen(void) {
    return (screen_top > 0) ? screen_stack[screen_top - 1] : NULL;
}

bool ui_build_event_from_kbd(const kbd_msg_t *kbd, UiEvent *ev) {
    if (!kbd) return false;
    ev->type = 1;
    ev->key  = kbd->keys;
    return true;
}