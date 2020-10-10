#if !defined(__PMA_TX_H__)
#define __PMA_TX_H__

// Levels:
// 0/undefined: base
// 1: higher
// Note: DEBUG_PMA_TX needs to be defined first
//#define DEBUG_PMA_TX_LEVEL 1u

typedef struct s_pma_tx_state 
{
   // PMA_UNITDATA_request()
   sym5b                   tx_sym;
   bool                    b_new;

#if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL < 1u)
   sym5b                   tx_sym_prev;
#endif // if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL < 1u)

   bool                    b_dmc0;
   sym5b                   sym;
   uint                    idx;

   E_pma_state             bi_d;
} S_pma_tx_state;

void INIT_pma_tx(uint pid);
void CLK_pma_tx(uint pid);
void PMA_UNITDATA_request(uint pid, sym5b tx_sym);

#endif // if !defined(__PMA_TX_H__)
