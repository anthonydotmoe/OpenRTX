#include <string.h>

#include "ui/ui_textedit.h"
#include "ui/ui_screen.h"

#include "core/graphics.h"
#include "core/input.h"
#include "interfaces/delays.h"
#include "interfaces/keyboard.h"
#include "hwconfig.h"

static const char *symbols_ITU_T_E161[] =
{
    " 0",
    ",.?1",
    "abc2ABC",
    "def3DEF",
    "ghi4GHI",
    "jkl5JKL",
    "mno6MNO",
    "pqrs7PQRS",
    "tuv8TUV",
    "wxyz9WXYZ",
    "-/*",
    "#"
};

static const char *symbols_ITU_T_E161_callsign[] =
{
    "0 ",
    "1",
    "ABC2",
    "DEF3",
    "GHI4",
    "JKL5",
    "MNO6",
    "PQRS7",
    "TUV8",
    "WXYZ9",
    "-/",
    ""
};

#define TEXTEDIT_SCRATCH_SIZE 256

typedef struct {
    char scratch[TEXTEDIT_SCRATCH_SIZE]; //TODO: evaluate size
    char *target; // final buffer (binding->ptr)
    uint8_t max_len; // max chars (no '\0')
    uint8_t cursor;  // index 0..len (not needed for now)
    uint8_t len;     // current length

    // multi-tap state
    uint8_t active_key; // last numeric key
    uint8_t active_idx; // index into symbols_...[active_key]
    long long last_keypress; // getTick() of last keypress
    bool candidate_active;

    bool dirty;

    const char *title;
    const char *const *symbols;
} TexteditState;

static TexteditState g_textedit_state;

static void textedit_tick(UiScreen *self, const UiEvent *ev);
static void textedit_draw(UiScreen *self);

static UiScreen g_textedit_screen = {
    .tick = textedit_tick,
    .draw = textedit_draw,
    .ctx  = &g_textedit_state,
};

/* --- Internal helpers --- */

static void textedit_reset(TexteditState *st)
{
    st->cursor        = 0;
    st->len           = 0;
    st->active_key    = 0;
    st->active_idx    = 0;
    st->last_keypress = 0;
    st->candidate_active = false;
    st->dirty         = true;
    
    st->scratch[0] = '\0';
}

static void textedit_keypad(TexteditState *st, kbd_msg_t msg)
{
    long long now = getTick();
    uint8_t num_key = input_getPressedChar(msg);

    const char *set = st->symbols[num_key];
    uint8_t num_symbols = set ? strlen(set) : 0;
    if (num_symbols == 0) {
        return;
    }

    bool same_key  = (st->candidate_active && st->active_key == num_key);
    bool timed_out = st->candidate_active &&
                     ((now - st->last_keypress) >= UI_TEXTEDIT_KEY_TIMEOUT);

    // If we're at max len and we'd need to advance cursor, bail
    if (!same_key || timed_out) {
        // Changing key or previous candidate expired.
        // In either case, we want a new candidate at the end.
        if (st->len >= st->max_len) {
            // Can't append
            return;
        }

        st->candidate_active = true;
        st->active_key       = num_key;
        st->active_idx       = 0;

        // Append new candidate char at end
        st->scratch[st->len] = set[0];
        st->len++;
        st->scratch[st->len] = '\0';
    } else {
        // same key, within timeout: cycle candidate
        // The candidate is at `st->len - 1`
        st->active_idx = (uint8_t)((st->active_idx + 1u) % num_symbols);
        if (st->len > 0) {
            st->scratch[st->len - 1] = set[st->active_idx];
        }
    }

    //vp_announceInputChar(st->scratch[st->len - 1]);

    st->last_keypress = now;
    st->dirty         = true;
}

static void textedit_del(TexteditState *st)
{
    if (st->len == 0) {
        // nothing to delete
        st->candidate_active = false;
        st->active_key       = 0;
        st->active_key       = 0;
        st->last_keypress    = 0;
        return;
    }

    // The last character is always removed, whether is was a candidate or not
    st->len--;
    st->scratch[st->len] = '\0';

    // Deletion always ends the multi-tap sequence
    st->candidate_active = false;
    st->active_key       = 0;
    st->active_idx       = 0;
    st->last_keypress    = 0;
}

/* --- Public API --- */
void ui_open_textedit(const UiTextEditParams *p)
{
    if (!p || !p->buf || !p->title) {
        return;
    }
    textedit_reset(&g_textedit_state);

    g_textedit_state.title = p->title ? p->title : "";
    g_textedit_state.max_len = p->max_len;
    g_textedit_state.target = p->buf;

    strncpy(g_textedit_state.scratch, p->buf, g_textedit_state.max_len);
    g_textedit_state.scratch[g_textedit_state.max_len] = '\0';
    g_textedit_state.len = strlen(g_textedit_state.scratch);

    switch (p->profile)
    {
    case UI_STR_PROFILE_CALLSIGN:
        g_textedit_state.symbols = symbols_ITU_T_E161_callsign;
        break;

    default:
        g_textedit_state.symbols = symbols_ITU_T_E161;
        break;
    }

    ui_push_screen(&g_textedit_screen);
}

