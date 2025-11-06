#include <stdint.h>
#include <stdbool.h>
#include <string.h> // for NULL

#include "ui/ui_menu.h"
#include "ui/ui_screen.h"
#include "interfaces/keyboard.h" //TODO: For keycodes

/* menu_draw includes */
#include "hwconfig.h"
#include "core/graphics.h"

/* How many rows we plan to show at once; for scrolling logic */
#define MENU_VISIBLE_ROWS 6

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
    // TODO: more, e.g. edit mode?
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

    /* If somehow unitialized, reset to root */
    if (st->depth == 0) {
        menu_reset_to_root(st);
    }

    /* No event? nothing to do this tick */
    if (!ev) {
        return;
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

    switch (key) {
        case KEY_UP:
        {
            if (frame->pos > 0) {
                frame->pos--;
            } else {
                frame->pos = menu->child_count - 1; // wrap to last
            }
            menu_ensure_visible(frame);
            st->dirty = true;
            break;
        }

        case KEY_DOWN:
        {
            if (frame->pos + 1 < menu->child_count) {
                frame->pos++;
            } else {
                frame->pos = 0; // wrap to first
            }
            menu_ensure_visible(frame);
            st->dirty = true;
            break;
        }

        case KEY_ENTER:
        {
            const MenuItem *item = &menu->children[frame->pos];

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
            } else {
                /* Action or Value node: invoke callback if present */
                if (item->cb) {
                    (void)item->cb(MENU_CMD_SELECT, 0, item->user);
                }
            }
            break;
        }

        case KEY_ESC:
        {
            if (st->depth > 1) {
                /* Go up one folder */
                st->depth--;
                st->dirty = true;
            } else {
                /* At root: leave menu back to previous screen */
                ui_pop_screen();
            }
            break;
        }

        default:
        {
            /* Ignore other keys for now */
            break;
        }
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
        const MenuItem *item = &menu->children[idx];
        const char *label    = item->label ? item->label : "";
        
        color_t text_color = color_white;

        if (idx == frame->pos)
        {
            text_color = color_black;
            point_t rect_pos = {0, pos.y - menu_h + 3};
            gfx_drawRect(rect_pos, CONFIG_SCREEN_WIDTH, menu_h, color_white, true);
            // announceMenuItemIfNeeded()
        }
        gfx_print(pos, menu_font, TEXT_ALIGN_LEFT, text_color, label);
        pos.y += menu_h;
    }

    gfx_render();
    st->dirty = false;
}