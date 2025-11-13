#include <stdlib.h>

#include "ui/ui_menu.h"
#include "ui/ui_screen.h"
#include "interfaces/keyboard.h" //TODO: For keycodes

#include "hwconfig.h"
#include "ui/ui_strings.h"
#include "core/state.h"
#include "core/graphics.h"

static void gps_tick(UiScreen *self, const UiEvent *ev);
static void gps_draw(UiScreen *self);

static UiScreen g_gps_screen = {
    .tick = gps_tick,
    .draw = gps_draw,
    .ctx  = NULL,
};

/* --- Public API --- */
void ui_gps_screen(void)
{
    ui_push_screen(&g_gps_screen);
}

/* --- UiScreen implementation --- */
static void gps_tick(UiScreen *self, const UiEvent *ev)
{
    (void)self;
    if (!ev) {
        return;
    }

    if (ev->type != UI_EVENT_KEY)
    {
        return;
    }

    // It's a key event

    if (ev->key == KEY_ESC)
    {
        ui_pop_screen();
    }
}

static void gps_draw(UiScreen *self)
{
    (void)self;
    // consts borrowed from existing code for large displays
    // TODO: copy over `layout_t` and helpers
    const uint16_t text_v_offset = 1;
    const uint16_t status_v_pad = 2;
    const uint16_t top_h = 16;
    const uint16_t top_pad = 4;
    const uint16_t line1_h = 20;
    const uint16_t line2_h = 20;
    const uint16_t small_line_v_pad = 2;
    const uint16_t horizontal_pad = 4;
    const uint16_t top_pos_y = top_h - status_v_pad - text_v_offset;
    const uint16_t line1_pos_y = top_h + top_pad + line1_h - small_line_v_pad - text_v_offset;
    const uint16_t line2_pos_y = top_h + top_pad + line1_h + line2_h - small_line_v_pad - text_v_offset;
    const uint16_t bottom_pad = top_pad;
    const uint16_t bottom_pos_y = CONFIG_SCREEN_HEIGHT - bottom_pad - status_v_pad - text_v_offset;
    const point_t top_pos = {horizontal_pad, top_pos_y};
    const point_t line1_pos = {horizontal_pad, line1_pos_y};
    const point_t line2_pos = {horizontal_pad, line2_pos_y};
    const point_t bottom_pos = {horizontal_pad, bottom_pos_y};
    const fontSize_t top_font = FONT_SIZE_8PT;
    const fontSize_t line3_large_font = FONT_SIZE_16PT;
    const fontSize_t bottom_font = FONT_SIZE_8PT;
    const color_t color_white = {255, 255, 255, 255};

    const char *fix_buf, *type_buf;
    const char *status_msg = NULL;
    gfx_clearScreen();

    // Print "GPS" on top bar
    gfx_print(top_pos, top_font, TEXT_ALIGN_CENTER, color_white, currentLanguage->gps);
    point_t fix_pos = {line2_pos.x, CONFIG_SCREEN_HEIGHT * 2 / 5};

    // Print GPS status, if no fix, hide details
    if(!state.gpsDetected)
        status_msg = currentLanguage->noGps;
    else if (!state.settings.gps_enabled)
        status_msg = currentLanguage->gpsOff;
    else if (state.gps_data.fix_quality == FIX_QUALITY_NO_FIX)
        status_msg = currentLanguage->noFix;
    else if (state.gps_data.fix_quality == FIX_QUALITY_ESTIMATED)
        status_msg = currentLanguage->fixLost;
    
    if (status_msg) {
        gfx_print(fix_pos, line3_large_font, TEXT_ALIGN_CENTER, color_white, status_msg);
    } else {
        switch(state.gps_data.fix_quality)
        {
            case FIX_QUALITY_GPS:
                fix_buf = "GPS";
                break;
            case FIX_QUALITY_DGPS:
                fix_buf = "DGPS";
                break;
            case FIX_QUALITY_PPS:
                fix_buf = "PPS";
                break;
            case FIX_QUALITY_RTK:
            case FIX_QUALITY_RTK_FLOAT:
                fix_buf = "RTK";
                break;
            default:
                fix_buf = currentLanguage->error;
                break;
        }

        switch (state.gps_data.fix_type)
        {
            case FIX_TYPE_NOT_AVAIL:
                type_buf = "";
                break;
            case FIX_TYPE_2D:
                type_buf = "2D";
                break;
            case FIX_TYPE_3D:
                type_buf = "3D";
                break;
            default:
                type_buf = currentLanguage->error;
        }

        gfx_print(line1_pos, top_font, TEXT_ALIGN_LEFT, color_white, fix_buf);
        gfx_print(line2_pos, top_font, TEXT_ALIGN_LEFT, color_white, type_buf);

        // Convert from signed lat/lon to unsigned + direction
        int32_t latitude     = abs(state.gps_data.latitude);
        uint8_t latitude_int = latitude / 1000000;
        int32_t latitude_dec = latitude % 1000000;
        char direction_lat  = (state.gps_data.latitude < 0) ? 'S' : 'N';

        int32_t longitude     = abs(state.gps_data.longitude);
        uint8_t longitude_int = longitude / 1000000;
        int32_t longitude_dec = longitude % 1000000;
        char direction_lon   = (state.gps_data.longitude < 0) ? 'W' : 'E';

        gfx_print(line1_pos, top_font, TEXT_ALIGN_RIGHT, color_white, "%d.%.6d%c", latitude_int, latitude_dec, direction_lat);
        gfx_print(line2_pos, top_font, TEXT_ALIGN_RIGHT, color_white, "%d.%.6d%c", longitude_int, longitude_dec, direction_lon);

        gfx_print(bottom_pos, bottom_font, TEXT_ALIGN_CENTER, color_white,
            "S %dkm/h A %dm",
            state.gps_data.speed,
            state.gps_data.altitude
        );
    }

    // Draw compass
    point_t compass_pos = { horizontal_pad * 2, CONFIG_SCREEN_HEIGHT / 2 };
    gfx_drawGPScompass(compass_pos,
                       CONFIG_SCREEN_WIDTH / 9 + 2,
                       state.gps_data.tmg_true,
                       state.gps_data.fix_quality != 0 &&
                       state.gps_data.fix_quality != 6
                    );
    
    // Draw satellites bar graph
    point_t bar_pos = { horizontal_pad + CONFIG_SCREEN_WIDTH * 1 / 3,
                        CONFIG_SCREEN_HEIGHT / 2  };
    gfx_drawGPSgraph(bar_pos,
                     (CONFIG_SCREEN_WIDTH * 2 / 3) - horizontal_pad,
                     CONFIG_SCREEN_HEIGHT / 3,
                     state.gps_data.satellites,
                     state.gps_data.active_sats
    );

    gfx_render();
}

/* MenuItem implementation */

static int gps_menuitem_cb(MenuCmd cmd, void *arg, void *cb_ctx)
{
    (void)cb_ctx;
    (void)arg;

    switch (cmd) {
        case MENU_CMD_SELECT: {
            ui_gps_screen();
            return 1;
        }
        default:
            return 0;
    }
}

const MenuItem g_gps_menu = {
    .kind        = MENU_NODE_ACTION,
    .label       = "GPS",
    .child_count = 0,
    .children    = NULL,
    .binding     = NULL,
    .cb          = gps_menuitem_cb,
    .cb_ctx      = NULL,
};
