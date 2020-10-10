#include "states.h"

#if defined(DEBUG_PLCA_DATA)
static const char *lC_names[_PDS_CNT] =
{
   "NORMAL", 
   "PDS_WAIT_IDLE", 
   "IDLE", 
   "RECEIVE", 
   "HOLD", 
   "ABORT", 
   "COLLIDE", 
   "DELAY_PENDING", 
   "PENDING", 
   "WAIT_MAC", 
   "TRANSMIT", 
   "FLUSH"
};

static const char *l_get_name(uint pid, E_plca_data_state state)
{
   const char *p_retval = "?";
   uint       t_idx;

   if (state == _PDS_UNDEFINED)
   {
      p_retval = "<N/A>";
   }
   else
   {
      if (state >= _PDS_MIN && 
          (t_idx = (state - _PDS_MIN)) < ELEMS_OF(lC_names))
      {
         p_retval = lC_names[t_idx];
      }
      else
      {
         ASSERT(0u, pid);
      }
   }

   return p_retval;
}
#endif // if defined(DEBUG_PLCA_DATA)

static bool l_encode_txer(uint pid, E_plca_rtx_cmd tx_cmd, bool b_pla_txer)
{
   ASSERT((tx_cmd >= _PRTC_MIN && tx_cmd <= _PRTC_MAX), pid);

   return (tx_cmd == PRTC_BEACON || 
           tx_cmd == PRTC_COMMIT || 
           b_pla_txer);
}

static sym4b l_encode_txd(uint pid, E_plca_rtx_cmd tx_cmd)
{
   sym4b retval = 0u;

   switch (tx_cmd)
   {
   case PRTC_BEACON:
      retval = 2u;
      break;

   case PRTC_COMMIT:
      retval = 3u;
      break;

   default:
      retval = 0u;
      break;
   }

   return retval;
}

static sym4b l_get_data(uint pid, const S_plca_data_state *p_curr)
{
   uint t_idx = ((p_curr->a <= p_curr->n) ? p_curr->n : 
                 (p_curr->n + PLCA_DATA_DELAY_LINE_LEN));
   sym4b retval = p_curr->delay_line[t_idx - p_curr->a];

   ASSERT((retval >= 0u && retval <= 16u), pid);

   return (retval - 1u);
}

static void l_set_state(uint pid, S_plca_data_state *p_curr, S_plca_state *p_plca, const S_mii_signals_rx *p_mii_rx_s, const S_mii_signals_tx *p_mii_tx_n, E_plca_data_state state)
{
   S_mii_signals_rx *p_mii_rx_n = ADDR_OF(p_plca->mii_rx);
   S_mii_signals_tx *p_mii_tx_s = ADDR_OF(p_plca->mii_tx);

#if defined(DEBUG_PLCA_DATA)
   if (p_curr->state != state)
   {
      print(DEBUG_PLCA_DATA, pid, DEBUG_PLCA_DATA_P, "PLCA_DATA: %s->%s",
            l_get_name(pid, p_curr->state), l_get_name(pid, state));
   }
   else
   {
#if (DEBUG_PLCA_DATA_LEVEL  >= 1u)
      print(DEBUG_PLCA_DATA, pid, DEBUG_PLCA_DATA_P, "PLCA_DATA: %s*", 
            l_get_name(pid, state));
#endif // if (DEBUG_PLCA_DATA_LEVEL  >= 1u)
   }
#endif // if defined(DEBUG_PLCA_DATA)

   switch (p_curr->state = state)
   {
   case PDS_NORMAL:
      p_plca->b_packet_pending = false;

      p_mii_rx_n->b_crs = p_mii_rx_s->b_crs;
      p_mii_rx_n->b_col = p_mii_rx_s->b_col;

      p_mii_tx_s->d = p_mii_tx_n->d;
      p_mii_tx_s->b_en = p_mii_tx_n->b_en;
      p_mii_tx_s->b_er = p_plca->mii_tx.b_er;
      break;

   case PDS_WAIT_IDLE:
      p_plca->b_packet_pending = false;

      p_mii_rx_n->b_crs = false;
      p_mii_rx_n->b_col = true;

      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      p_mii_tx_s->b_en = false;
      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);

      p_curr->a = 0u;
      p_curr->b = 0u;
      break;

   case PDS_IDLE:
      p_plca->b_packet_pending = false;

      p_mii_rx_n->b_crs = false;
      p_mii_rx_n->b_col = false;

      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      p_mii_tx_s->b_en = false;
      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);

      p_curr->a = 0u;
      p_curr->b = 0u;
#if defined(_DO_ASSERT)
      memset(ADDR_OF(p_curr->delay_line), 0u, SIZE_OF(p_curr->delay_line));
