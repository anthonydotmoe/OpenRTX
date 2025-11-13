#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h> // for NULL
#include <inttypes.h>

#include "ui/ui_menu.h"
#include "ui/ui_screen.h"
#include "ui/ui_textedit.h"
#include "interfaces/keyboard.h" //TODO: For keycodes

/* menu_draw includes */
#include "hwconfig.h"
#include "core/graphics.h"

/* How many rows we plan to show at once; for scrolling logic */
#define MENU_VISIBLE_ROWS 6

static void u8_adjust(uint8_t *data, bool inc, uint8_t min, uint8_t max, uint8_t step, bool wrap) {
    uint8_t v = *data;

    /* TODO: Evaluate responsibility
    // Sanity check: if min > max, swap
    if (min > max) {
        uint8_t tmp = min;
        min = max;
        max = tmp;
    }
    
    if (step == 0) {
        step = 1;
    }
    */

    // Normalize any out-of-range value
    if (v < min) {
        v = min;
    } else if (v > max) {
        v = max;
    }

    if (inc) {
        // Increment
        if (v >= max) {
            if (wrap) {
                v = min;
            } else {
                v = max;
            }
        } else {
            // Do arithmetic in a wider type to avoid overflow
            unsigned int tmp = (unsigned int)v + (unsigned int)step;
            if (tmp > max) {
                if (wrap) {
                    v = min;
                } else {
                    v = max;
                }
            } else {
                v = (uint8_t)tmp;
            }
        }
    } else {
        // Decrement
        if (v <= min) {
            if (wrap) {
                v = max;
            } else {
                v = min;
            }
        } else {
            int tmp = (int)v - (int)step;
            if (tmp < (int)min) {
                if (wrap) {
                    v = max;
                } else {
                    v = min;
                }
            } else {
                v = (uint8_t)tmp;
            }
        }
    }
    
    *data = v;
}


static void i32_adjust(int32_t *data, bool inc,
                       int32_t min, int32_t max,
                       int32_t step, bool wrap)
{
    int32_t v = *data;

    // Normalize any out-of-range value
    if (v < min) {
        v = min;
    } else if (v > max) {
        v = max;
    }

    if (inc) {
        // Increment
        if (v >= max) {
            v = wrap ? min : max;
        } else {
            int64_t tmp = (int64_t)v + (int64_t)step;
            if (tmp > (int64_t)max) {
                v = wrap ? min : max;
            } else {
                v = (int32_t)tmp;
            }
        }
    } else {
        // Decrement
        if (v <= min) {
            v = wrap ? max : min;
        } else {
            int64_t tmp = (int64_t)v - (int64_t)step;
            if (tmp < (int64_t)min) {
                v = wrap ? max : min;
            } else {
                v = (int32_t)tmp;
            }
        }
    }

    *data = v;
}

static void menu_value_adjust(const MenuValueBinding *b, bool inc)
{
    if (!b || !b->ptr) {
        return;
    }

    switch (b->kind) {
        case MENU_VAL_BOOL: {
            bool *p = (bool *)b->ptr;
            *p = !*p;
            break;
        }
        case MENU_VAL_U8: {
            uint8_t *p = (uint8_t *)b->ptr;
            u8_adjust(
                p,
                inc,
                b->u.u8.min,
                b->u.u8.max,
                b->u.u8.step,
                b->u.u8.wrap
            );
            break;
        }
        case MENU_VAL_I32: {
            int32_t *p = (int32_t *)b->ptr;
            i32_adjust(
                p,
                inc,
                b->u.i32.min,
                b->u.i32.max,
                b->u.i32.step,
                b->u.i32.wrap
            );
            break;
        }
        case MENU_VAL_ENUM: {
            uint8_t *p = (uint8_t *)b->ptr;
            u8_adjust(
                p,
                inc,
                0,
                b->u.enm.count-1,
                1,
                true
            );
            break;
        }
        default:
            break;
    }

    if (b->on_change) {
        b->on_change(b->ptr);
    }
}

