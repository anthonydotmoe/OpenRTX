#include "core/graphics.h"
#include "ui/ui_new.h"
#include "hwconfig.h"

// Calculate UI layout depending on vertical resolution
// Tytera MD380, MD-UV380
#if CONFIG_SCREEN_HEIGHT > 127

// Height and padding shown in diagram at beginning of file
static const uint16_t top_h = 16;
static const uint16_t status_v_pad = 2;
static const uint16_t status_bar_height = top_h + status_v_pad;
static const uint16_t horizontal_pad = 4;

static const fontSize_t status_bar_font = FONT_SIZE_8PT;

static const fontSize_t small_font = FONT_SIZE_6PT;
static const fontSize_t text_font = FONT_SIZE_8PT;
static const fontSize_t large_font = FONT_SIZE_12PT;

// Radioddity GD-77
#elif CONFIG_SCREEN_HEIGHT > 63

static const uint16_t top_h = 11;
static const uint16_t status_v_pad = 1;
static const uint16_t status_bar_height = top_h + status_v_pad;
static const uint16_t horizontal_pad = 4;

static const fontSize_t status_bar_font = FONT_SIZE_6PT;

static const fontSize_t small_font = FONT_SIZE_5PT;
static const fontSize_t text_font = FONT_SIZE_6PT;
static const fontSize_t large_font = FONT_SIZE_10PT;

// Radioddity RD-5R
#elif CONFIG_SCREEN_HEIGHT > 47

static const uint16_t top_h = 11;
static const uint16_t status_v_pad = 1;
static const uint16_t status_bar_height = top_h + status_v_pad;
static const uint16_t horizontal_pad = 4;

static const fontSize_t status_bar_font = FONT_SIZE_6PT;

static const fontSize_t small_font = FONT_SIZE_5PT;
static const fontSize_t text_font = FONT_SIZE_6PT;
static const fontSize_t large_font = FONT_SIZE_10PT;

#else
#error Unsupported vertical resolution!
#endif

// Calculate printing positions
static const uint16_t user_start_y = status_bar_height;
static const uint16_t user_screen_size = CONFIG_SCREEN_HEIGHT - user_start_y;

static const newlayout_t new_layout =
{
    .status_bar_height = top_h,
    .status_bar_font = status_bar_font,

    .user_start_y = user_start_y,
    .user_screen_size = user_screen_size,

    .small_font = small_font,
    .text_font  = text_font,
    .large_font = large_font,

    .horizontal_pad = horizontal_pad,
};

const newlayout_t *ui_layout = &new_layout;

void ui_drawStatusBar(const char *text)
{
    color_t white  = { 255, 255, 255, 255 };
    gfx_rect_t status_bar_pos = {
        .x = 0,
        .y = 0,
        .w = CONFIG_SCREEN_WIDTH,
        .h = ui_layout->status_bar_height,
    };

    gfx_drawTextRect(
        status_bar_pos,
        ui_layout->status_bar_font,
        TEXT_ALIGN_CENTER,
        TEXT_VALIGN_MIDDLE,
        white,
        text
    );
}