#endif // if defined(_DO_ASSERT)
      break;

   case PDS_RECEIVE:
      p_mii_rx_n->b_crs = (p_mii_rx_s->b_crs && p_plca->rx_cmd != PRTC_COMMIT);
      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);
      break;

   case PDS_HOLD:
      p_plca->b_packet_pending = true;
      p_mii_rx_n->b_crs = true;
      p_curr->a++;

      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);
      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      break;

   case PDS_ABORT:
      p_plca->b_packet_pending = false;

      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);
      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      break;

   case PDS_COLLIDE:
      p_plca->b_packet_pending = false;
      p_mii_rx_n->b_crs = true;
      p_mii_rx_n->b_col = true;

      timer_start(pid, TID_PLCAD_PENDING, false);

      p_curr->a = 0u;
      p_curr->b = 0u;
#if defined(_DO_ASSERT)
      memset(ADDR_OF(p_curr->delay_line), 0u, SIZE_OF(p_curr->delay_line));
#endif // if defined(_DO_ASSERT)

      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);
      break;

   case PDS_DELAY_PENDING:
      p_mii_rx_n->b_col = false;

      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);
      break;

   case PDS_PENDING:
      p_plca->b_packet_pending = true;
      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);
      break;

   case PDS_WAIT_MAC:
      timer_start(pid, TID_PLCAD_COMMIT, false);

      p_mii_rx_n->b_crs = false;
      p_mii_tx_s->d = l_encode_txd(pid, p_curr->tx_cmd_sync);
      p_mii_tx_s->b_er = l_encode_txer(pid, p_curr->tx_cmd_sync, p_mii_tx_n->b_er);
      break;

   case PDS_TRANSMIT:
      p_plca->b_packet_pending = false;
      p_mii_rx_n->b_crs = true;
      p_mii_tx_s->d = l_get_data(pid, p_curr);
      p_mii_tx_s->b_en = true;
      p_mii_tx_s->b_er = p_plca->mii_tx.b_er;
      p_mii_rx_n->b_col = p_mii_rx_s->b_col;
      if (p_mii_rx_s->b_col)
      {
         p_curr->a = 0u;
      }
      break;

   case PDS_FLUSH:
      p_mii_rx_n->b_crs = true;
      p_mii_tx_s->d = l_get_data(pid, p_curr);
      p_mii_tx_s->b_en = true;
      p_mii_tx_s->b_er = p_plca->mii_tx.b_er;
      ASSERT((p_curr->a <= PLCA_DATA_DELAY_LINE_LEN && 
              p_curr->b < p_curr->a), pid);
      p_curr->b++;
      p_mii_rx_n->b_col = p_mii_rx_s->b_col;
      break;

   default:
      ASSERT(0u, pid);
      break;
   }
}