static void menu_value_format(const MenuValueBinding *b, char *buf, size_t n)
{
    if (!b || !b->ptr || !buf || n == 0) {
        return;
    }

    switch (b->kind) {
        case MENU_VAL_BOOL: {
            bool v = *(bool *)b->ptr;
            sniprintf(buf, n, "%s", v ? "ON" : "OFF");
            break;
        }
        case MENU_VAL_U8: {
            uint8_t v = *(uint8_t *)b->ptr;
            sniprintf(buf, n, "%"PRIu8, v);
            break;
        }
        case MENU_VAL_I32: {
            int32_t v = *(int32_t *)b->ptr;
            sniprintf(buf, n, "%"PRIi32, v);
            break;
        }
        case MENU_VAL_ENUM: {
            char **names = (char **)b->u.enm.names;
            char *v = names[*((uint8_t*)b->ptr)];
            sniprintf(buf, n, "%s", v);
            break;
        }
        case MENU_VAL_STR: {
            char *v = (char *)b->ptr;
            sniprintf(buf, n, "%s", v);
            break;
        }
        default:
            sniprintf(buf, n, "?");
            break;
    }
}

typedef struct {
    const MenuItem *menu;   // pointer to current folder node
    uint8_t pos;            // selected index within menu->children
    uint8_t first;          // index of first visible child
} MenuFrame;

//TODO: Determine size at runtime, for now use a safe fixed bound
#define MENU_MAX_DEPTH 8

typedef struct {
    MenuFrame   stack[MENU_MAX_DEPTH];
    uint8_t     depth;
    bool        dirty;
    bool        edit;
} MenuState;

static void menu_tick(UiScreen *self, const UiEvent *ev);
static void menu_draw(UiScreen *self);

/* Global menu state + screen object */
static MenuState g_menu_state;

static UiScreen  g_menu_screen = {
    .tick = menu_tick,
    .draw = menu_draw,
    .ctx  = &g_menu_state,
};

/* Root menu defined in ui_menu_tree.c */
extern const MenuItem g_root_menu;

/* --- Internal helpers --- */
static void menu_reset_to_root(MenuState *st)
{
    st->depth = 1;

    st->stack[0].menu  = &g_root_menu;
    st->stack[0].pos   = 0;
    st->stack[0].first = 0;
    
    st->dirty = true;
    st->edit  = false;
}

/* Clamp first so that pos is always visible in [first, first + MENU_VISIBLE_ROWS - 1] */
static void menu_ensure_visible(MenuFrame *frame)
{
    if (frame->pos < frame->first) {
        frame->first = frame->pos;
    } else if (frame->pos >= (uint8_t)(frame->first + MENU_VISIBLE_ROWS)) {
        frame->first = frame->pos - (MENU_VISIBLE_ROWS - 1);
    }
}

static void menu_textedit_done(bool applied, void *user)
{
    MenuValueBinding *b = (MenuValueBinding *)user;
    if (applied && b && b->on_change) {
        b->on_change(b->ptr);
    }
}

/* --- Public API --- */
UiScreen *ui_get_menu_screen(void)
{
    return &g_menu_screen;
}

void ui_menu_open_root(void)
{
    MenuState *st = &g_menu_state;
    menu_reset_to_root(st);
    ui_push_screen(&g_menu_screen);
}

/* --- UiScreen implementation --- */

