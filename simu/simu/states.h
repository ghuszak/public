#if !defined(__STATES_H__)
#define __STATES_H__

#include "base.h"

#define BR                    10000000u // 10Mbps
#define TX_TCLK               25000000u // 25MHz
#define PLCA_EN               1u
// PLCA_BURST_COUNT: it is meaningful only if PLCA_EN is enabled
#define PLCA_BURST_COUNT      0u // 0u: No burst
// TODO 2->10 (FEC)
#define PHY_CNT               2u

#if (PLCA_EN == 1u)
#define MII_TX                plca.mii_tx
#define MII_RX                plca.mii_rx
#else // if (PLCA_EN == 1u)
#define MII_TX                mac_pls.mii_tx
#define MII_RX                pcs_rx.mii_rx
#endif // else if (PLCA_EN == 1u)

// SUPPORT_TIMER_STD:
// - When defined, PCS_TX implementation follows the draft strictly
// - When undefined, STD is not implemented and it is considered 
//   to be in "done" state at every PCS_TX clock cycle
// Note: The frequency of STD and PCS_TX clock matches exactly 
// (both being 2.5MHz), thus STD can be considered 
// to be "done" at every PCS_TX clock, so not
// having it is an idempotent optimization
//#define SUPPORT_TIMER_STD

// SUPPORT_TIMER_RSCD:
// - When defined, PCS_RX implementation follows the draft strictly
// - When undefined, RSCD is not implemented and it is considered 
//   to be in "done" state at every PCS_RX clock cycle
// Note: The frequency of RSCD and PCS_RX clock matches exactly 
// (both being 2.5MHz), thus RSCD can be considered 
// to be "done" at every PCS_RX clock, so not
// having it is an idempotent optimization
//#define SUPPORT_TIMER_RSCD

// SUPPORT_SCRAMBLING:
// - When defined, scrambling is enabled in both directions
// - When undefined, scrambling is disabled and MII 
//   data appears on the channel unscrumbled
#define SUPPORT_SCRAMBLING
// SUPPORT_SCRAMBLING_OPTIMIZATION:
// - When defined, scrambling algorithm is optimized (4B symbol-based)
// - When undefined, scrambling algorithm is optimized (bit-based)
// Note: works only if SUPPORT_SCRAMBLING is defined
#define SUPPORT_SCRAMBLING_OPTIMIZATION

// SUPPORT_FC: fc_supported, whether the optional feature of 
// false carrier detection is supported by the PHY
#define SUPPORT_FC

// Change X_DEBUG to _DEBUG to enable full debugging in debug build
#if defined(X_DEBUG)
#define DEBUG_TIMER           stdout
#define DEBUG_TIMER_P         0u
#define DEBUG_CHANNEL_R       stdout
#define DEBUG_CHANNEL_R_P     0u
#define DEBUG_CHANNEL_W       stdout
#define DEBUG_CHANNEL_W_P     0u
#define DEBUG_PMA_TX          stdout
#define DEBUG_PMA_TX_P        0u
#define DEBUG_PMA_RX          stdout
#define DEBUG_PMA_RX_P        0u
#define DEBUG_PCS_TX          stdout
#define DEBUG_PCS_TX_P        0u
#define DEBUG_PCS_RX          stdout
#define DEBUG_PCS_RX_P        0u
#define DEBUG_SCRAMBLING      stdout
#define DEBUG_SCRAMBLING_P    0u
#define DEBUG_PLCA_CTRL       stdout
#define DEBUG_PLCA_CTRL_P     0u
#define DEBUG_PLCA_DATA       stdout
#define DEBUG_PLCA_DATA_P     0u
#define DEBUG_PLCA_STATUS     stdout
#define DEBUG_PLCA_STATUS_P   0u
#else // if defined(_DEBUG)
//#define DEBUG_TIMER           stdout
//#define DEBUG_TIMER_P         0u
//#define DEBUG_CHANNEL_R       stdout
//#define DEBUG_CHANNEL_R_P     0u
//#define DEBUG_CHANNEL_W       stdout
//#define DEBUG_CHANNEL_W_P     0u
//#define DEBUG_PMA_TX          stdout
//#define DEBUG_PMA_TX_P        0u
//#define DEBUG_PMA_RX          stdout
//#define DEBUG_PMA_RX_P        0u
//#define DEBUG_PCS_TX          stdout
//#define DEBUG_PCS_TX_P        0u
//#define DEBUG_PCS_RX          stdout
//#define DEBUG_PCS_RX_P        0u
//#define DEBUG_SCRAMBLING      stdout
//#define DEBUG_SCRAMBLING_P    0u
//#define DEBUG_PLCA_CTRL       stdout
//#define DEBUG_PLCA_CTRL_P     0u
//#define DEBUG_PLCA_DATA       stdout
//#define DEBUG_PLCA_DATA_P     0u
//#define DEBUG_PLCA_STATUS     stdout
//#define DEBUG_PLCA_STATUS_P   0u
#endif // else if defined(_DEBUG)

#define OUTPUT_LINUX_LIKE     1u

#if (!(PHY_CNT >= 2u))
#error "PHY_CNT is undefined or invalid"
#endif // if (!(PHY_CNT >= 2u))

#include "main.h"

#include "random.h"
#include "4b5b.h"

#include "pma.h"
#include "pcs.h"
#include "mii.h"
#include "plca.h"
#include "timer.h"
#include "pma_tx.h"
#include "pma_rx.h"
#include "pcs_tx.h"
#include "pcs_rx.h"
#include "mac_pls.h"
#include "channel.h"
#include "mac_simu.h"
#include "plca_ctrl.h"
#include "plca_data.h"
#include "plca_status.h"

#if defined(SUPPORT_SCRAMBLING)
#include "scrambling.h"
#endif // if defined(SUPPORT_SCRAMBLING)

typedef struct s_station_states 
{
   // true: ENABLE
   // false: DISABLE
   bool                       link_control;
   uint32                     canary_1;

   S_timer_state              timers[_TID_MAX_CNT];
   uint32                     canary_2;

   S_pma_state                pma;
   uint32                     canary_3;
   S_pma_tx_state             pma_tx;
   uint32                     canary_4;
   S_pma_rx_state             pma_rx;
   uint32                     canary_5;

   S_pcs_state                pcs;
   uint32                     canary_6;
#if defined(SUPPORT_SCRAMBLING)
   S_scrambling_state         scrambling;
#endif // if defined(SUPPORT_SCRAMBLING)
   uint32                     canary_7;
   S_pcs_tx_state             pcs_tx;
   uint32                     canary_8;
   S_pcs_rx_state             pcs_rx;
   uint32                     canary_9;

   S_plca_state               plca;
   uint32                     canary_10;
   S_plca_ctrl_state          plca_ctrl;
   uint32                     canary_11;
   S_plca_data_state          plca_data;
   uint32                     canary_12;
   S_plca_status_state        plca_status;
   uint32                     canary_13;

   S_mac_pls_states           mac_pls;
   uint32                     canary_14;
} S_station_states;

typedef struct s_states 
{
   S_station_states           stations[PHY_CNT];

   S_tx_drivers_states        tx_drivers;
} S_states;

void INIT_states(void);
bool CHECK_states(void);

extern S_states g_states;

#endif // if !defined(__STATES_H__)
