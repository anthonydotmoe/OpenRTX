#include <stdint.h>
#include <string.h>

#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"

/* include menu items here, gate with compile flags */

/* from ui_settings_display.c */
extern const MenuItem g_display_settings_menu;
extern const MenuItem g_gps_settings_menu;

static const MenuItem *const settings_children[] = {
    &g_display_settings_menu,
    &g_gps_settings_menu,
};

static const MenuItem m_banks    = { MENU_NODE_FOLDER, "Banks",    0, NULL, NULL, NULL };
static const MenuItem m_channels = { MENU_NODE_FOLDER, "Channels", 0, NULL, NULL, NULL };
static const MenuItem m_contacts = { MENU_NODE_FOLDER, "Contacts", 0, NULL, NULL, NULL };
static const MenuItem m_gps      = { MENU_NODE_FOLDER, "GPS",      0, NULL, NULL, NULL };
static const MenuItem m_settings = { MENU_NODE_FOLDER, "Settings",
    .child_count = ARRAY_LEN(settings_children),
    .children = settings_children,
    .cb = NULL,
    .user = NULL,
};
static const MenuItem m_info     = { MENU_NODE_FOLDER, "Info",     0, NULL, NULL, NULL };
static const MenuItem m_about    = { MENU_NODE_FOLDER, "About",    0, NULL, NULL, NULL };

static const MenuItem *const root_menu_children[] = {
    &m_banks,
    &m_channels,
    &m_contacts,
    &m_gps,
    &m_settings,
    &m_info,
    &m_about,
};

const MenuItem g_root_menu = {
    .kind           = MENU_NODE_FOLDER,
    .label          = "Menu",
    .child_count    = ARRAY_LEN(root_menu_children),
    .children       = root_menu_children,
    .cb             = NULL,
    .user           = NULL,
};