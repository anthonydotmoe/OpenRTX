#include <stdint.h>
#include <string.h>

#include "ui/ui_menu.h"

/* include menu items here, gate with compile flags */

static const MenuItem display_children[] = {
    { MENU_NODE_FOLDER, "Brightness",   0, NULL, NULL, NULL },
    { MENU_NODE_FOLDER, "Timer",        0, NULL, NULL, NULL },
    { MENU_NODE_FOLDER, "Battery Icon", 0, NULL, NULL, NULL },
};

static const MenuItem display_settings_menu = {
    .kind           = MENU_NODE_FOLDER,
    .label          = "Display",
    .child_count    = sizeof(display_children)/sizeof(display_children[0]),
    .children       = display_children,
    .cb             = NULL,
    .user           = NULL,
};

static const MenuItem settings_children[] = {
    display_settings_menu,
};

static const MenuItem root_menu_children[] = {
    { MENU_NODE_FOLDER, "Banks",    0, NULL, NULL, NULL },
    { MENU_NODE_FOLDER, "Channels", 0, NULL, NULL, NULL },
    { MENU_NODE_FOLDER, "Contacts", 0, NULL, NULL, NULL },
    { MENU_NODE_FOLDER, "GPS",      0, NULL, NULL, NULL },
    { MENU_NODE_FOLDER, "Settings",
        .child_count = sizeof(settings_children)/sizeof(settings_children[0]),
        .children = settings_children,
        .cb = NULL,
        .user = NULL,
    },
    { MENU_NODE_FOLDER, "Info",     0, NULL, NULL, NULL },
    { MENU_NODE_FOLDER, "About",    0, NULL, NULL, NULL },
};

const MenuItem g_root_menu = {
    .kind           = MENU_NODE_FOLDER,
    .label          = "Menu",
    .child_count    = sizeof(root_menu_children)/sizeof(display_children[0]),
    .children       = root_menu_children,
    .cb             = NULL,
    .user           = NULL,
};