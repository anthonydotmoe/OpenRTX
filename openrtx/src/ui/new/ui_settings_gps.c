#include <stdint.h>
#include <stdio.h>
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

static int utc_timezone_cb(MenuCmd cmd, void *arg, void *cb_ctx)
{
    (void)cb_ctx;
    int8_t *tz_setting = &state.settings.utc_timezone;

    switch (cmd) {
        case MENU_CMD_DRAW_VALUE: {
            MenuDrawValueArgs *a = (MenuDrawValueArgs *)arg;

            if (*tz_setting == 0) {
                sniprintf(a->buf, a->buf_len, "UTC");
                return 1;
            }

            int8_t tz_hr  = *tz_setting / 2;
            int8_t tz_min = *tz_setting & 1 ? 30 : 0;
            char   sign;

            if(*tz_setting > 0)
            {
                sign = '+';
            }
            else
            {
                sign    = '-';
                tz_hr  *= (-1);
            }

            sniprintf(a->buf, a->buf_len, "UTC%c%d:%02d", sign, tz_hr, tz_min);
            return 1;
        }

        case MENU_CMD_EDIT_BEGIN:
            // Nothing to do
            return 0;
        
        case MENU_CMD_EDIT_KEY: {
            const UiEvent *ev = (const UiEvent *)arg;
            if (!ev) return 0;

            if (ev->key == KEY_UP || ev->key == KEY_DOWN) {
                bool inc = ev->key == KEY_UP;
                if (inc) {
                    *tz_setting += 1;
                } else {
                    *tz_setting -= 1;
                }
                return 1;
            }
            return 0;
        }

        case MENU_CMD_EDIT_CANCEL:
            // Optional: re-sync from some saved copy or just leave it
            return 0;
        
        case MENU_CMD_EDIT_APPLY:
            // Nothing to do, we already wrote into state
            return 0;
        
        case MENU_CMD_SELECT:
            /* TODO: Reconsider this one, doesn't make much sense. */
            return 0;
    }

    return 0;
}

static const MenuItem m_utc_timezone =
    MENU_ITEM_VALUE_CB("Timezone", utc_timezone_cb, NULL);
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
