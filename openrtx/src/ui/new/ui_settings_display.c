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

static const MenuItem m_brightness = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Brightness",
    .child_count = 0,
    .children    = NULL,
    .binding     = &brightness_binding,
    .cb          = NULL,
    .cb_ctx      = NULL,
};
#endif

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

static const MenuItem m_contrast = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Contrast",
    .child_count = 0,
    .children    = NULL,
    .binding     = &contrast_binding,
    .cb          = NULL,
    .cb_ctx      = NULL,
};
#endif

static const MenuItem m_timer = {
    .kind        = MENU_NODE_UNIMPLEMENTED,
    .label       = "Timer",
    .child_count = 0,
    .children    = NULL,
    .binding     = NULL,
    .cb          = NULL,
    .cb_ctx      = NULL,
};

static MenuValueBinding battery_icon_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.showBatteryIcon,
    .on_change = NULL,
};

static const MenuItem m_battery_icon = {
    .kind        = MENU_NODE_VALUE,
    .label       = "Battery Icon",
    .child_count = 0,
    .children    = NULL,
    .binding     = &battery_icon_binding,
    .cb          = NULL,
    .cb_ctx      = NULL,
};

/* Pointer array of children for this folder */
static const MenuItem *const display_children[] = {
#ifdef CONFIG_SCREEN_BRIGHTNESS
    &m_brightness,
#endif
#ifdef CONFIG_SCREEN_CONTRAST
    &m_contrast,
#endif
    &m_timer,
    &m_battery_icon,
};

const MenuItem g_display_settings_menu = {
    .kind        = MENU_NODE_FOLDER,
    .label       = "Display",
    .child_count = ARRAY_LEN(display_children),
    .children    = display_children,
    .binding     = NULL,
    .cb          = NULL,
    .cb_ctx      = NULL,
};
