/* ui_menu.h */
#ifndef UI_MENU_H
#define UI_MENU_H

#include <stdint.h>
#include "ui/ui_screen.h"

typedef enum {
    MENU_NODE_FOLDER,
    MENU_NODE_ACTION,
    MENU_NODE_VALUE,
} MenuNodeKind;

typedef enum {
    MENU_CMD_DRAW,
    MENU_CMD_SELECT,
    MENU_CMD_GETSTATE,
    MENU_CMD_EDIT_BEGIN,
    MENU_CMD_EDIT_APPLY,
    MENU_CMD_EDIT_CANCEL,
} MenuCmd;

typedef int (*MenuCb)(MenuCmd cmd, int arg, void *user);

typedef struct MenuItem MenuItem;
struct MenuItem {
    MenuNodeKind           kind;
    const char            *label;
    uint8_t                child_count;
    const MenuItem *const *children;       // only for FOLDER
    MenuCb                 cb;             // for values/actions
    void                  *user;
};

/* The menu screen itself */
UiScreen *ui_get_menu_screen(void);

/* Helper to reset state to root and push the menu screen on the stack */
void ui_menu_open_root(void);

/* Root menu entry point (global tree) */
extern const MenuItem g_root_menu;

typedef enum {
    MENU_VAL_BOOL,
    // TODO: more later: MENU_VAL_I32, MENU_VAL_ENUM, etc.
} MenuValueKind;

typedef struct {
    MenuValueKind kind;
    void         *ptr;  // pointer to the underlying value

    void (*on_change)(void *ptr); // optional side-effect on change
} MenuValueBinding;

#endif