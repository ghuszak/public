#if !defined(__PCS_H__)
#define __PCS_H__

typedef enum e_pcs_tx_cmd 
{
   PTC_SILENCE = 0u, 
   PTC_BEACON, 
   PTC_COMMIT, 

   _PTC_MIN = PTC_SILENCE, 
   _PTC_MAX = PTC_COMMIT, 
   _PTC_CNT = ((_PTC_MAX - _PTC_MIN) + 1u)
} E_pcs_tx_cmd;

typedef struct s_pcs_state 
{
   // b_reset: pcs_reset
   // true: ON
   // false: OFF
   bool                    b_reset;

   // b_transmitting: transmitting
   bool                    b_transmitting;
} S_pcs_state;

void INIT_pcs(uint pid);
void RESET_pcs(uint pid, bool b_assert);

#endif // if !defined(__PCS_H__)
