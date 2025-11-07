#include <stdint.h>
#include <string.h>

#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"
#include "core/state.h"

#ifdef CONFIG_GPS
static MenuValueBinding gps_en_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.gps_enabled,
    .on_change = NULL,
};

static const MenuItem m_gps_en = {
    .kind        = MENU_NODE_VALUE,
    .label       = "GPS Enabled",
    .child_count = 0,
    .children    = NULL,
    .binding     = &gps_en_binding,
    .cb          = NULL,
    .cb_ctx      = NULL,
};
#endif

#ifdef CONFIG_RTC
static MenuValueBinding gps_set_time_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.gpsSetTime,
    .on_change = NULL,
};

static const MenuItem m_gps_set_time = {
    .kind        = MENU_NODE_VALUE,
    .label       = "GPS Set Time",
    .child_count = 0,
    .children    = NULL,
    .binding     = &gps_set_time_binding,
    .cb          = NULL,
    .cb_ctx      = NULL,
};

/*
static MenuValueBinding utc_timezone_binding = {
    .kind      = MENU_VAL_I32, //TODO: Not implemented yet
    .ptr       = &state.settings.gpsSetTime,
    .on_change = NULL,
};
*/

static const MenuItem m_utc_timezone = {
    .kind        = MENU_NODE_UNIMPLEMENTED,
    .label       = "UTC Timezone",
    .child_count = 0,
    .children    = NULL,
    .binding     = NULL,
    .cb          = NULL,
    .cb_ctx      = NULL,
};
#endif

static const MenuItem *const gps_children[] = {
#ifdef CONFIG_GPS
    &m_gps_en,
#endif
#ifdef CONFIG_RTC
    &m_gps_set_time,
    &m_utc_timezone,
#endif
};

const MenuItem g_gps_settings_menu = {
    .kind        = MENU_NODE_FOLDER,
    .label       = "GPS",
    .child_count = ARRAY_LEN(gps_children),
    .children    = gps_children,
    .binding     = NULL,
    .cb          = NULL,
    .cb_ctx      = NULL,
};