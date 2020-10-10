#if !defined(__MII_H__)
#define __MII_H__

typedef struct s_mii_signals_tx 
{
   // b_en: pcs_txen / TX_EN
   bool                    b_en;
   // b_en: pcs_txer / TX_ER
   bool                    b_er;
   // d: pcs_txd / TXD<0:3>
   sym4b                   d;
} S_mii_signals_tx;

typedef struct s_mii_signals_rx 
{
   // b_dv: RX_DV / PLS_DATA_VALID.indication()
   bool                    b_dv;
   // b_er: RX_ER
   bool                    b_er;
   // b_crs: CRS / PLS_CARRIER.indication (CARRIER_STATUS)
   bool                    b_crs;
   // b_col: COL / PLS_SIGNAL.indication (SIGNAL_STATUS)
   // SIGNAL_STATUS:
   // - true: SIGNAL_ERROR
   // - false: NO_SIGNAL_ERROR
   bool                    b_col;
   // d: RXD / PLS_DATA.indication()
   sym4b                   d;
} S_mii_signals_rx;

#endif // if !defined(__MII_H__)
