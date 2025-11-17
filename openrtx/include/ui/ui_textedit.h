#ifndef UI_TEXTEDIT_H
#define UI_TEXTEDIT_H

#include <stddef.h>
#include <stdint.h>

#include "ui/ui_screen.h"
#include "ui/ui_menu.h"

typedef struct {
    /// @brief target buffer (existing value on entry)
    char        *buf;

    /// @brief not including '\0'
    uint8_t      max_len;

    /// @brief entry type
    UiStrProfile profile;

    /// @brief shown as header
    const char  *title;

    /// @brief ioptional completion callback (called on apply or cancel)
    void (*on_done)(bool applied, void *user);
    void *user;
} UiTextEditParams;

#define UI_TEXTEDIT_KEY_TIMEOUT 700 //TODO: Make this a setting in the menu

/// @brief Push the text editor UiScreen onto the screen stack
void ui_open_textedit(const UiTextEditParams *p);

#endif
