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

#define TEXTEDIT_SCRATCH_SIZE    256
#define TEXTEDIT_CARET_PERIOD_MS 500

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

    // caret blink state
    bool      caret_visible;
    long long last_blink;
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

static void textedit_end_multitap(TexteditState *st, long long *now)
{
        st->candidate_active = false;
        st->active_key       = 0;
        st->active_idx       = 0;
        st->last_keypress    = 0;
        st->last_blink       = *now;
        st->caret_visible    = true;
}

static void textedit_reset(TexteditState *st)
{
    st->cursor        = 0;
    st->len           = 0;
    st->active_key    = 0;
    st->active_idx    = 0;
    st->last_keypress = 0;
    st->candidate_active = false;
    st->dirty         = true;

    st->caret_visible = true;
    st->last_blink    = getTick();
    
    st->scratch[0] = '\0';
}

static bool textedit_insert_char_at_cursor(TexteditState *st, char ch)
{
    if (st->len >= st->max_len) {
        return false; // no room
    }

    // Clamp cursor to valid range
    if (st->cursor > st->len) {
        st->cursor = st->len;
    }

    // Tail length: characters from cursor to end excluding '\0'
    size_t tail_len = (size_t)(st->len - st->cursor);

    // Move tail + '\0' right by one byte
    memmove(&st->scratch[st->cursor + 1],
            &st->scratch[st->cursor],
            tail_len + 1
    );

    st->scratch[st->cursor] = ch;
    st->cursor++;
    st->len++;

    return true;
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

    if (!same_key || timed_out) {
        // Changing key or previous candidate expired.
        // In either case, we want a new candidate at the caret.
        if (!textedit_insert_char_at_cursor(st, set[0])) {
            // Can't append
            textedit_end_multitap(st, &now);
            return;
        }

        st->candidate_active = true;
        st->active_key       = num_key;
        st->active_idx       = 0;
    } else {
        // Continuing the same key within timeout
        // - cycle the candidate character in place
        // - candidate is always at cursor-1 while multi-tap is active
        if (st->cursor == 0) {
            textedit_end_multitap(st, &now);
        }

        // same key, within timeout: cycle candidate
        uint8_t candidate_idx = (uint8_t)(st->cursor - 1);

        st->active_idx = (uint8_t)((st->active_idx + 1u) % num_symbols);
        st->scratch[candidate_idx] = set[st->active_idx];
    }

    //vp_announceInputChar(st->scratch[st->len - 1]);

    st->last_keypress = now;
    st->dirty         = true;
}

static void textedit_backspace(TexteditState *st)
{
    if (st->len == 0 || st->cursor == 0) {
        // nothing to delete
        st->candidate_active = false;
        st->active_key       = 0;
        st->active_idx       = 0;
        st->last_keypress    = 0;
        return;
    }

    // Index of the character we're removing
    uint8_t remove_idx = (uint8_t)(st->cursor - 1);

    // Number of characters after the cursor (not counting the removed one)
    // e.g.: len=5, cursor=3 -> removing index 2 -> characters at 3,4 (2 chars)
    size_t tail_len = (size_t)(st->len - st->cursor);

    // Move tail (plus '\0') left by one
    memmove(&st->scratch[remove_idx],
            &st->scratch[st->cursor],
            tail_len + 1
    );

    st->cursor--;
    st->len--;

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
    if (g_textedit_state.max_len >= TEXTEDIT_SCRATCH_SIZE) {
        g_textedit_state.max_len = TEXTEDIT_SCRATCH_SIZE - 1;
    }

    g_textedit_state.target = p->buf;

    strncpy(g_textedit_state.scratch, p->buf, g_textedit_state.max_len);
    g_textedit_state.scratch[g_textedit_state.max_len] = '\0';

    g_textedit_state.len = (uint8_t)strlen(g_textedit_state.scratch);
    g_textedit_state.cursor = g_textedit_state.len;

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
            textedit_end_multitap(st, &now);
            st->dirty            = true;
        }
    }

    // 1b: caret blink when no candidate is active
    if (!st->candidate_active) {
        long long dt_caret = now - st->last_blink;
        if (dt_caret >= TEXTEDIT_CARET_PERIOD_MS) {
            st->caret_visible = !st->caret_visible;
            st->last_blink    = now;
            st->dirty         = true;
        }
    } else {
        // Hide the caret bar while multi-tap is active
        if (st->caret_visible) {
            st->caret_visible = false;
            st->dirty         = true;
        }
    }

    // 2: If no event, nothing else to do this tick.
    if (!ev) {
        return;
    }

    switch (ev->type) {
    case UI_EVENT_FOCUS_GAIN:
        st->dirty = true;
        return;
    case UI_EVENT_FOCUS_LOST:
        // Nothing for now
        return;
    case UI_EVENT_KEY:
        break; // handled below
    default:
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
        // Enforce invariant
        if (st->len > st->max_len) {
            st->len = st->max_len;
        }

        // Copy string including '\0'
        memcpy(st->target, st->scratch, (size_t)st->len + 1);

        ui_pop_screen();
        return;
    }
    else if (key == KEY_F5)
    {
        textedit_backspace(st);
        // reset caret blink on edit
        st->caret_visible = true;
        st->last_blink    = now;
        st->dirty         = true;
        return;
    }
    else if (key == KEY_DOWN) // cursor left
    {
        if (st->cursor > 0) {
            st->cursor--;
            st->dirty = true;
        }
        // moving the curosr commits multi-tap
        textedit_end_multitap(st, &now);
        return;
    }
    else if (key == KEY_UP) // cursor right
    {
        if (st->cursor < st->len) {
            st->cursor++;
            st->dirty = true;
        }
        // moving the curosr commits multi-tap
        textedit_end_multitap(st, &now);
        return;
    }
    else if(input_isCharPressed(msg))
    {
        textedit_keypad(st, msg);
        st->dirty = true;
        return;
    }
}

