#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"

#include "core/state.h"

static const MenuItem m_fm_ctcss_tone =
    MENU_ITEM_UNIMPLEMENTED("CTCSS Tone");

typedef enum {
    CTC_EN_NONE = 0,
    CTC_EN_BOTH,
    CTC_EN_ENCODE,
    CTC_EN_DECODE,
} CtcEnMode;

static CtcEnMode fm_ctcss_mode_from_state()
{
    bool tx = state.channel.fm.txToneEn;
    bool rx = state.channel.fm.rxToneEn;

    if (!tx && !rx) return CTC_EN_NONE;
    if (tx && rx)   return CTC_EN_BOTH;
    if (tx && !rx)  return CTC_EN_ENCODE;
    if (!tx && rx)  return CTC_EN_DECODE;

    return CTC_EN_NONE;
}

static void fm_ctcss_mode_to_state(CtcEnMode m)
{
    switch (m) {
    case CTC_EN_NONE:
        state.channel.fm.txToneEn = false;
        state.channel.fm.rxToneEn = false;
        break;
    case CTC_EN_BOTH:
        state.channel.fm.txToneEn = true;
        state.channel.fm.rxToneEn = true;
        break;
    case CTC_EN_ENCODE:
        state.channel.fm.txToneEn = true;
        state.channel.fm.rxToneEn = false;
        break;
    case CTC_EN_DECODE:
        state.channel.fm.txToneEn = false;
        state.channel.fm.rxToneEn = true;
        break;
    }

    //TODO: old UI says `*sync_rtx = true`,
    //      need to update the rtx thread
}

static int fm_ctcss_en_cb(MenuCmd cmd, void *arg, void *cb_ctx)
{
    (void)cb_ctx;

    switch (cmd) {
        case MENU_CMD_DRAW_VALUE: {
            MenuDrawValueArgs *a = (MenuDrawValueArgs *)arg;
            CtcEnMode m = fm_ctcss_mode_from_state();
            const char *txt = "ERR";

            switch (m) {
            case CTC_EN_NONE:   txt = "None";   break;
            case CTC_EN_BOTH:   txt = "Both";   break;
            case CTC_EN_ENCODE: txt = "Encode"; break;
            case CTC_EN_DECODE: txt = "Decode"; break;
            }

            sniprintf(a->buf, a->buf_len, "%s", txt);
            return 1;
        }

        case MENU_CMD_EDIT_BEGIN:
            // Nothing to do
            return 0;
        
        case MENU_CMD_EDIT_KEY: {
            const UiEvent *ev = (const UiEvent *)arg;
            CtcEnMode m = fm_ctcss_mode_from_state();

            if (!ev) return 0;

            if (ev->key == KEY_UP || ev->key == KEY_DOWN) {
                bool inc = ev->key == KEY_UP;
                m = (CtcEnMode)((m + (inc ? 1 : -1)) % 4);
                fm_ctcss_mode_to_state(m);
                return 1;
            }
            return 0;
        }

        case MENU_CMD_EDIT_CANCEL:
            // Optional: re-sync from some saved copy or just leave it
            return 0;
        
        case MENU_CMD_EDIT_APPLY:
            // Nothing to do, we already wrote into state
            return 0;
        
        case MENU_CMD_SELECT:
            /* TODO: Reconsider this one, doesn't make much sense. */
            return 0;
    }

    return 0;
}

static const MenuItem m_fm_ctcss_en =
    MENU_ITEM_VALUE_CB("CTCSS En.", fm_ctcss_en_cb, NULL);

static const MenuItem *const fm_children[] = {
    &m_fm_ctcss_tone,
    &m_fm_ctcss_en,
};

const MenuItem g_fm_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("FM", fm_children);
