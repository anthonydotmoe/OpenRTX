#ifndef UI_NEW_H
#define UI_NEW_H

#include "core/input.h"
#include "ui/ui_screen.h"

#define UI_ALIGN_LEFT       0x00
#define UI_ALIGN_RIGHT      0x01
#define UI_ALIGN_CENTER     0x02

#define UI_TYPE_UPPERCASE   0x01
#define UI_TYPE_LOWERCASE   0x02
#define UI_TYPE_NUMBER      0x04
#define UI_TYPE_HEX         0x08
#define UI_TYPE_TEXT        UI_TYPE_LOWERCASE | UI_TYPE_UPPERCASE
#define UI_TYPE_ALL         UI_TYPE_LOWERCASE | UI_TYPE_UPPERCASE | UI_TYPE_NUMBER

bool ui_build_event_from_kbd(const kbd_msg_t *kbd, UiEvent *ev);
#endif