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

static const MenuItem m_gps_en =
    MENU_ITEM_VALUE_BINDING("GPS Enabled", &gps_en_binding);
#endif

#ifdef CONFIG_RTC
static MenuValueBinding gps_set_time_binding = {
    .kind      = MENU_VAL_BOOL,
    .ptr       = &state.settings.gpsSetTime,
    .on_change = NULL,
};

static const MenuItem m_gps_set_time =
    MENU_ITEM_VALUE_BINDING("GPS Set Time", &gps_set_time_binding);

/*
static MenuValueBinding utc_timezone_binding = {
    .kind      = MENU_VAL_I32, //TODO: Not implemented yet
    .ptr       = &state.settings.gpsSetTime,
    .on_change = NULL,
};
*/

static const MenuItem m_utc_timezone =
    MENU_ITEM_UNIMPLEMENTED("UTC Timezone");
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

const MenuItem g_gps_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("GPS", gps_children);
