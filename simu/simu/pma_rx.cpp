#include "states.h"

void l_rx(uint pid, S_pma_rx_state *p_curr, bool b_one)
{
   uint  t_cnt = p_curr->rx_bit_cnt;
   sym5b t_sym = p_curr->rx_sym;

   ASSERT((t_cnt < 5u), pid);

   if (t_cnt < 5u)
   {
      if (t_cnt == 0u)
      {
         t_sym = 0u;
      }

#if (defined(DEBUG_PMA_RX) && DEBUG_PMA_RX_LEVEL >= 1u)
      print(DEBUG_PMA_RX, pid, DEBUG_PMA_RX_P, "PMA_RX: DMC_%s", (b_one ? "1" : "0"));
#endif // if (defined(DEBUG_PMA_RX) && DEBUG_PMA_RX_LEVEL >= 1u)

      if (b_one)
      {
         t_sym |= (1u << t_cnt);
      }

      if ((++t_cnt) == 5u)
      {
#if defined(DEBUG_PMA_RX)
         print(DEBUG_PMA_RX, pid, DEBUG_PMA_RX_P, "PMA_RX: UNITDATA.indication(%u/%s)", 
               t_sym, get_name_5b(pid, t_sym));
#endif // if defined(DEBUG_PMA_RX)

         PMA_UNITDATA_indication(pid, t_sym);

         p_curr->rx_bit_cnt = 0u;
      }
      else
      {
         p_curr->rx_bit_cnt = t_cnt;
         p_curr->rx_sym = t_sym;
      }
   }
}

void INIT_pma_rx(uint pid)
{
   S_pma_rx_state   *p_curr = ADDR_OF(g_states.stations[pid].pma_rx);
   bool             b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void CLK_pma_rx(uint pid)
{
   S_station_states *p_states = ADDR_OF(g_states.stations[pid]);
   S_mii_signals_rx *p_mii = ADDR_OF(p_states->pcs_rx.mii_rx);
   S_pma_rx_state   *p_curr = ADDR_OF(p_states->pma_rx);
   bool             b_ok = (pid < PHY_CNT);
   bool             b_cs, b_col, b_equ;
   E_pma_state      ch;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      if (p_states->pma.b_reset)
      {
         memset(p_curr, 0u, SIZE_OF(*p_curr));
      }
      else
      {
         ch = channel_read(pid, ADDR_OF(b_col));

         if ((p_mii->b_col = b_col))
         {
            b_cs = true;
            p_curr->prev_len = 0u;
            p_curr->b_processing = false;
         }
         else
         {
            b_cs = (ch != PMS_Z);

            if (p_curr->prev_len >= 2u)
            {
               b_equ = (ch == p_curr->ch_prev[0]);

               if (!p_curr->b_processing && 
                     b_equ && 
                     ch == p_curr->ch_prev[1])
               {
                  p_curr->b_processing = true;

                  if (b_cs)
                  {
                     p_curr->b_data_edge = false;
                     p_curr->rx_bit_cnt = 0u;
                  }
               }

               if (p_curr->b_processing)
               {
                  if (b_cs)
                  {
                     if (!(p_curr->b_data_edge = (!p_curr->b_data_edge)))
                     {
                        l_rx(pid, p_curr, (!b_equ));
                     }
                  }
                  else
                  {
                     p_curr->b_data_edge = false;
                     p_curr->rx_bit_cnt = 0u;
                  }
               }
            }
         }

         p_curr->ch_prev[1] = p_curr->ch_prev[0];
         p_curr->ch_prev[0] = ch;

         if (p_curr->prev_len < 2u)
         {
            p_curr->prev_len++;
         }

         if (p_mii->b_crs != b_cs)
         {
            p_mii->b_crs = b_cs;

#if defined(DEBUG_PMA_RX)
            print(DEBUG_PMA_RX, pid, DEBUG_PMA_RX_P, 
                  "PMA_RX: CARRIER.indication(CARRIER_%s)", (b_cs ? "ON" : "OFF"));
#endif // if defined(DEBUG_PMA_RX)
         }

         if (p_curr->b_col_prev != b_col)
         {
            p_curr->b_col_prev = b_col;

#if (defined(DEBUG_PMA_RX) && DEBUG_PMA_RX_LEVEL >= 1u)
            print(DEBUG_PMA_RX, pid, DEBUG_PMA_RX_P, "PMA_RX: COL_%s", (b_col ? "1" : "0"));
#endif // if (defined(DEBUG_PMA_RX) && DEBUG_PMA_RX_LEVEL >= 1u)
         }
      }
   }
}
