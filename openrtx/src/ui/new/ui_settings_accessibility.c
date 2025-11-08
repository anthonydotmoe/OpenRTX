#include <string.h>

#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"

/* TODO: Replace fake values with real ones */

static bool macrolatch = false;
static bool phonetic   = false;
static bool voice      = false;

/* End TODO:--------------------------------*/

static MenuValueBinding macro_latch_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &macrolatch,
    .on_change = NULL,
};

static const MenuItem m_macro_latch =
    MENU_ITEM_VALUE_BINDING("Macro Latch", &macro_latch_binding);

static MenuValueBinding voice_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &voice,
    .on_change = NULL,
};

static const MenuItem m_voice = {
    .kind        = MENU_NODE_UNIMPLEMENTED,
    .label       = "Voice",
    .child_count = 0,
    .children    = NULL,
    .binding     = &voice_binding,
    .cb          = NULL,
    .cb_ctx      = NULL,
};

static MenuValueBinding phonetic_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &phonetic,
    .on_change = NULL,
};

static const MenuItem m_phonetic = {
    .kind        = MENU_NODE_UNIMPLEMENTED,
    .label       = "Phonetic",
    .child_count = 0,
    .children    = NULL,
    .binding     = &phonetic_binding,
    .cb          = NULL,
    .cb_ctx      = NULL,
};

static const MenuItem *const accessibility_children[] = {
    &m_macro_latch,
    &m_voice,
    &m_phonetic,
};

const MenuItem g_accessibility_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("Accessibility", accessibility_children);
