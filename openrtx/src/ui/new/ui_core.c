#include <stdint.h>
#include <stdbool.h>
#include <string.h> // For NULL

#include "core/input.h"
#include "ui/ui_screen.h"

// TODO: Figure this out
#define UI_SCREEN_STACK_MAX 4

static UiScreen *screen_stack[UI_SCREEN_STACK_MAX];
static uint8_t   screen_top = 0;

static void ui_send_focus_event(UiScreen *s, UiEventType type) {
    if (!s) return;
    UiEvent ev = {
        .type = type,
        .key = 0, // unused
    };
    s->tick(s, &ev);
}

UiScreen *ui_current_screen(void) {
    return (screen_top > 0) ? screen_stack[screen_top - 1] : NULL;
}

void ui_push_screen(UiScreen *s) {
    if (!s) return;
    if (screen_top >= UI_SCREEN_STACK_MAX) {
        //TODO: maybe log/assert?
        return;
    }

    UiScreen *old = ui_current_screen();
    if (old) {
        ui_send_focus_event(old, UI_EVENT_FOCUS_LOST);
    }

    screen_stack[screen_top++] = s;

    ui_send_focus_event(s, UI_EVENT_FOCUS_GAIN);
}

void ui_pop_screen(void) {
    if (screen_top == 0) {
        return;
    }

    UiScreen *old = screen_stack[screen_top - 1];
    ui_send_focus_event(old, UI_EVENT_FOCUS_LOST);

    screen_top--;

    UiScreen *now = ui_current_screen();
    if (now) {
        ui_send_focus_event(now, UI_EVENT_FOCUS_GAIN);
    }
}

bool ui_build_event_from_kbd(const kbd_msg_t *kbd, UiEvent *ev) {
    if (!kbd) return false;
    ev->type = UI_EVENT_KEY;
    ev->key  = kbd->keys;
    return true;
}