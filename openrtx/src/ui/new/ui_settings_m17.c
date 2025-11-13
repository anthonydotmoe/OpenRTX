#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"

#include "core/settings.h"
#include "core/state.h"

static void on_change(void *ptr) {
    (void)ptr;
    //TODO: old UI says `*sync_rtx = true`,
    //      need to update the rtx thread
}

static MenuValueBinding callsign_binding = {
    .kind = MENU_VAL_STR,
    .ptr = &state.settings.callsign,
    .on_change = on_change,
    .u.str = {
        .max_len = sizeof(((settings_t *)0)->callsign) - 1,
        .profile = UI_STR_PROFILE_CALLSIGN,
    },
};

static const MenuItem m_callsign =
    MENU_ITEM_VALUE_BINDING("Callsign", &callsign_binding);

static MenuValueBinding can_binding = {
    .kind = MENU_VAL_U8,
    .ptr = &state.settings.m17_can,
    .on_change = on_change,
    .u.u8 = { .min = 0, .max = 15, .step = 1, .wrap = true },
};

static const MenuItem m_can =
    MENU_ITEM_VALUE_BINDING("CAN", &can_binding);

static MenuValueBinding can_rx_check_binding = {
    .kind = MENU_VAL_BOOL,
    .ptr = &state.settings.m17_can_rx,
    .on_change = on_change,
};

static const MenuItem m_can_rx_check =
    MENU_ITEM_VALUE_BINDING("CAN RX Check", &can_rx_check_binding);

static const MenuItem *const m17_children[] = {
    &m_callsign,
    &m_can,
    &m_can_rx_check,
};

const MenuItem g_m17_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("M17", m17_children);
