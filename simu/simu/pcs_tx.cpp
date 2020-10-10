#include "states.h"

#if defined(DEBUG_PCS_TX)
static const char *lC_names[_PCTS_CNT] = 
{
   "SILENT", 
   "COMMIT", 
   "SYNC1", 
   "SYNC2", 
   "SSD1", 
   "SSD2", 
   "DATA", 
   "ESD", 
   "BAD_ESD", 
   "GOOD_ESD", 
   "UNJAB_WAIT"
};

static const char *l_get_name(uint pid, E_pcs_tx_state state)
{
   const char *p_retval = "?";
   uint       t_idx;

   if (state == _PCTS_UNDEFINED)
   {
      p_retval = "<N/A>";
   }
   else
   {
      if (state >= _PCTS_MIN && 
          (t_idx = (state - _PCTS_MIN)) < ELEMS_OF(lC_names))
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
#endif // if defined(DEBUG_PCS_TX)

static E_pcs_tx_cmd l_txcmd_decode(const S_mii_signals_tx *p_mii)
{
   E_pcs_tx_cmd retval = PTC_SILENCE;

   if (!p_mii->b_en && p_mii->b_er)
   {
      switch (p_mii->d)
      {
      case 2u:
         retval = PTC_BEACON;
         break;

      case 3u:
         retval = PTC_COMMIT;
         break;

/*      default:
         ASSERT(0u, pid);
         break;*/
      }
   }

   return retval;
}

static sym5b l_txcmd_encode(const S_mii_signals_tx *p_mii)
{
   sym5b retval;

   switch (l_txcmd_decode(p_mii))
   {
   case PTC_BEACON:
      retval = S5B_BEACON;
      break;

   case PTC_COMMIT:
      retval = S5B_COMMIT;
      break;

   default:
      retval = S5B_SILENCE;
      break;
   }

   return retval;
}

static void l_set_state(uint pid, S_station_states *p_states, S_pcs_tx_state *p_curr, E_pcs_tx_state state, bool b_force_print)
{
   S_pcs_state *p_pcs = ADDR_OF(p_states->pcs);
   sym4b       txd;

#if defined(DEBUG_PCS_TX)
   if (p_curr->state != state)
   {
      print(DEBUG_PCS_TX, pid, DEBUG_PCS_TX_P, "PCS_TX: %s->%s", 
            l_get_name(pid, p_curr->state), l_get_name(pid, state));
   }
   else
   {
      if (b_force_print)
      {
         print(DEBUG_PCS_TX, pid, DEBUG_PCS_TX_P, "PCS_TX: %s*", 
               l_get_name(pid, state));
      }
   }
#endif // if defined(DEBUG_PCS_TX)

   switch (p_curr->state = state)
   {
   case PCTS_SILENT:
      p_pcs->b_transmitting = false;
      p_curr->b_err = false;
      p_curr->tx_sym = l_txcmd_encode(ADDR_OF(p_states->MII_TX));
      break;

   case PCTS_COMMIT:
      break;

   case PCTS_SYNC1:
      p_pcs->b_transmitting = true;
      p_curr->b_err = (p_curr->b_err || p_states->MII_TX.b_er);
      p_curr->tx_sym = S5B_SYNC;
      break;

   case PCTS_SYNC2:
      p_curr->b_err = (p_curr->b_err || p_states->MII_TX.b_er);
      break;

   case PCTS_SSD1:
      p_curr->tx_sym = S5B_SSD;
      p_curr->b_err = (p_curr->b_err || p_states->MII_TX.b_er);
      break;

   case PCTS_SSD2:
      p_curr->b_err = (p_curr->b_err || p_states->MII_TX.b_er);
      timer_start(pid, TID_MAX_TIMER, false);
      break;

   case PCTS_DATA:
      // TODO remove (FEC)
      switch (++p_curr->i)
      {
      case 7u:
         p_curr->tx_sym = S5B_ESDBRS;
         break;

      case 8u:
         p_curr->tx_sym = S5B_ESDOK;
         break;

      default:
         txd = (p_states->MII_TX.d & 0x0fu);
#if defined(SUPPORT_SCRAMBLING)
         txd = scramble(pid, txd);
#else // if defined(SUPPORT_SCRAMBLING)
#if defined(DEBUG_PCS_TX)
         print(DEBUG_PCS_TX, pid, DEBUG_PCS_TX_P, "PCS_TX: send(%u)", txd);
#endif // if defined(DEBUG_PCS_TX)
#endif // else if defined(SUPPORT_SCRAMBLING)
         p_curr->tx_sym = ENCODE(pid, txd);
      }

      p_curr->b_err = (p_curr->b_err || p_states->MII_TX.b_er);
      break;

   case PCTS_ESD:
      p_curr->tx_sym = ((l_txcmd_decode(ADDR_OF(p_states->MII_TX)) != PTC_COMMIT) ? 
                        S5B_ESD : S5B_ESDBRS);
      break;

   case PCTS_BAD_ESD:
      p_curr->tx_sym = (p_curr->b_err ? S5B_ESDERR : S5B_ESDJAB);
      break;

   case PCTS_GOOD_ESD:
      p_curr->tx_sym = S5B_ESDBRS;
      break;

   case PCTS_UNJAB_WAIT:
      p_curr->tx_sym = S5B_SILENCE;
      timer_start(pid, TID_UNJAB, false);
      break;

   default:
      ASSERT(0u, pid);
      break;
   }
}

void INIT_pcs_tx(uint pid)
{
   S_pcs_tx_state *p_curr = ADDR_OF(g_states.stations[pid].pcs_tx);
   bool           b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void CLK_pcs_tx(uint pid)
{
   S_station_states *p_states = ADDR_OF(g_states.stations[pid]);
   S_pcs_tx_state   *p_curr = ADDR_OF(p_states->pcs_tx);
   E_pcs_tx_state   new_state = _PCTS_UNDEFINED;
   bool             b_ok = (pid < PHY_CNT);
   bool             b_std;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      if (p_states->pcs.b_reset)
      {
         if (!p_curr->b_reset_prev)
         {
            new_state = PCTS_SILENT;
            p_curr->b_reset_prev = true;

#if defined(SUPPORT_TIMER_STD)
            timer_stop(pid, TID_STD);
#endif // if defined(SUPPORT_TIMER_STD)
            timer_stop(pid, TID_MAX_TIMER);
            timer_stop(pid, TID_UNJAB);
         }
      }
      else
      {
         if (p_curr->b_reset_prev)
         {
            p_curr->b_reset_prev = false;

#if defined(SUPPORT_TIMER_STD)
            timer_start(pid, TID_STD, true);
#endif // if defined(SUPPORT_TIMER_STD)
         }

#if defined(SUPPORT_TIMER_STD)
         if ((b_std = timer_isdone(pid, TID_STD)))
         {
            timer_start(pid, TID_STD, false);
         }

         ASSERT(b_std, pid);
#else // if defined(SUPPORT_TIMER_STD)
         b_std = 1u;
#endif // else if defined(SUPPORT_TIMER_STD)

         switch (p_curr->state)
         {
         case PCTS_SILENT:
            if (b_std)
            {
               if (p_states->MII_TX.b_en)
               {
                  new_state = PCTS_SYNC1;
               }
               else
               {
                  if (//!p_states->MII_TX.b_en && 
                      l_txcmd_decode(ADDR_OF(p_states->MII_TX)) == PTC_COMMIT)
                  {
                     new_state = PCTS_COMMIT;
                  }
                  else
                  {
                     new_state = PCTS_SILENT;
                  }
               }
            }
            break;

         case PCTS_COMMIT:
            if (b_std)
            {
               if (p_states->MII_TX.b_en)
               {
                  new_state = PCTS_SYNC1;
               }
               else
               {
                  if (//!p_states->MII_TX.b_en && 
                      l_txcmd_decode(ADDR_OF(p_states->MII_TX)) == PTC_SILENCE)
                  {
                     new_state = PCTS_ESD;
                  }
               }
            }
            break;

         case PCTS_SYNC1:
            if (b_std)
            {
               new_state = PCTS_SYNC2;
            }
            break;

         case PCTS_SYNC2:
            if (b_std)
            {
               new_state = PCTS_SSD1;
            }
            break;

         case PCTS_SSD1:
            if (b_std)
            {
               new_state = PCTS_SSD2;
            }
            break;

         case PCTS_SSD2:
            if (b_std)
            {
               new_state = PCTS_DATA;

               // TODO remove (FEC)
               p_curr->i = 0u;
            }
            break;

         case PCTS_DATA:
            if (b_std)
            {
               if (p_states->MII_TX.b_en && 
                   !timer_isdone(pid, TID_MAX_TIMER))
               {
                  l_set_state(pid, p_states, p_curr, PCTS_DATA, true);
               }
               else
               {
                  if (!p_states->MII_TX.b_en || 
                      timer_isdone(pid, TID_MAX_TIMER))
                  {
                     new_state = PCTS_ESD;
                  }
               }
            }
            break;

         case PCTS_ESD:
            if (b_std)
            {
               if (p_curr->b_err || 
                   timer_isdone(pid, TID_MAX_TIMER))
               {
                  new_state = PCTS_BAD_ESD;
               }
               else
               {
                  /*if (!p_curr->b_err && 
                        !timer_isdone(pid, TID_MAX_TIMER))*/
                  {
                     new_state = PCTS_GOOD_ESD;
                  }
               }
            }
            break;

         case PCTS_BAD_ESD:
            if (b_std)
            {
               if (p_curr->b_err)
               {
                  new_state = PCTS_SILENT;
               }
               else
               {
                  if (timer_isdone(pid, TID_MAX_TIMER))
                  {
                     new_state = PCTS_UNJAB_WAIT;
                  }
               }
            }
            break;

         case PCTS_GOOD_ESD:
            if (b_std)
            {
               new_state = PCTS_SILENT;
            }
            break;

         case PCTS_UNJAB_WAIT:
            if (b_std && 
                !p_states->MII_TX.b_en && 
                timer_isdone(pid, TID_UNJAB))
            {
               new_state = PCTS_SILENT;
            }
            break;

         default:
            ASSERT(0u, pid);
            break;
         }
      }

      if (new_state != _PCTS_UNDEFINED)
      {
         l_set_state(pid, p_states, p_curr, new_state, false);
      }

      PMA_UNITDATA_request(pid, p_curr->tx_sym);
   }
}