#define TEXTEDIT_TEXT_Y_ADJUST 12 //TODO: fix the existance of this magic number

static void textedit_draw(UiScreen *self)
{
    const uint16_t text_v_offset    = 1;
    const uint16_t status_v_pad     = 2;
    const uint16_t top_h            = 16;
    const uint16_t top_pad          = 4;
    const uint16_t line1_h          = 20;
    const uint16_t small_line_v_pad = 2;
    const uint16_t horizontal_pad   = 4;
    const uint16_t bottom_h         = 23;
    const uint8_t  lines_visible    = 4;  // how many lines fit in the box
    const uint8_t  line_spacing     = 2;  // vertical pixels between baselines

    const fontSize_t top_font       = FONT_SIZE_8PT;
    const fontSize_t text_font      = FONT_SIZE_8PT;

    const color_t color_white = (color_t){255, 255, 255, 255};

    TexteditState *st = (TexteditState *)self->ctx;

    if(!st || !st->dirty)
    {
        return;
    }

    gfx_clearScreen();

    // Header
    const uint16_t top_pos_y = top_h - status_v_pad - text_v_offset;
    const point_t  top_pos   = { horizontal_pad, top_pos_y };
    gfx_print(top_pos, top_font, TEXT_ALIGN_CENTER, color_white, st->title);

    // Status bar
    const uint16_t status_bar_pos_y = top_h + top_pad + 8;
    const point_t  status_bar_pos = { horizontal_pad, status_bar_pos_y - status_v_pad - text_v_offset };
    gfx_print(status_bar_pos, FONT_SIZE_6PT, TEXT_ALIGN_RIGHT, color_white, "%" PRIu8 "/%" PRIu8, st->len, st->max_len);
    
    // Softkey labels
    const uint16_t softkey_pos_y = CONFIG_SCREEN_HEIGHT - status_v_pad;
    const point_t  softkey_pos = { horizontal_pad, softkey_pos_y };
    gfx_print(softkey_pos, FONT_SIZE_6PT, TEXT_ALIGN_RIGHT, color_white, "Delete");
    
    /* --- Edit box geometry --- */

    // Font metrics
    uint8_t font_h = gfx_getFontHeight(text_font);

    // Outer rectangle margins on screen
    const uint16_t rect_margin_x   = 4;
    const uint16_t rect_margin_top = status_bar_pos_y + top_pad;

    // Inner padding between rect border and text
    const uint16_t inner_pad_x     = 4;
    const uint16_t inner_pad_y     = 4;

    // Height for multiple lines:
    // top pad = N*font + (N-1)*spacing + bottom pad
    uint16_t rect_width  = CONFIG_SCREEN_WIDTH - (rect_margin_x * 2);
    uint16_t rect_height =
        (uint16_t)(inner_pad_y * 2 +
                   lines_visible * font_h +
                   (lines_visible - 1) * line_spacing);

    point_t rect_origin = {
        (int16_t)rect_margin_x,
        (int16_t)rect_margin_top
    };

    gfx_drawRect(rect_origin, rect_width, rect_height, color_white, false);

    // Base glyph box for the first line
    int16_t base_glyph_top_line0 = (int16_t)(rect_origin.y + inner_pad_y);

    // Text drawing area
    int16_t text_start_x = (int16_t)rect_origin.x + inner_pad_x;
    int16_t text_area_right = (int16_t)(rect_origin.x + rect_width - inner_pad_x);

    /* --- First pass: lay out to find total_lines and caret position --- */

    uint8_t total_lines = 1;
    uint8_t cur_line    = 0;
    int16_t cur_x       = text_start_x;

    uint8_t caret_line_idx = 0;
    int16_t caret_x_full   = text_start_x;
    bool    caret_set      = false;

    if (st->len == 0) {
        // Empty buffer: caret at start of first line
        total_lines    = 1;
        caret_line_idx = 0;
        caret_x_full   = text_start_x;
        caret_set      = true;
    } else {
        for (uint8_t i = 0; i < st->len; ++i) {
            char c = st->scratch[i];
            if (!c) {
                break;
            }

            char tmp[2] = { c, 0 };
            point_t sz = gfx_textSize(text_font, tmp);

            // If this glyph would overflow the text area, wrap to next line
            if ((cur_x + (int16_t)sz.x > text_area_right) && (cur_x != text_start_x)) {
                cur_line++;
                total_lines = (uint8_t)(cur_line + 1);
                cur_x = text_start_x;
            }

            // If cursor is before the first character and not yet set
            if (!caret_set && st->cursor == 0 && i == 0) {
                caret_line_idx = cur_line;
                caret_x_full   = text_start_x;
                caret_set      = true;
            }

            // If cursor is after this character (typical case cursor = i+1)
            if (!caret_set && st->cursor == (uint8_t)(i+1)) {
                caret_line_idx = cur_line;
                caret_x_full   = (int16_t)(cur_x + sz.x); // end of glyph
                caret_set      = true;
            }

            cur_x = (int16_t)(cur_x + sz.x);
        }

        // Cursor at end of text and never set in the loop (e.g. len == cursor but last
        // character was '\0' short-circuiting): fall back to end-of-line.
        if (!caret_set) {
            caret_line_idx = cur_line;
            caret_x_full   = cur_x;
            caret_set      = true;
        }
    }

    /* --- Decide which lines to show so the caret is visible --- */
    uint8_t first_visible_line = 0;
    if (total_lines <= lines_visible) {
        first_visible_line = 0;
    } else {
        // Try to keep the caret on the bottom visible line
        int min_start = (int)caret_line_idx - (int)(lines_visible - 1);
        if (min_start < 0)
            min_start = 0;
        if (min_start > (int)total_lines - (int)lines_visible)
            min_start = (int)total_lines - (int)lines_visible;
        first_visible_line = (uint8_t)min_start;
    }

    /* --- Second pass: draw visible characters --- */

    cur_line = 0;
    cur_x    = text_start_x;

    for (uint8_t i = 0; i < st->len; ++i) {
        char c = st->scratch[i];
        if (!c) {
            break;
        }

        char tmp[2] = { c, 0 };
        point_t sz = gfx_textSize(text_font, tmp);

        // If this glyph would overflow the text area, wrap to next line
        if ((cur_x + (int16_t)sz.x > text_area_right) && (cur_x != text_start_x)) {
            cur_line++;
            cur_x = text_start_x;
        }

        if (cur_line >= first_visible_line &&
            cur_line < (uint8_t)(first_visible_line + lines_visible)) {
            
            uint8_t rel_line = (uint8_t)(cur_line - first_visible_line);

            // Base glyph box for this line
            int16_t base_glyph_top =
                (int16_t)(base_glyph_top_line0 +
                          rel_line * (font_h + line_spacing));
            
            // Actual text baseline for this line
            int16_t glyph_top = (int16_t)(base_glyph_top + TEXTEDIT_TEXT_Y_ADJUST);

            point_t pos = {
                cur_x,
                glyph_top
            };

            gfx_printBuffer(pos, text_font, TEXT_ALIGN_LEFT, color_white, tmp);
        }

        cur_x = (int16_t)(cur_x + sz.x);
    }

    /* --- Caret drawing --- */

    if (!st->candidate_active && st->caret_visible) {
        // Clamp caret line to visible region
        if (caret_line_idx < first_visible_line) {
            caret_line_idx = first_visible_line;
        }
        if (caret_line_idx >= (uint8_t)(first_visible_line + lines_visible)) {
            caret_line_idx = (uint8_t)(first_visible_line + lines_visible - 1);
        }

        uint8_t caret_rel_line = (uint8_t)(caret_line_idx - first_visible_line);

        // Clamp caret horizontally to inside the rectangle
        int caret_x = caret_x_full;
        int rect_left  = rect_origin.x;
        int rect_right = rect_origin.y + (int)rect_width - 1;

        if (caret_x < rect_left)  caret_x = rect_left;
        if (caret_x > rect_right) caret_x = rect_right;

        // Base glyph box for caret's line
        int16_t caret_base_glyph_top =
            (int16_t)(base_glyph_top_line0 +
                      caret_rel_line * (font_h + line_spacing));

        uint16_t caret_height = (uint16_t)(font_h + 2);
        uint16_t caret_top    = (uint16_t)(caret_base_glyph_top - 1);
        point_t  caret_pos    = { (int16_t)caret_x, (int16_t)caret_top };

        gfx_drawRect(caret_pos, 1, caret_height, color_white, true);
    }

    gfx_render();
    st->dirty = false;
}
