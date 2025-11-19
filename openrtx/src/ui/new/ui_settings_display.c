#include <stdint.h>
#include <string.h>

#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"
#include "core/state.h"
#include "interfaces/display.h"

#ifdef CONFIG_SCREEN_BRIGHTNESS
static void brightness_on_change(void *ptr) {
    uint8_t *p = (uint8_t *)ptr;
    display_setBacklightLevel(*p);
}

static MenuValueBinding brightness_binding = {
    .kind      = MENU_VAL_U8,
    .ptr       = &state.settings.brightness,
    .u.u8      = { .min = 0, .max = 100, .step = 5, .wrap = false },
    .on_change = brightness_on_change,
};

static const MenuItem m_brightness =
    MENU_ITEM_VALUE_BINDING("Brightness", &brightness_binding);
#endif // CONFIG_SCREEN_BRIGHTNESS

#ifdef CONFIG_SCREEN_CONTRAST
static void contrast_on_change(void *ptr) {
    uint8_t *p = (uint8_t *)ptr;
    display_setContrast(*p);
}

static MenuValueBinding contrast_binding = {
    .kind      = MENU_VAL_U8,
    .ptr       = &state.settings.contrast,
    .u.u8      = { .min = 0, .max = 255, .step = 4, .wrap = false },
    .on_change = contrast_on_change,
};

static const MenuItem m_contrast =
    MENU_ITEM_VALUE_BINDING("Contrast", &contrast_binding);
#endif // CONFIG_SCREEN_CONTRAST

static const char *timer_names[] =
{
    "OFF",
    "5 s",
    "10 s",
    "15 s",
    "20 s",
    "25 s",
    "30 s",
    "1 min",
    "2 min",
    "3 min",
    "4 min",
    "5 min",
    "15 min",
    "30 min",
    "45 min",
    "1 hour"
};

static MenuValueBinding timer_binding = {
    .kind      = MENU_VAL_ENUM,
    .ptr       = &state.settings.display_timer,
    .u.enm     = { .names = timer_names, .count = ARRAY_LEN(timer_names) },
    .on_change = NULL,
};

static const MenuItem m_timer =
    MENU_ITEM_VALUE_BINDING("Timer", &timer_binding);

#ifndef CONFIG_BAT_NONE
static MenuValueBinding battery_icon_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.showBatteryIcon,
    .on_change = NULL,
};

static const MenuItem m_battery_icon =
    MENU_ITEM_VALUE_BINDING("Battery Icon", &battery_icon_binding);
#endif // CONFIG_BAT_NONE

/* Pointer array of children for this folder */
static const MenuItem *const display_children[] = {
#ifdef CONFIG_SCREEN_BRIGHTNESS
    &m_brightness,
#endif // CONFIG_SCREEN_BRIGHTNESS
#ifdef CONFIG_SCREEN_CONTRAST
    &m_contrast,
#endif // CONFIG_SCREEN_CONTRAST
    &m_timer,
#ifndef CONFIG_BAT_NONE
    &m_battery_icon,
#endif // CONFIG_BAT_NONE
};

const MenuItem g_display_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("Display", display_children);