static void menu_tick(UiScreen *self, const UiEvent *ev)
{
    MenuState *st = (MenuState *)self->ctx;

    if (!st) {
        return;
    }

    /* If somehow unitialized, reset to root */
    if (st->depth == 0) {
        menu_reset_to_root(st);
    }

    if (!ev) {
        return;
    }

    switch (ev->type) {
    case UI_EVENT_FOCUS_GAIN:
        st->dirty = true;
        return;

    case UI_EVENT_FOCUS_LOST:
        // Nothing special for now
        return;
    case UI_EVENT_KEY:
        break; // handle below
    default:
        return;
    }

    // It's a key event
    enum key key = ev->key;

    MenuFrame *frame = &st->stack[st->depth - 1];
    const MenuItem *menu = frame->menu;

    if (!menu || menu->child_count == 0 || !menu->children) {
        /* Empty folder: Back should just pop, anything else ignored */
        if (key == KEY_ESC) {
            if (st->depth > 1) {
                st->depth--;
                st->dirty = true;
            } else {
                ui_pop_screen();
            }
        }
        return;
    }

    /* Handle keypress */
    if (!st->edit) {
        /* --- NORMAL NAVIGATION MODE --- */
        switch (key)
        {
        case KEY_UP:
            if (frame->pos > 0) {
                frame->pos--;
            } else {
                frame->pos = menu->child_count - 1; // wrap to last
            }
            menu_ensure_visible(frame);
            st->dirty = true;
            break;

        case KEY_DOWN:
            if (frame->pos + 1 < menu->child_count) {
                frame->pos++;
            } else {
                frame->pos = 0; // wrap to first
            }
            menu_ensure_visible(frame);
            st->dirty = true;
            break;

        case KEY_ENTER: {
            const MenuItem *item = menu->children[frame->pos];

            /* Enter non-empty folder*/
            if (item->kind == MENU_NODE_FOLDER && item->child_count > 0 && item->children != NULL) {
                /* Descend into child folder */
                if (st->depth < MENU_MAX_DEPTH) {
                    MenuFrame *child = &st->stack[st->depth];
                    child->menu  = item;
                    child->pos   = 0;
                    child->first = 0;
                    st->depth++;
                    st->dirty = true;
                }
            }
            /* Activate edit mode on value node */
            else if (item->kind == MENU_NODE_VALUE && item->binding) {
                const MenuValueBinding *b = item->binding;

                if (b->kind == MENU_VAL_STR) {
                    /* Launch text editor screen instead of inline edit */
                    UiTextEditParams p = {
                        .buf       = (char *)b->ptr,
                        .max_len   = b->u.str.max_len,
                        .profile   = b->u.str.profile,
                        .title     = item->label,
                        .on_done   = menu_textedit_done,
                        .user      = (void *)b,
                    };
                    ui_open_textedit(&p);
                    return;
                }

                /* Non-string values; normal inline edit mode */
                st->edit  = true;
                st->dirty = true;
            }
            else if (item->kind == MENU_NODE_VALUE && item->cb) {
                item->cb(MENU_CMD_EDIT_BEGIN, NULL, item->cb_ctx);
                st->edit  = true;
                st->dirty = true;
            }
            else if (item->kind == MENU_NODE_ACTION && item->cb) {
                item->cb(MENU_CMD_SELECT, NULL, item->cb_ctx);
                st->dirty = true;
            }
            break;
        }
        
        case KEY_ESC:
            if (st->depth > 1) {
                /* Go up one folder */
                st->depth--;
                st->dirty = true;
            } else {
                /* At root: leave menu back to previous screen */
                ui_pop_screen();
            }
            break;
        
        default:
            break;
        }
    } else {
        /* ---------- EDIT MODE ----------*/
        const MenuItem *item = menu->children[frame->pos];
        
        if (item->binding) {
            /* Generic value */
            bool inc = (key == KEY_UP);
            bool dec = (key == KEY_DOWN);

            if ((item->kind == MENU_NODE_VALUE) && (inc || dec)) {
                menu_value_adjust(item->binding, inc);
                st->dirty = true;
                return;
            }
        }

        if (item->cb) {
            /* Let callback handle keys in edit mode */
            item->cb(MENU_CMD_EDIT_KEY, (void *)ev, item->cb_ctx);
            st->dirty = true;
        }

        if (key == KEY_ESC) {
            if (item->cb) {
                item->cb(MENU_CMD_EDIT_CANCEL, NULL, item->cb_ctx);
            }
            st->edit  = false;
            st->dirty = true;
        } else if (key == KEY_ENTER) {
            if (item->cb) {
                item->cb(MENU_CMD_EDIT_APPLY, NULL, item->cb_ctx);
            }
            st->edit  = false;
            st->dirty = true;
        }

        return;
    }
}

