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

static const MenuItem m_macro_latch = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Macro Latch",
    .child_count = 0,
    .children    = NULL,
    .cb          = NULL,
    .user        = &macro_latch_binding,
};

static MenuValueBinding voice_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &voice,
    .on_change = NULL,
};

static const MenuItem m_voice = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Voice",
    .child_count = 0,
    .children    = NULL,
    .cb          = NULL,
    .user        = &voice_binding,
};

static MenuValueBinding phonetic_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &phonetic,
    .on_change = NULL,
};

static const MenuItem m_phonetic = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Phonetic",
    .child_count = 0,
    .children    = NULL,
    .cb          = NULL,
    .user        = &phonetic_binding,
};

static const MenuItem *const accessibility_children[] = {
    &m_macro_latch,
    &m_voice,
    &m_phonetic,
};

const MenuItem g_accessibility_settings_menu = {
    .kind           = MENU_NODE_FOLDER,
    .label          = "Accessibility",
    .child_count    = ARRAY_LEN(accessibility_children),
    .children       = accessibility_children,
    .cb             = NULL,
    .user           = NULL,
};