#if !defined(__PMA_RX_H__)
#define __PMA_RX_H__

// Levels:
// 0/undefined: base
// 1: higher
// Note: DEBUG_PMA_RX needs to be defined first
#define DEBUG_PMA_RX_LEVEL 1u

typedef struct s_pma_rx_state 
{
   E_pma_state             ch_prev[2];
   uint                    prev_len;

   bool                    b_processing;
   bool                    b_col_prev;
   // b_data_edge:
   // - false (first edge): clock edge
   // - true (second edge): data edge
   bool                    b_data_edge;

   uint                    rx_bit_cnt;
   sym5b                   rx_sym;
} S_pma_rx_state;

void INIT_pma_rx(uint pid);
void CLK_pma_rx(uint pid);

#endif // if !defined(__PMA_RX_H__)
