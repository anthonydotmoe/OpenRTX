/* compatibility UiScreen implementation for legacy UI */
#include "core/graphics.h"
#include "core/state.h"
#include "core/ui.h"
#include "interfaces/keyboard.h"
#include "ui/ui_screen.h"
#include "ui/ui_menu.h"

typedef struct {
    bool sync_rtx; /* set true by FSM when radio config should be updated */
} CompatState;

static void compat_tick(UiScreen *self, const UiEvent *ev);
static void compat_draw(UiScreen *self);

/* Single global compat state + screen */
static CompatState g_compat_state = {
    .sync_rtx = true,   /* force one initial rtx_configure after boot */
};

static UiScreen g_compat_screen = {
    .tick = compat_tick,
    .draw = compat_draw,
    .ctx  = &g_compat_state,
};

/* Public accessors used by ui_threadFunc */

UiScreen *ui_get_compat_screen(void)
{
    return &g_compat_screen;
}

/* Returns a pointer that ui_threadFunc casts back to CompatState */
void *ui_get_compat_state(void)
{
    return &g_compat_state;
}

/* --- UIScreen implementation --- */

static void compat_tick(UiScreen *self, const UiEvent *ev)
{
    CompatState *st = (CompatState *)self->ctx;

    if (ev && ev->key == KEY_F3) {
        ui_menu_open_root();
        return;
    }

    /**
     * Old world:
     *   ui_updateFSM(&sync_rtx);
     * was called directly from ui_threadFunc.
     * 
     * New world:
     *   ui_threadFunc calls scr->tick(), and this wrapper calls the
     *   legacy FSM exactly as before.
     */
    ui_updateFSM(&st->sync_rtx);
}

/**
 * Drawing wrapper:
 *  - ui_threadFunc calls scr->draw(scr);
 *  - compat_draw uses the existing ui_updateGUI() and gfx_render()
 *    so nothing else in the old UI has to change yet.
 */
static void compat_draw(UiScreen *self) {
    (void)self;

    if (ui_updateGUI())
    {
        gfx_render();
    }
}
