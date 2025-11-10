#include "core/settings.h"

const settings_t default_settings = {
    .brightness = 100,
#ifdef CONFIG_SCREEN_CONTRAST
    .contrast = CONFIG_DEFAULT_CONTRAST,
#else
    .contrast = 255,
#endif
    .sqlLevel = 4,
    .voxLevel = 0,
    .utc_timezone = 0,
    .gps_enabled = false,
    .callsign = "",
    .display_timer = TIMER_30S,
    .m17_can = 0,
    .vpLevel = 0,
    .vpPhoneticSpell = 0,
    .macroMenuLatch = true,
    .m17_can_rx = false,
    .m17_dest = "",
    .showBatteryIcon = false,
    .gpsSetTime = false,
};
