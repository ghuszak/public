#include "states.h"

#if defined(DEBUG_PLCA_CTRL)
static const char *lC_names[_PCS_CNT] =
{
   "DISABLE", 
   "RESYNC", 
   "RECOVER", 
   "SEND_BEACON", 
   "SYNCING", 
   "WAIT_TO", 
   "EARLY_RECEIVE", 
   "COMMIT", 
   "YIELD", 
   "RECEIVE", 
   "TRANSMIT", 
   "BURST", 
   "NEXT_TX_OPPORTUNITY", 
   "ABORT"
};

static const char *l_get_name(uint pid, E_plca_ctrl_state state)
{
   const char *p_retval = "?";
   uint       t_idx;

   if (state == _PCS_UNDEFINED)
   {
      p_retval = "<N/A>";
   }
   else
   {
      if (state >= _PCS_MIN && 
          (t_idx = (state - _PCS_MIN)) < ELEMS_OF(lC_names))
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
#endif // if defined(DEBUG_PLCA_CTRL)

static void l_set_state(uint pid, S_plca_ctrl_state *p_curr, S_plca_state *p_plca, E_plca_ctrl_state state)
{
#if defined(DEBUG_PLCA_CTRL)
   if (p_curr->state != state)
   {
      print(DEBUG_PLCA_CTRL, pid, DEBUG_PLCA_CTRL_P, "PLCA_CTRL: %s->%s",
            l_get_name(pid, p_curr->state), l_get_name(pid, state));
   }
#endif // if defined(DEBUG_PLCA_CTRL)

   switch (p_curr->state = state)
   {
   case PCS_DISABLE:
      p_plca->b_committed = false;
      p_plca->tx_cmd = PRTC_NONE;
      p_plca->b_active = false;
      p_curr->cur_id = 0u;
      break;

   case PCS_RESYNC:
      p_plca->b_active = false;
      break;

   case PCS_RECOVER:
      p_plca->b_active = false;
      break;

   case PCS_SEND_BEACON:
      timer_start(pid, TID_PLCAC_BEACON, false);
      p_plca->tx_cmd = PRTC_BEACON;
      p_plca->b_active = true;
      break;

   case PCS_SYNCING:
      p_plca->tx_cmd = PRTC_NONE;
      p_plca->b_active = true;
      p_curr->cur_id = 0u;

      if (p_plca->node_id != 0u && 
          p_plca->rx_cmd != PRTC_BEACON)
      {
         timer_start(pid, TID_PLCAC_INVALID_BEACON, false);
      }
      break;

   case PCS_WAIT_TO:
      timer_start(pid, TID_PLCAC_TO, false);
      break;

   case PCS_EARLY_RECEIVE:
      timer_stop(pid, TID_PLCAC_TO);
      timer_start(pid, TID_PLCAC_BEACON_DET, false);
      break;

   case PCS_COMMIT:
      timer_stop(pid, TID_PLCAC_TO);
      p_plca->tx_cmd = PRTC_COMMIT;
      p_plca->b_committed = true;
      p_curr->bc = 0u;
      break;

   case PCS_YIELD:
      break;

   case PCS_RECEIVE:
      break;

   case PCS_TRANSMIT:
      p_plca->tx_cmd = PRTC_NONE;

      if (p_curr->bc >= p_plca->max_bc)
      {
         p_plca->b_committed = false;
      }
      break;

   case PCS_BURST:
      ASSERT((p_curr->bc < 255u), pid);

      p_curr->bc++;
      p_plca->tx_cmd = PRTC_COMMIT;
      timer_start(pid, TID_PLCAC_BURST, false);
      break;

   case PCS_NEXT_TX_OPPORTUNITY:
      ASSERT((p_curr->cur_id < 255u), pid);

      p_curr->cur_id++;
      p_plca->b_committed = false;
      break;

   case PCS_ABORT:
      p_plca->tx_cmd = PRTC_NONE;
      break;

   default:
      ASSERT(0u, pid);
      break;
   }
}

void INIT_plca_ctrl(uint pid)
{
   S_plca_ctrl_state *p_curr = ADDR_OF(g_states.stations[pid].plca_ctrl);
   bool              b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void CLK_plca_ctrl(uint pid, bool b_mcd, bool b_pmcd)
{
   S_station_states       *p_states = ADDR_OF(g_states.stations[pid]);
   S_plca_state           *p_plca = ADDR_OF(p_states->plca);
   const S_mii_signals_tx *p_mii_tx = ADDR_OF(p_plca->mii_tx);
   S_plca_ctrl_state      *p_curr = ADDR_OF(p_states->plca_ctrl);
   const S_mii_signals_rx *p_mii_rx = ADDR_OF(p_states->pcs_rx.mii_rx);
   E_plca_ctrl_state      new_state = _PCS_UNDEFINED;
   bool                   b_ok = (pid < PHY_CNT);
   uint                   node_id;
   bool                   b_en;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
#if (PLCA_EN == 1u)
      b_en = true;
#else // if (PLCA_EN == 1u)
      b_en = false;
#endif // else if (PLCA_EN == 1u)

      node_id = p_plca->node_id;

      if (p_plca->b_reset || 
          !b_en || 
          node_id == CAST_TO(-1, uint8))
      {
         new_state = PCS_DISABLE;

         if (!p_curr->b_reset_prev)
         {
            p_curr->b_reset_prev = true;

            timer_stop(pid, TID_PLCAC_BEACON);
            timer_stop(pid, TID_PLCAC_BURST);
            timer_stop(pid, TID_PLCAC_TO);
         }
      }
      else
      {
         p_curr->b_reset_prev = false;

         switch (p_curr->state)
         {
         case PCS_DISABLE:
            new_state = ((node_id == 0u)? 
                         PCS_RECOVER : PCS_RESYNC);
            break;

         case PCS_RESYNC:
            if (node_id == 0u)
            {
               if (!p_mii_rx->b_crs && 
                   b_pmcd)
               {
                  new_state = PCS_SEND_BEACON;
               }
            }
            else
            {
               if (p_mii_rx->b_crs)
               {
                  new_state = PCS_EARLY_RECEIVE;
               }
            }
            break;

         case PCS_RECOVER:
            new_state = PCS_WAIT_TO;
            break;

         case PCS_SEND_BEACON:
            if (timer_isdone(pid, TID_PLCAC_BEACON))
            {
               new_state = PCS_SYNCING;
            }
            break;

         case PCS_SYNCING:
            if (!p_mii_rx->b_crs)
            {
               new_state = PCS_WAIT_TO;
            }
            break;

         case PCS_WAIT_TO:
            if (p_mii_rx->b_crs)
            {
               new_state = PCS_EARLY_RECEIVE;
            }
            else
            {
               if (node_id == p_curr->cur_id)
               {
                  if (p_plca->b_packet_pending && 
                      p_plca->b_active)
                  {
                     new_state = PCS_COMMIT;
                  }
                  else
                  {
                     new_state = PCS_YIELD;
                  }
               }
               else
               {
                  if (timer_isdone(pid, TID_PLCAC_TO))
                  {
                     new_state = PCS_NEXT_TX_OPPORTUNITY;
                  }
               }
            }
            break;

         case PCS_EARLY_RECEIVE:
            if (p_mii_rx->b_crs)
            {
               if (p_plca->b_receiving)
               {
                  new_state = PCS_RECEIVE;
               }
               else
               {
                  if (node_id != 0u && 
                      p_plca->rx_cmd == PRTC_BEACON)
                  {
                     new_state = PCS_SYNCING;
                  }
               }
            }
            else
            {
               if (node_id == 0u)
               {
                  new_state = PCS_RECEIVE;
               }
               else
               {
                  if (p_plca->rx_cmd == PRTC_BEACON || 
                      !timer_isdone(pid, TID_PLCAC_BEACON_DET))
                  {
                     if (p_plca->b_receiving)
                     {
                        new_state = PCS_SYNCING;
                     }
                  }
                  else
                  {
                     new_state = PCS_RESYNC;
                  }
               }
            }
            break;

         case PCS_COMMIT:
            if (p_mii_tx->b_en)
            {
               new_state = PCS_TRANSMIT;
            }
            else
            {
               if (!p_plca->b_packet_pending)
               {
                  new_state = PCS_ABORT;
               }
            }
            break;

         case PCS_YIELD:
            if (timer_isdone(pid, TID_PLCAC_TO))
            {
               new_state = PCS_NEXT_TX_OPPORTUNITY;
            }
            else
            {
               if (p_mii_rx->b_crs)
               {
                  new_state = PCS_EARLY_RECEIVE;
               }
            }
            break;

         case PCS_RECEIVE:
            if (!p_mii_rx->b_crs)
            {
               new_state = PCS_NEXT_TX_OPPORTUNITY;
            }
            break;

         case PCS_TRANSMIT:
            if (!p_mii_tx->b_en)
            {
               if (p_curr->bc < p_plca->max_bc)
               {
                  new_state = PCS_BURST;
               }
               else
               {
                  if (!p_mii_rx->b_crs)
                  {
                     new_state = PCS_NEXT_TX_OPPORTUNITY;
                  }
               }
            }
            break;

         case PCS_BURST:
            if (p_mii_tx->b_en)
            {
               new_state = PCS_TRANSMIT;
            }
            else
            {
               if (timer_isdone(pid, TID_PLCAC_BURST))
               {
                  new_state = PCS_ABORT;
               }
            }
            break;

         case PCS_NEXT_TX_OPPORTUNITY:
            if ((node_id == 0u && 
                 p_curr->cur_id >= PHY_CNT) || 
                p_curr->cur_id == 255u)
            {
               new_state = PCS_RESYNC;
            }
            else
            {
               new_state = PCS_WAIT_TO;
            }
            break;

         case PCS_ABORT:
            if (!p_mii_rx->b_crs)
            {
               new_state = PCS_NEXT_TX_OPPORTUNITY;
            }
            break;

         default:
            ASSERT(0u, pid);
            break;
         }
      }

      if (new_state != _PCS_UNDEFINED)
      {
         l_set_state(pid, p_curr, p_plca, new_state);
      }
   }
}
