#ifndef UI_SCREEN_H
#define UI_SCREEN_H

/* ui_screen.h */

/* Full screen interface controlled by main UI thread */
/* OpModes/Apps define `UiScreen` instances */

#include <stdint.h>

#include "interfaces/keyboard.h"

typedef struct UiScreen UiScreen;

typedef enum {
    UI_EVENT_NONE       = 0,
    UI_EVENT_KEY        = 1,
    UI_EVENT_FOCUS_GAIN = 2,
    UI_EVENT_FOCUS_LOST = 3,
} UiEventType;

typedef struct {
    UiEventType type;
    enum key key;   // valid only when type == UI_EVENT_KEY
} UiEvent;

typedef void (*UiScreenTick)(UiScreen *self, const UiEvent *ev);
typedef void (*UiScreenDraw)(UiScreen *self);

struct UiScreen {
    UiScreenTick tick;
    UiScreenDraw draw;
    

    /// @brief screen-local state
    void *ctx;
};

/* Stack management owned by ui_core.c */
void ui_push_screen(UiScreen *s);
void ui_pop_screen(void);
UiScreen *ui_current_screen(void);

#endif