/* --- UiScreen implementation --- */
static void textedit_tick(UiScreen *self, const UiEvent *ev)
{
    TexteditState *st = (TexteditState *)self->ctx;

    if (!st) {
        return;
    }

    long long now = getTick();

    // 1: Time-based expiry of candidate
    if (st->candidate_active) {
        long long dt = now - st->last_keypress;
        if (dt >= UI_TEXTEDIT_KEY_TIMEOUT) {
            // Commit the candidate visually: keep it in scratch,
            // but stop treating it as "live" multi-tap.
            st->candidate_active = false;
            st->active_key       = 0;
            st->active_idx       = 0;
            st->last_keypress    = 0;
            st->dirty            = true;
        }
    }

    // 2: If no event, nothing else to do this tick.
    if (!ev) {
        return;
    }

    enum key key = ev->key;

    //TODO: hacky
    kbd_msg_t msg = {
        .long_press = 0,
        .keys = key,
    };

    // 3: Handle keys
    if (key == KEY_ESC)
    {
        /* cancel everything */
        ui_pop_screen();
        return;
    }
    else if(key == KEY_ENTER)
    {
        strncpy(st->target, st->scratch, st->max_len);
        ui_pop_screen();
        return;
    }
    else if (key == KEY_UP || key == KEY_DOWN)
    {
        textedit_del(st);
        st->dirty = true;
        return;
    }
    else if(input_isCharPressed(msg))
    {
        textedit_keypad(st, msg);
        return;
    }
}

static void textedit_draw(UiScreen *self)
{
    // consts borrowed from existing code for large displays
    // TODO: copy over `layout_t` and helpers
    const uint16_t text_v_offset = 1;
    const uint16_t status_v_pad = 2;
    const uint16_t top_h = 16;
    const uint16_t top_pad = 4;
    const uint16_t line1_h = 20;
    const uint16_t small_line_v_pad = 2;
    const uint16_t horizontal_pad = 4;
    const uint16_t top_pos_y = top_h - status_v_pad - text_v_offset;
    const uint16_t line1_pos_y = top_h + top_pad + line1_h - small_line_v_pad - text_v_offset;
    const uint16_t menu_h = 16;
    const uint16_t bottom_h = 23;
    const point_t top_pos = {horizontal_pad, top_pos_y};
    const point_t line1_pos = {horizontal_pad, line1_pos_y};
    const fontSize_t top_font = FONT_SIZE_8PT;
    const fontSize_t text_font = FONT_SIZE_8PT;
    const color_t color_white = {255, 255, 255, 255};
    const color_t color_black = {0, 0, 0, 255};

    TexteditState *st = (TexteditState *)self->ctx;

    if(!st || !st->dirty)
    {
        return;
    }

    gfx_clearScreen();

    // Header
    gfx_print(top_pos, top_font, TEXT_ALIGN_CENTER, color_white, st->title);

    // Edit box
    uint16_t rect_width = CONFIG_SCREEN_WIDTH - (horizontal_pad * 2);
    uint16_t rect_height = (CONFIG_SCREEN_HEIGHT - (top_h + bottom_h))/2;
    point_t rect_origin = {(CONFIG_SCREEN_WIDTH - rect_width) / 2,
                            (CONFIG_SCREEN_HEIGHT - rect_height) / 2};
    gfx_drawRect(rect_origin, rect_width, rect_height, color_white, false);

    // Text
    point_t pos = {
        rect_origin.x + horizontal_pad,
        rect_origin.y + top_h + 24,
    };
    uint16_t baseline_y = pos.y;

    int caret_x0 = -1;
    int caret_x1 = -1;
    
    // Draw all characters and track caret spans
    for (uint8_t i = 0; i < st->len; ++i) {
        char c = st->scratch[i];
        if (!c)
            break;
        
        char tmp[2] = { c, 0 };
        point_t sz = gfx_textSize(text_font, tmp);

        gfx_printBuffer(pos, text_font, TEXT_ALIGN_LEFT, color_white, tmp);

        if (st->candidate_active && i == st->len - 1) {
            // Underline span for candidate char
            caret_x0 = pos.x;
            caret_x1 = pos.x + sz.x;
        }

        pos.x += sz.x;
    }

    // If no active candidate, caret is a short bar at the end
    if (!st->candidate_active) {
        caret_x0 = pos.x;
        caret_x1 = pos.x + 6;
    }

    if (caret_x0 >= 0 && caret_x1 > caret_x0) {
        uint16_t ul_y = baseline_y + 2;
        point_t ul_origin = { (uint16_t)caret_x0, ul_y };
        uint16_t ul_width = (uint16_t)(caret_x1 - caret_x0);
        gfx_drawRect(ul_origin, ul_width, 1, color_white, true);
    }

    gfx_render();
    st->dirty = false;
}
