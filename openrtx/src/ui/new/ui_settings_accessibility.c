#include <string.h>

#include "core/state.h"
#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"

static MenuValueBinding macro_latch_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.macroMenuLatch,
    .on_change = NULL,
};

static const MenuItem m_macro_latch =
    MENU_ITEM_VALUE_BINDING("Macro Latch", &macro_latch_binding);

static const char *voice_names[] =
{
    "OFF",
    "Beep",
    "1",
    "2",
    "3",
};

static MenuValueBinding voice_binding = {
    .kind      = MENU_VAL_ENUM,
    .ptr       = &state.settings.vpLevel,
    .u.enm     = { .names = voice_names, .count = ARRAY_LEN(voice_names) },
    .on_change = NULL,
};

static const MenuItem m_voice =
    MENU_ITEM_VALUE_BINDING("Voice", &voice_binding);

static MenuValueBinding phonetic_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.vpPhoneticSpell,
    .on_change = NULL,
};

static const MenuItem m_phonetic =
    MENU_ITEM_VALUE_BINDING("Phonetic", &phonetic_binding);

static const MenuItem *const accessibility_children[] = {
    &m_macro_latch,
    &m_voice,
    &m_phonetic,
};

const MenuItem g_accessibility_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("Accessibility", accessibility_children);
