#ifndef UI_NEW_H
#define UI_NEW_H

#include "core/graphics.h"
#include "core/input.h"
#include "ui/ui_screen.h"

bool ui_build_event_from_kbd(const kbd_msg_t *kbd, UiEvent *ev);

/*
 *      ┌─────────────────────────┐
 *      │    status_bar_height    │  status_bar_font
 *      ├─────────────────────────┤  <- user_start_y (Start coordinate)
 *      │                   ┆     │ 
 *      │     user_screen_size    │  User available screen size (Y-dimension)
 *      │                   ┆     │ 
 *      │                   ┆     │
 *      │                   ┆     │
 *      │                   ┆     │
 *      │                   ┆     │
 *      └─────────────────────────┘
 */
typedef struct newlayout_t
{
    uint16_t   status_bar_height;   // Height of the status bar area
    point_t    status_bar_pos;      // Status bar text position
    fontSize_t status_bar_font;     // Status bar text font

    uint16_t   user_start_y;        // Start of user screen area
    uint16_t   user_screen_size;    // User available screen size (Y-dimension)

    fontSize_t small_font;          // Small font size
    fontSize_t text_font;           // Typical font size
    fontSize_t large_font;          // Large font size

    uint16_t horizontal_pad;
} newlayout_t;

extern const newlayout_t *ui_layout;

void ui_drawStatusBar(const char *text);

#endif // UI_NEW_H