void INIT_plca_data(uint pid)
{
   S_plca_data_state *p_curr = ADDR_OF(g_states.stations[pid].plca_data);
   bool              b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void CLK_plca_data(uint pid, bool b_mcd)
{
   S_station_states          *p_states = ADDR_OF(g_states.stations[pid]);
   const S_mii_signals_tx    *p_mii_tx_n = ADDR_OF(p_states->mac_pls.mii_tx);
   const S_mii_signals_rx    *p_mii_rx_s = ADDR_OF(p_states->pcs_rx.mii_rx);
   const S_plca_status_state *p_plca_s = ADDR_OF(p_states->plca_status);
   S_plca_data_state         *p_curr = ADDR_OF(p_states->plca_data);
   S_plca_state              *p_plca = ADDR_OF(p_states->plca);
   E_plca_data_state         new_state = _PDS_UNDEFINED;
   bool                      b_ok = (pid < PHY_CNT);
   bool                      b_en;
   uint                      n;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
#if (PLCA_EN == 1u)
      b_en = true;
#else // if (PLCA_EN == 1u)
      b_en = false;
#endif // else if (PLCA_EN == 1u)

      if (p_plca->b_reset || 
          !b_en || 
          !p_plca_s->b_ok)
      {
         new_state = PDS_NORMAL;

         if (!p_curr->b_reset_prev)
         {
            p_curr->b_reset_prev = true;

#if defined(_DO_ASSERT)
            memset(ADDR_OF(p_curr->delay_line), 0u, SIZE_OF(p_curr->delay_line));
#endif // if defined(_DO_ASSERT)

            timer_stop(pid, TID_PLCAD_COMMIT);
            timer_stop(pid, TID_PLCAD_PENDING);
         }
      }
      else
      {
         p_curr->b_reset_prev = false;

         if (b_mcd)
         {
            p_curr->tx_cmd_sync = p_plca->tx_cmd;

            n = p_curr->n;
            ASSERT((n < PLCA_DATA_DELAY_LINE_LEN), pid);
            p_curr->n = (((++n) == PLCA_DATA_DELAY_LINE_LEN) ? 0u : n);

            if (p_mii_tx_n->b_en)
            {
               p_curr->delay_line[n] = (p_mii_tx_n->d + 1u);
            }
         }

         switch (p_curr->state)
         {
         case PDS_NORMAL:
#if (PLCA_EN == 1u)
            new_state = PDS_IDLE;
#else // if (PLCA_EN == 1u)
            new_state = PDS_NORMAL;
#endif // else if (PLCA_EN == 1u)
            break;

         case PDS_WAIT_IDLE:
            if (b_mcd && 
                (!p_mii_rx_s->b_crs || 
                 p_mii_tx_n->b_en))
            {
               new_state = (p_mii_rx_s->b_crs ? 
                            PDS_TRANSMIT : PDS_IDLE);
            }
            else
            {
               // ELSE
               new_state = PDS_WAIT_IDLE;
            }
            break;

         case PDS_IDLE:
            if (p_mii_tx_n->b_en)
            {
               new_state = PDS_HOLD;
            }
            else
            {
               if (p_plca->tx_cmd == PRTC_NONE && 
                   p_plca->b_receiving)
               {
                  new_state = PDS_RECEIVE;
               }
               else
               {
                  // ELSE
                  new_state = PDS_IDLE;
               }
            }
            break;

         case PDS_RECEIVE:
            if (p_mii_tx_n->b_en)
            {
               new_state = PDS_COLLIDE;
            }
            else
            {
               if (!p_plca->b_receiving)
               {
                  new_state = PDS_IDLE;
               }
               else
               {
                  // ELSE
                  new_state = PDS_RECEIVE;
               }
            }
            break;

         case PDS_HOLD:
            if (!p_mii_tx_n->b_er && 
                (p_plca->b_receiving || 
                 p_curr->a >= PLCA_DATA_DELAY_LINE_LEN))
            {
               new_state = PDS_COLLIDE;
            }
            else
            {
               if (b_mcd)
               {
                  if (p_mii_tx_n->b_er)
                  {
                     new_state = PDS_ABORT;
                  }
                  else
                  {
                     /*if (!p_plca->b_receiving && 
                         p_curr->a < PLCA_DATA_DELAY_LINE_LEN)*/
                     {
                        new_state = ((p_plca->b_committed) ? 
                                     PDS_TRANSMIT : PDS_HOLD);
                     }
                  }
               }
            }
            break;

         case PDS_ABORT:
            if (!p_mii_tx_n->b_en)
            {
               new_state = PDS_IDLE;
            }
            else
            {
               // ELSE
               new_state = PDS_ABORT;
            }
            break;

         case PDS_COLLIDE:
            if (!p_mii_tx_n->b_en)
            {
               new_state = PDS_DELAY_PENDING;
            }
            else
            {
               // ELSE
               new_state = PDS_COLLIDE;
            }
            break;

         case PDS_DELAY_PENDING:
            if (timer_isdone(pid, TID_PLCAD_PENDING))
            {
               new_state = PDS_PENDING;
            }
            else
            {
               // ELSE
               new_state = PDS_DELAY_PENDING;
            }
            break;

         case PDS_PENDING:
            if (p_plca->b_committed)
            {
               new_state = PDS_WAIT_MAC;
            }
            else
            {
               // ELSE
               new_state = PDS_PENDING;
            }
            break;

         case PDS_WAIT_MAC:
            if (b_mcd && p_mii_tx_n->b_en)
            {
               new_state = PDS_TRANSMIT;
            }
            else
            {
               if (timer_isdone(pid, TID_PLCAD_PENDING))
               {
                  new_state = PDS_WAIT_IDLE;
               }
            }

            if (new_state == _PDS_UNDEFINED)
            {
               // ELSE
               new_state = PDS_WAIT_MAC;
            }
            break;

         case PDS_TRANSMIT:
            if (b_mcd)
            {
               if (p_mii_tx_n->b_en)
               {
                  new_state = PDS_TRANSMIT;
               }
               else
               {
                  ASSERT((p_curr->a <= PLCA_DATA_DELAY_LINE_LEN), pid);

                  if (p_curr->a == 0u)
                  {
                     new_state = PDS_WAIT_IDLE;
                  }
                  else
                  {
                     new_state = PDS_FLUSH;
                  }
               }
            }
            break;

         case PDS_FLUSH:
            if (b_mcd)
            {
               ASSERT((p_curr->b <= p_curr->a && 
                       p_curr->a <= PLCA_DATA_DELAY_LINE_LEN && 
                       p_curr->b <= PLCA_DATA_DELAY_LINE_LEN), pid);

               if (p_curr->a == p_curr->b)
               {
                  new_state = PDS_WAIT_IDLE;
               }
               else
               {
                  new_state = PDS_FLUSH;
               }
            }
            break;

         default:
            ASSERT(0u, pid);
            break;
         }
      }

      if (new_state != _PDS_UNDEFINED)
      {
         l_set_state(pid, p_curr, p_plca, p_mii_rx_s, p_mii_tx_n, new_state);
      }
   }
}