static void menu_draw(UiScreen *self)
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
    const point_t top_pos = {horizontal_pad, top_pos_y};
    const point_t line1_pos = {horizontal_pad, line1_pos_y};
    const fontSize_t top_font = FONT_SIZE_8PT;
    const fontSize_t menu_font = FONT_SIZE_8PT;
    const color_t color_white = {255, 255, 255, 255};
    const color_t color_black = {0, 0, 0, 255};

    MenuState *st = (MenuState *)self->ctx;

    if(!st || !st->dirty)
    {
        return;
    }

    if (st->depth == 0)
    {
        return;
    }

    MenuFrame *frame = &st->stack[st->depth - 1];
    const MenuItem *menu = frame->menu;

    if (!menu)
    {
        return;
    }

    gfx_clearScreen();
    
    // Header
    const char *title = menu->label ? menu->label : "";
    gfx_print(top_pos, top_font, TEXT_ALIGN_CENTER, color_white, title);

    // Menu items
    point_t pos   = line1_pos;
    int     first = frame->first;
    int     count = menu->child_count;

    for(int idx = first; idx < first + MENU_VISIBLE_ROWS && idx < count; ++idx)
    {
        const MenuItem *item = menu->children[idx];
        const char *label    = item->label ? item->label : "";
        
        color_t text_color = color_white;

        /* Highlight if selected */
        if (idx == frame->pos)
        {
            text_color = color_black;
            bool full_rect = true;
            /* If in edit mode, draw a hollow rectangle */
            if(st->edit)
            {
                text_color = color_white;
                full_rect = false;
            }
            point_t rect_pos = {0, pos.y - menu_h + 3};
            gfx_drawRect(rect_pos, CONFIG_SCREEN_WIDTH, menu_h, color_white, full_rect);
            // announceMenuItemIfNeeded()
        }
        gfx_print(pos, menu_font, TEXT_ALIGN_LEFT, text_color, label);

        char value_buf[16] = {0};
        /* Show if node is unimplemented */
        if (item->kind == MENU_NODE_UNIMPLEMENTED) {
            gfx_print(pos, menu_font, TEXT_ALIGN_RIGHT, text_color, ":(");
        }

        /* Value on the right if this is a VALUE node */
        else if (item->kind == MENU_NODE_VALUE) {
            /**
             * TODO: Evaluate value buffer size
             * - We don't want the printed value to take up too much room in the
             * screen, taking away from the item title.
             * - Variable length fonts make this tricky.
             * - Truncating could be misleading
             */

            if (item->binding) {
                /* Generic value binding */
                menu_value_format((const MenuValueBinding *)item->binding, value_buf, sizeof value_buf);
            } else if (item->cb) {
                /* Custom value: let the callback format it */
                MenuDrawValueArgs args = {
                    .buf     = value_buf,
                    .buf_len = sizeof value_buf
                };
                item->cb(MENU_CMD_DRAW_VALUE, &args, item->cb_ctx);
            }
        }

        /* Show value if MENU_NODE_ACTION supports it */
        else if (item->kind == MENU_NODE_ACTION && item->cb) {
            MenuDrawValueArgs args = {
                .buf     = value_buf,
                .buf_len = sizeof value_buf
            };
            item->cb(MENU_CMD_DRAW_VALUE, &args, item->cb_ctx);
        }

        if (value_buf[0] != '\0') {
            gfx_print(pos, menu_font, TEXT_ALIGN_RIGHT, text_color, value_buf);
        }

        pos.y += menu_h;
    }

    gfx_render();
    st->dirty = false;
}