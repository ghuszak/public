#if !defined(__TIMER_H__)
#define __TIMER_H__

#define TIMER_CLK             TX_TCLK

// Levels:
// 0/undefined: base
// 1: higher
// 2: highest (not_done also printed)
// Note: DEBUG_TIMER needs to be defined first
#define DEBUG_TIMER_LEVEL     1u

typedef enum e_timer_id 
{
   _TID_UNDEFINED = 0u, 

   // xmit_max_timer
   TID_MAX_TIMER, 
   // unjab_timer
   TID_UNJAB, 
#if defined(SUPPORT_TIMER_STD)
   // STD: "5B Symbol Timer Done" (PCS_TX 5B symbol timer)
   TID_STD, 
#endif // if defined(SUPPORT_TIMER_STD)

#if defined(SUPPORT_TIMER_RSCD)
   // RSCD: "Receive Symbol Conversion Done" (PCS_RX 5B symbol timer)
   TID_RSCD, 
#endif // if defined(SUPPORT_TIMER_RSCD)

   // PLCA control beacon_timer
   TID_PLCAC_BEACON, 
   // PLCA control beacon_det_timer
   TID_PLCAC_BEACON_DET, 
   // PLCA control invalid_beacon_timer
   TID_PLCAC_INVALID_BEACON, 
   // PLCA control burst_timer
   TID_PLCAC_BURST, 
   // PLCA control to_timer
   TID_PLCAC_TO, 

   // PLCA data commit_timer
   TID_PLCAD_COMMIT, 
   // PLCA data pending_timer
   TID_PLCAD_PENDING, 

   // PLCA status plca_status_timer
   TID_PLCAS_PLCA_STATUS, 

   _TID_MAX_MIN = TID_MAX_TIMER, 
   _TID_MAX_MAX = TID_PLCAS_PLCA_STATUS, 
   _TID_MAX_CNT = ((_TID_MAX_MAX - _TID_MAX_MIN) + 1u)
} E_timer_id;

typedef struct s_timer_state 
{
   clock                   timeout;
   clock                   counter;
} S_timer_state;

bool timer_start(uint pid, uint tid, bool b_indone);
// This does not work as normal 802.3 timer stopping would:
// - this also cnacels the done state
// - 802.3 stopping keeps done state but prevents the timer from counting
// The reason for implementing it this way is the fact that, we are calling 
// timer_stop() only in reset state, so there is no risk of breaking anything
bool timer_stop(uint pid, uint tid);
bool timer_isdone(uint pid, uint tid);

void INIT_timer(uint pid);
void CLK_timer(uint pid);

#endif // if !defined(__TIMER_H__)
