#include "states.h"

#if defined(DEBUG_PMA_TX)
void l_dbg(uint pid, bool b_one, bool b_extra)
{
   print(DEBUG_PMA_TX, pid, DEBUG_PMA_TX_P, "PMA_TX: DMC_%s%s", 
         (b_one ? "1" : "0"), (b_extra ? "+" : ""));
}
#endif // if defined(DEBUG_PMA_TX)

void INIT_pma_tx(uint pid)
{
   S_pma_tx_state   *p_curr = ADDR_OF(g_states.stations[pid].pma_tx);
   bool             b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void CLK_pma_tx(uint pid)
{
   S_station_states *p_states = ADDR_OF(g_states.stations[pid]);
   S_pma_tx_state   *p_curr = ADDR_OF(p_states->pma_tx);
   bool             b_silence, b_clk, b_inv;
   bool             b_ok = (pid < PHY_CNT);
   E_pma_state      bi_d = PMS_Z;
   sym5b            t_sym;
   bool             b_set;
   uint             idx;
#if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL >= 1u)
   bool             b_one;
#endif // if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL >= 1u)

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      if (p_states->pma.b_reset)
      {
         memset(p_curr, 0u, SIZE_OF(*p_curr));

         p_curr->sym = S5B_SILENCE;
         p_curr->idx = (2u * 5u);
      }
      else
      {
         if (p_curr->b_new)
         {
            p_curr->b_new = false;
            t_sym = p_curr->tx_sym;

            // SILENCE may be left at any time
            ASSERT((p_curr->sym == S5B_SILENCE || p_curr->idx == 0u), pid);

            idx = (2u * 5u);

            // If the tx_sym parameter value is the special 5B 
            // symbol 'I', the PMA shall, in order:
            // a) Transmit an additional DME encoded 0, if the previous value 
            //    of the tx_sym parameter was anything but the 5B symbol 'I'
            if (t_sym == S5B_SILENCE && p_curr->sym != S5B_SILENCE)
            {
               idx += 2u;
            }

            p_curr->idx = idx;
            p_curr->sym = t_sym;
         }

         idx = p_curr->idx;

         ASSERT((idx != 0u), pid);

         if (idx != 0u)
         {
            b_set = true;

            if (idx > (2u * 5u))
            {
#if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL >= 1u)
               if (idx > ((2u * 5u) + 1u))
               {
                  l_dbg(pid, false, true);
               }
#endif // if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL >= 1u)

               b_inv = (((idx--) & 1u) == 0u);
            }
            else
            {
               t_sym = p_curr->sym;
               b_silence = (t_sym == S5B_SILENCE);

               if (b_silence)
               {
                  b_set = false;
               }
               else
               {
                  b_clk = (((--idx) & 1u) != 0u);

#if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL >= 1u)
                  b_one = ((t_sym & (0x10u >> (idx >> 1u))) != 0u);

                  if (b_clk)
                  {
                     l_dbg(pid, b_one, false);
                  }

                  b_inv = (b_clk || b_one);
#else // if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL >= 1u)
                  b_inv = (b_clk || ((t_sym & (0x10u >> (idx >> 1u))) != 0u));
#endif // else if (defined(DEBUG_PMA_TX) && DEBUG_PMA_TX_LEVEL >= 1u)
               }
            }

            if (b_set)
            {
               // Always start with PMS_1
               bi_d = ((b_inv != (p_curr->bi_d == PMS_1)) ? 
                       PMS_1 : PMS_0);
            }

            p_curr->idx = idx;
         }
      }

      channel_write(pid, (p_curr->bi_d = bi_d));
   }
}

void PMA_UNITDATA_request(uint pid, sym5b tx_sym)
{
   S_pma_tx_state *p_curr = ADDR_OF(g_states.stations[pid].pma_tx);
   bool           b_ok = (pid < PHY_CNT && is_5b_valid(tx_sym));

   ASSERT(b_ok, pid);

   if (b_ok)
   {
#if defined(DEBUG_PMA_TX)
#if (DEBUG_PMA_TX_LEVEL < 1u)
      if (p_curr->tx_sym != S5B_SILENCE || 
          p_curr->tx_sym_prev != S5B_SILENCE)
#endif // if (DEBUG_PMA_TX_LEVEL < 1u)
      {
         print(DEBUG_PMA_TX, pid, DEBUG_PMA_TX_P, 
               "PMA_TX: UNITDATA.request(%u/%s)", 
               tx_sym, get_name_5b(pid, tx_sym));
      }

#if (DEBUG_PMA_TX_LEVEL < 1u)
      p_curr->tx_sym_prev = tx_sym;
#endif // if (DEBUG_PMA_TX_LEVEL < 1u)
#endif // if defined(DEBUG_PMA_TX)

      ASSERT((!p_curr->b_new), pid);

      p_curr->tx_sym = tx_sym;
      p_curr->b_new = true;
   }
}
