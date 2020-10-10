#if !defined(__PLCA_DATA_H__)
#define __PLCA_DATA_H__

// Levels:
// 0/undefined: base
// 1: higher
// Note: DEBUG_PLCA_DATA needs to be defined first
#define DEBUG_PLCA_DATA_LEVEL       0u

#define PLCA_DATA_DELAY_LINE_LEN    99u

typedef enum e_plca_data_state 
{
   _PDS_UNDEFINED = 0u, 

   PDS_NORMAL, 
   PDS_WAIT_IDLE, 
   PDS_IDLE, 
   PDS_RECEIVE, 
   PDS_HOLD, 
   PDS_ABORT, 
   PDS_COLLIDE, 
   PDS_DELAY_PENDING, 
   PDS_PENDING, 
   PDS_WAIT_MAC, 
   PDS_TRANSMIT, 
   PDS_FLUSH, 

   _PDS_MIN = PDS_NORMAL,
   _PDS_MAX = PDS_FLUSH,
   _PDS_CNT = ((_PDS_MAX- -_PDS_MIN) + 1u)
} E_plca_data_state;

typedef struct s_plca_data_state 
{
   E_plca_data_state                state;
   bool                             b_reset_prev;

   sym4b                            delay_line[PLCA_DATA_DELAY_LINE_LEN];
   uint                             n;
   uint                             a;
   uint                             b;

   E_plca_rtx_cmd                   tx_cmd_sync;
} S_plca_data_state;

void INIT_plca_data(uint pid);
void CLK_plca_data(uint pid, bool b_mcd);

#endif // if !defined(__PLCA_DATA_H__)
