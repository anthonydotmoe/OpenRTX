#include "ui/ui_menu.h"
#include "ui/ui_menu_dsl.h"

static const MenuItem m_mic_gain   = MENU_ITEM_UNIMPLEMENTED("Mic Gain");
static const MenuItem m_ptt_in     = MENU_ITEM_UNIMPLEMENTED("PTT In");
static const MenuItem m_ptt_out    = MENU_ITEM_UNIMPLEMENTED("PTT Out");
static const MenuItem m_tx_phase   = MENU_ITEM_UNIMPLEMENTED("TX Phase");
static const MenuItem m_rx_phase   = MENU_ITEM_UNIMPLEMENTED("RX Phase");
static const MenuItem m_tx_softpot = MENU_ITEM_UNIMPLEMENTED("TX Softpot");
static const MenuItem m_rx_softpot = MENU_ITEM_UNIMPLEMENTED("RX Softpot");

static const MenuItem *const module17_children[] = {
    &m_mic_gain,
    &m_ptt_in,
    &m_ptt_out,
    &m_tx_phase,
    &m_rx_phase,
    &m_tx_softpot,
    &m_rx_softpot,
};

const MenuItem g_module17_settings_menu =
    MENU_FOLDER_FROM_CHILDREN("Module 17", module17_children);
