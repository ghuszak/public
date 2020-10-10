#if !defined(__PLCA_H__)
#define __PLCA_H__

typedef enum e_plca_rtx_cmd 
{
   PRTC_NONE = 0u, 

   PRTC_BEACON, 
   PRTC_COMMIT, 

   _PRTC_MIN = PRTC_NONE, 
   _PRTC_MAX = PRTC_COMMIT, 
   _PRTC_CNT = ((_PRTC_MAX - _PRTC_MIN) + 1u)
} E_plca_rtx_cmd;

typedef struct s_plca_state 
{
   // max_bc: max_bc
   uint8                   max_bc;
   // b_reset: plca_reset
   bool                    b_reset;
   // node_id: local_nodeID
   uint8                   node_id;
   // b_active: plca_active
   bool                    b_active;
   // b_packet_pending: packetPending
   bool                    b_packet_pending;
   // b_committed: committed
   bool                    b_committed;

   // mii_tx: plca_txen, plca_txer, plca_txd<3:0>
   // Note: plca_txer is no specified/implemented
   S_mii_signals_tx        mii_tx;
   E_plca_rtx_cmd          tx_cmd;

   // mii_rx: , ,CARRIER_STATUS, , 
   // b_dv: PLS_DATA_VALID.indication (DATA_VALID_STATUS)
   // - true: DATA_VALID
   // - false: DATA_NOT_VALID
   // b_er: RX_ER
   // b_crs: PLS_CARRIER.indication (CARRIER_STATUS)
   // - true: CARRIER_ON
   // - false: CARRIER_OFF
   // b_col: PLS_SIGNAL.indication (SIGNAL_STATUS)
   // - true: SIGNAL_ERROR
   // - false: NO_SIGNAL_ERROR
   // d: PLS_DATA.indication (INPUT_UNIT)
   S_mii_signals_rx        mii_rx;
   // b_receiving: receiving
   bool                    b_receiving;
   E_plca_rtx_cmd          rx_cmd;
} S_plca_state;

void INIT_plca(uint pid, uint8 max_bc);
void RESET_plca(uint pid, bool b_assert);
void CLK_plca(uint pid);

#endif // if !defined(__PLCA_H__)
