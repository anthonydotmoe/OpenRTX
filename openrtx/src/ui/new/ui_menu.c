#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h> // for NULL
#include <inttypes.h>

#include "ui/ui_menu.h"
#include "ui/ui_screen.h"
#include "interfaces/keyboard.h" //TODO: For keycodes

/* menu_draw includes */
#include "hwconfig.h"
#include "core/graphics.h"

/* How many rows we plan to show at once; for scrolling logic */
#define MENU_VISIBLE_ROWS 6

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
            uint8_t v = *p;

            uint8_t min  = b->u.u8.min;
            uint8_t max  = b->u.u8.max;
            uint8_t step = b->u.u8.step;
            bool    wrap = b->u.u8.wrap;

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
            
            *p = v;
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
            sniprintf(buf, n, "%s", v ? "On" : "Off");
            break;
        }
        case MENU_VAL_I32: {
            int32_t v = *(int32_t *)b->ptr;
            sniprintf(buf, n, "%"PRIi32, v);
            break;
        }
        case MENU_VAL_U8: {
            uint8_t v = *(uint8_t *)b->ptr;
            sniprintf(buf, n, "%"PRIu8, v);
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

    if (!st || !ev) {
        return;
    }

    /* If somehow unitialized, reset to root */
    if (st->depth == 0) {
        menu_reset_to_root(st);
    }

    /* For now we treat all events as key events */
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
                st->edit  = true;
                st->dirty = true;
            }
            else if (item->kind == MENU_NODE_VALUE && item->cb) {
                item->cb(MENU_CMD_EDIT_BEGIN, NULL, item->cb_ctx);
                st->edit  = true;
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

        /* Value on the right if this is a VALUE node */
        if (item->kind == MENU_NODE_VALUE) {
            // TODO: Evaluate value buffer size
            char buf[16] = {0};

            if (item->binding) {
                /* Generic value binding */
                menu_value_format((const MenuValueBinding *)item->binding, buf, sizeof buf);
            } else if (item->cb) {
                /* Custom value: let the callback format it */
                MenuDrawValueArgs args = {
                    .buf     = buf,
                    .buf_len = sizeof buf
                };
                item->cb(MENU_CMD_DRAW_VALUE, &args, item->cb_ctx);
            }
            if (buf[0] != '\0') {
                gfx_print(pos, menu_font, TEXT_ALIGN_RIGHT, text_color, buf);
            }
        }

        /* Show if node is unimplemented */
        if (item->kind == MENU_NODE_UNIMPLEMENTED) {
            gfx_print(pos, menu_font, TEXT_ALIGN_RIGHT, text_color, ":(");
        }
        pos.y += menu_h;
    }

    gfx_render();
    st->dirty = false;
}