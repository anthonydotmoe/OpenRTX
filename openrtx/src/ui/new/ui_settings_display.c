#include <stdint.h>
#include <string.h>

#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"
#include "core/state.h"

static void battery_icon_on_change(void *ptr)
{
    (void)ptr;
    /* TODO: do something */
}

static MenuValueBinding battery_icon_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.showBatteryIcon,
    .on_change = battery_icon_on_change,
};

/* Leaf nodes */
static const MenuItem m_brightness = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Brightness",
    .child_count = 0,
    .children    = NULL,
    .cb          = NULL,
    .user        = NULL,
};

static const MenuItem m_timer = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Timer",
    .child_count = 0,
    .children    = NULL,
    .cb          = NULL,
    .user        = NULL,
};

static const MenuItem m_battery_icon = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Battery Icon",
    .child_count = 0,
    .children    = NULL,
    .cb          = NULL,
    .user        = &battery_icon_binding,
};

/* Pointer array of children for this folder */
static const MenuItem *const display_children[] = {
    &m_brightness,
    &m_timer,
    &m_battery_icon,
};

const MenuItem g_display_settings_menu = {
    .kind           = MENU_NODE_FOLDER,
    .label          = "Display",
    .child_count    = ARRAY_LEN(display_children),
    .children       = display_children,
    .cb             = NULL,
    .user           = NULL,
};
