#ifndef UI_MENU_DSL_H
#define UI_MENU_DSL_H

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

/// @brief Simple value item driven entirely by a MenuValueBinding
#define MENU_ITEM_VALUE_BINDING(_label, _binding_ptr) \
    {                                                 \
        .kind        = MENU_NODE_VALUE,               \
        .label       = (_label),                      \
        .child_count = 0,                             \
        .children    = NULL,                          \
        .binding     = (_binding_ptr),                \
        .cb          = NULL,                          \
        .cb_ctx      = NULL,                          \
    }

/// @brief Value item that is fully custom via callback
#define MENU_ITEM_VALUE_CB(_label, _cb, _ctx) \
    {                                         \
        .kind = MENU_NODE_VALUE,              \
        .label = (_label),                    \
        .child_count = 0,                     \
        .children = NULL,                     \
        .binding = NULL,                      \
        .cb = (_cb),                          \
        .cb_ctx = (_ctx),                     \
    }

/// @brief Folder whose children were declared as a separate array
#define MENU_FOLDER_FROM_CHILDREN(_label, _children_array) \
    {                                                      \
        .kind        = MENU_NODE_FOLDER,                   \
        .label       = (_label),                           \
        .child_count = ARRAY_LEN(_children_array),         \
        .children    = (_children_array),                  \
        .binding     = NULL,                               \
        .cb          = NULL,                               \
        .cb_ctx      = NULL,                               \
    }

/// @brief Placeholder unimplemented item
#define MENU_ITEM_UNIMPLEMENTED(_label)  \
    {                                    \
        .kind = MENU_NODE_UNIMPLEMENTED, \
        .label = (_label),               \
        .child_count = 0,                \
        .children = NULL,                \
        .binding = NULL,                 \
        .cb = NULL,                      \
        .cb_ctx = NULL,                  \
    }

#endif
