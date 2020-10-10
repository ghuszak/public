#if !defined(__PCS_RX_H__)
#define __PCS_RX_H__

// Levels:
// 0/undefined: base
// 1: higher
// Note: DEBUG_PCS_RX needs to be defined first
#define DEBUG_PCS_RX_LEVEL 1u

// See "147.3.3.4 Function"
#define DECODE             conv_5b_to_4b

typedef enum e_pcs_rx_state 
{
   _PCRS_UNDEFINED = 0u, 

   PCRS_WAIT_SYNC, 
   PCRS_SYNCING, 
   PCRS_COMMIT, 
   PCRS_WAIT_SSD, 
   PCRS_BAD_SSD, 
   PCRS_PRE, 
   PCRS_DATA, 
   PCRS_BAD_ESD, 
   PCRS_GOOD_ESD, 
   /*PCRS_HEARTBEAT1, 
   PCRS_HEARTBEAT2, */
   PCRS_BEACON1, 
   PCRS_BEACON2, 

   _PCRS_MIN = PCRS_WAIT_SYNC, 
   _PCRS_MAX = PCRS_BEACON2, 
   _PCRS_CNT = ((_PCRS_MAX - _PCRS_MIN) + 1u)
} E_pcs_rx_state;

/*typedef enum e_pcs_rx_cmd 
{
   PRC_NONE = 0u, 
   //PRC_HEARTBEAT, 
   PRC_BEACON, 
   PRC_COMMIT, 

   _PRC_MIN = PRC_NONE, 
   _PRC_MAX = PRC_COMMIT, 
   _PRC_CNT = ((_PRC_MAX - _PRC_MIN) + 1u)
} E_pcs_rx_cmd;*/

typedef struct s_pcs_rx_state 
{
   E_pcs_rx_state          state;
   bool                    b_reset_prev;

   // b_pmudi, pmudi_sym: for PMA_UNITDATA.indication (rx_sym)
   bool                    b_pmudi;
   sym5b                   pmudi_sym;

   // rxn: RXn[5]
   sym5b                   rxn[5];
   uint                    precnt;

   // mii_rx: pcs_rxdv (RX_DV), pcs_rxer (RX_ER), pcs_rxd (RXD<0:3>)
   S_mii_signals_rx        mii_rx;

   //E_pcs_rx_cmd            rx_cmd;
} S_pcs_rx_state;

void INIT_pcs_rx(uint pid);
void CLK_pcs_rx(uint pid);
void PMA_UNITDATA_indication(uint pid, sym5b rx_sym);

#endif // if !defined(__PCS_RX_H__)
