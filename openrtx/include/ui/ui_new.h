#ifndef UI_NEW_H
#define UI_NEW_H

#include "core/input.h"
#include "ui/ui_screen.h"

bool ui_build_event_from_kbd(const kbd_msg_t *kbd, UiEvent *ev);
#endif