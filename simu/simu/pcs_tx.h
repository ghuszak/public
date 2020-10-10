#if !defined(__PCS_TX_H__)
#define __PCS_TX_H__

// See "147.3.2.4 Function"
#define ENCODE             conv_4b_to_5b

typedef enum e_pcs_tx_state 
{
   _PCTS_UNDEFINED = 0u, 

   PCTS_SILENT, 
   PCTS_COMMIT, 
   PCTS_SYNC1, 
   PCTS_SYNC2, 
   PCTS_SSD1, 
   PCTS_SSD2, 
   PCTS_DATA, 
   PCTS_ESD, 
   PCTS_BAD_ESD, 
   PCTS_GOOD_ESD, 
   PCTS_UNJAB_WAIT, 

   _PCTS_MIN = PCTS_SILENT, 
   _PCTS_MAX = PCTS_UNJAB_WAIT, 
   _PCTS_CNT = ((_PCTS_MAX - _PCTS_MIN) + 1u)
} E_pcs_tx_state;

typedef struct s_pcs_tx_state 
{
   E_pcs_tx_state          state;
   bool                    b_reset_prev;

   // E_s5bs
   sym5b                   tx_sym;
   // b_err: err
   bool                    b_err;

   // TODO remove (FEC)
   uint                    i;
} S_pcs_tx_state;

void INIT_pcs_tx(uint pid);
void CLK_pcs_tx(uint pid);

#endif // if !defined(__PCS_TX_H__)
