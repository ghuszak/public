#if !defined(__PLCA_CTRL_H__)
#define __PLCA_CTRL_H__

typedef enum e_plca_ctrl_state 
{
   _PCS_UNDEFINED = 0u, 

   PCS_DISABLE, 
   PCS_RESYNC, 
   PCS_RECOVER, 
   PCS_SEND_BEACON, 
   PCS_SYNCING, 
   PCS_WAIT_TO, 
   PCS_EARLY_RECEIVE, 
   PCS_COMMIT, 
   PCS_YIELD, 
   PCS_RECEIVE, 
   PCS_TRANSMIT, 
   PCS_BURST, 
   PCS_NEXT_TX_OPPORTUNITY, 
   PCS_ABORT, 

   _PCS_MIN = PCS_DISABLE, 
   _PCS_MAX = PCS_ABORT, 
   _PCS_CNT = ((_PCS_MAX- -_PCS_MIN) + 1u)
} E_plca_ctrl_state;

typedef struct s_plca_ctrl_state 
{
   E_plca_ctrl_state       state;
   bool                    b_reset_prev;

   // cur_id: curID
   uint                    cur_id;
   // bc: bc
   uint8                   bc;
} S_plca_ctrl_state;

void INIT_plca_ctrl(uint pid);
void CLK_plca_ctrl(uint pid, bool b_mcd, bool b_pmcd);

#endif // if !defined(__PLCA_CTRL_H__)
