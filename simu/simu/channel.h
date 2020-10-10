#if !defined(__CHANNEL_H__)
#define __CHANNEL_H__

typedef struct s_tx_drivers_states 
{
   // E_pma_state
   E_pma_state             bi_d[PHY_CNT];

   uint                    active_count;
   bool                    b_high;
} S_tx_drivers_states;

void INIT_channel(void);
// state: E_pma_state
void channel_write(uint pid, E_pma_state state);
E_pma_state channel_read(uint pid, bool *pb_col);

extern uint g_transmitters[PHY_CNT];

#endif // if !defined(__CHANNEL_H__)
