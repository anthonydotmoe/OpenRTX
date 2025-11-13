/* ui_menu.h */
#ifndef UI_MENU_H
#define UI_MENU_H

#include <stddef.h>
#include <stdint.h>

#include "ui/ui_screen.h"

typedef enum {
    MENU_NODE_FOLDER,
    MENU_NODE_ACTION,
    MENU_NODE_VALUE,
    MENU_NODE_UNIMPLEMENTED,    // Marker which shows up in the UI
} MenuNodeKind;

typedef enum {
    /// @brief Ask the callback to format the right-hand value text.
    /// @param arg `MenuDrawValueArgs*`
    MENU_CMD_DRAW_VALUE = 0,

    /// @brief Called when user presses ENTER on this item while *not* in edit mode.
    /// @param arg `const UiEvent*` (can be NULL if you don't need it)
    MENU_CMD_SELECT,

    /// @brief Called when we're about to enter the generic edit mode on this item.
    /// @param arg `NULL`
    MENU_CMD_EDIT_BEGIN,

    /// @brief A key press while this item is in edit mode.
    /// @param arg `const UiEvent*`
    MENU_CMD_EDIT_KEY,

    /// @brief User cancelled edit (ESC).
    /// @param arg `NULL`
    MENU_CMD_EDIT_CANCEL,

    /// @brief User accepted edit (ESC).
    /// @param arg `NULL`
    MENU_CMD_EDIT_APPLY,
} MenuCmd;

typedef struct {
    char  *buf;
    size_t buf_len;
} MenuDrawValueArgs;

typedef int (*MenuCb)(MenuCmd cmd, void *arg, void *cb_ctx);

/* type mask values for MENU_VAL_STR */
#define UI_TYPE_UPPERCASE   0x01
#define UI_TYPE_LOWERCASE   0x02
#define UI_TYPE_NUMBER      0x04
#define UI_TYPE_HEX         0x08
#define UI_TYPE_TEXT        UI_TYPE_LOWERCASE | UI_TYPE_UPPERCASE
#define UI_TYPE_ALL         UI_TYPE_LOWERCASE | UI_TYPE_UPPERCASE | UI_TYPE_NUMBER

typedef enum {
    
    /// @brief general text entry
    UI_STR_PROFILE_TEXT,

    /// @brief callsigns / IDs
    UI_STR_PROFILE_CALLSIGN,

    /// @brief PINs, purely digits
    UI_STR_PROFILE_NUMERIC,

    /* TODO: Maybe more later? */
} UiStrProfile;

typedef enum {
    MENU_VAL_BOOL,
    MENU_VAL_U8,
    MENU_VAL_I32,
    MENU_VAL_ENUM,
    MENU_VAL_STR,
} MenuValueKind;

typedef struct {
    MenuValueKind kind;
    void         *ptr;  // pointer to the underlying value
    void (*on_change)(void *ptr); // optional side-effect on change

    union {
        struct { uint8_t min, max, step;   bool wrap; } u8;
        struct { int32_t min, max, step;   bool wrap; } i32;
        struct { const char *const *names; uint8_t count; } enm;
        struct {
            uint8_t      max_len;
            //uint8_t      type_mask; TODO: Use this or `profile`?
            UiStrProfile profile;
        } str;
    } u;
} MenuValueBinding;

typedef struct MenuItem MenuItem;
struct MenuItem {
    MenuNodeKind            kind;
    const char             *label;
    uint8_t                 child_count;
    const MenuItem *const  *children;       // only for FOLDER

    const MenuValueBinding *binding;        // generic value (if any)

    MenuCb                  cb;             // optional custom callback
    void                   *cb_ctx;         // context pointer for cb
};

/* The menu screen itself */
UiScreen *ui_get_menu_screen(void);

/* Helper to reset state to root and push the menu screen on the stack */
void ui_menu_open_root(void);

/* Root menu entry point (global tree) */
extern const MenuItem g_root_menu;

#endif