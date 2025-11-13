#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"

/*
static void on_change(void *ptr) {
    (void)ptr;
    //TODO: old UI says `*sync_rtx = true`,
    //      need to update the rtx thread
}
*/

static const MenuItem m_offset =
    MENU_ITEM_UNIMPLEMENTED("Offset");

static const MenuItem m_direction =
    MENU_ITEM_UNIMPLEMENTED("Direction");

static const MenuItem m_step =
    MENU_ITEM_UNIMPLEMENTED("Step");

static const MenuItem *const radio_children[] = {
    &m_offset,
    &m_direction,
    &m_step,
};

const MenuItem g_radio_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("Radio", radio_children);
