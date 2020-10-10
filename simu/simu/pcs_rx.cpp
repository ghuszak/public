#include "states.h"

#if defined(DEBUG_PCS_RX)
static const char *lC_names[_PCRS_CNT] = 
{
   "WAIT_SYNC", 
   "SYNCING", 
   "COMMIT", 
   "WAIT_SSD", 
   "BAD_SSD", 
   "PRE", 
   "DATA", 
   "BAD_ESD", 
   "GOOD_ESD", 
   /*"HEARTBEAT1", 
   "HEARTBEAT2", */
   "BEACON1", 
   "BEACON2"
};

static const char *l_get_name(uint pid, E_pcs_rx_state state)
{
   const char *p_retval = "?";
   uint       t_idx;

   if (state == _PCRS_UNDEFINED)
   {
      p_retval = "<N/A>";
   }
   else
   {
      if (state >= _PCRS_MIN && 
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
#endif // if defined(DEBUG_PCS_RX)

static sym4b l_decode(S_pcs_rx_state *p_curr, uint pid)
{
   bool  b_valid;
   sym4b retval = DECODE(pid, p_curr->rxn[3u], ADDR_OF(b_valid));

   ASSERT(b_valid, pid);

   if (b_valid)
   {
#if defined(SUPPORT_SCRAMBLING)
      retval = descramble(pid, retval);
#endif // if defined(SUPPORT_SCRAMBLING)

#if defined(DEBUG_PCS_RX)
      print(DEBUG_PCS_RX, pid, DEBUG_PCS_RX_P, "PCS_RX: receive(%u)", retval);
#endif // if defined(DEBUG_PCS_RX)
   }
   else
   {
      p_curr->mii_rx.b_er = true;
      retval = 0u;
   }

   return retval;
}

static void l_set_state(uint pid, const S_station_states *p_states, S_pcs_rx_state *p_curr, E_pcs_rx_state state, bool b_force_print)
{
   uint precnt;

#if defined(DEBUG_PCS_RX)
   if (p_curr->state != state)
   {
      print(DEBUG_PCS_RX, pid, DEBUG_PCS_RX_P, "PCS_RX: %s->%s", 
            l_get_name(pid, p_curr->state), l_get_name(pid, state));
   }
   else
   {
      if (b_force_print)
      {
         print(DEBUG_PCS_RX, pid, DEBUG_PCS_RX_P, "PCS_RX: %s*", 
               l_get_name(pid, state));
      }
   }
#endif // if defined(DEBUG_PCS_RX)

   switch (p_curr->state = state)
   {
   case PCRS_WAIT_SYNC:
      p_curr->mii_rx.b_dv = false;
      p_curr->mii_rx.b_er = false;
      p_curr->mii_rx.d = 0u;
      //p_curr->rx_cmd = PRC_NONE;
      break;

   case PCRS_SYNCING:
      break;

   case PCRS_COMMIT:
      p_curr->mii_rx.b_er = true;
      p_curr->mii_rx.d = 3u;
      //p_curr->rx_cmd = PRC_COMMIT;
      break;

   case PCRS_WAIT_SSD:
      p_curr->mii_rx.d = 0u;
      p_curr->precnt = 0u;
      p_curr->mii_rx.b_er = false;
      //p_curr->rx_cmd = PRC_NONE;
      break;

   case PCRS_BAD_SSD:
      p_curr->mii_rx.b_er = true;
      p_curr->mii_rx.d = 14u;
      //p_curr->rx_cmd = PRC_NONE;
      break;

   case PCRS_PRE:
      p_curr->mii_rx.b_dv = true;
      p_curr->mii_rx.d = 5u;

      precnt = p_curr->precnt;
      ASSERT((precnt < 9u), pid);

#if defined(SUPPORT_SCRAMBLING)
      if (precnt > 3u)
      {
         l_decode(p_curr, pid);
      }
#endif // if defined(SUPPORT_SCRAMBLING)

      p_curr->precnt = (precnt + 1u);
      break;

   case PCRS_DATA:
      p_curr->mii_rx.d = l_decode(p_curr, pid);
      break;

   case PCRS_BAD_ESD:
      p_curr->mii_rx.b_er = true;
      p_curr->mii_rx.d = 0u;
      break;

   case PCRS_GOOD_ESD:
      p_curr->mii_rx.b_dv = false;
      p_curr->mii_rx.d = 0u;
      break;

   /*case PCRS_HEARTBEAT1:
      break;
   case PCRS_HEARTBEAT2:
      p_curr->rx_cmd = PRC_HEARTBEAT;
      break;*/

   case PCRS_BEACON1:
      break;

   case PCRS_BEACON2:
      p_curr->mii_rx.b_er = true;
      p_curr->mii_rx.d = 2u;
      //p_curr->rx_cmd = PRC_BEACON;
      break;

   default:
      ASSERT(0u, pid);
      break;
   }
}

void INIT_pcs_rx(uint pid)
{
   S_pcs_rx_state *p_curr = ADDR_OF(g_states.stations[pid].pcs_rx);
   bool           b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void CLK_pcs_rx(uint pid)
{
   S_station_states *p_states = ADDR_OF(g_states.stations[pid]);
   S_pcs_rx_state   *p_curr = ADDR_OF(p_states->pcs_rx);
   E_pcs_rx_state   new_state = _PCRS_UNDEFINED;
   bool             b_reset, b_rscd, b_pmudi;
   bool             b_ok = (pid < PHY_CNT);
   bool             b_bad_esd, b_good_esd;
   sym5b            rxn, rxn1, rxn2, rxn3;
   uint             precnt;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      b_reset = (p_states->pcs.b_reset || p_states->pcs.b_transmitting);

      if ((b_pmudi = p_curr->b_pmudi))
      {
         p_curr->b_pmudi = false;
         p_curr->mii_rx.b_er = false;
      }

      if (b_reset)
      {
         if (!p_curr->b_reset_prev)
         {
            memset(p_curr->rxn, 0u, SIZE_OF(p_curr->rxn));

            p_curr->b_reset_prev = true;
            new_state = PCRS_WAIT_SYNC;

#if defined(SUPPORT_TIMER_RSCD)
            timer_stop(pid, TID_RSCD);
#endif // if defined(SUPPORT_TIMER_RSCD)
         }
      }
      else
      {
         if (p_curr->b_reset_prev)
         {
            p_curr->b_reset_prev = false;

#if defined(SUPPORT_TIMER_RSCD)
            timer_start(pid, TID_RSCD, true);
#endif // if defined(SUPPORT_TIMER_RSCD)
         }

#if defined(SUPPORT_TIMER_RSCD)
         if ((b_rscd = timer_isdone(pid, TID_RSCD)))
         {
            timer_start(pid, TID_RSCD, false);
         }

         ASSERT(b_rscd, pid);
#else // if defined(SUPPORT_TIMER_RSCD)
         b_rscd = true;
#endif // else if defined(SUPPORT_TIMER_RSCD)

         if (b_rscd)
         {
            memmove(ADDR_OF(p_curr->rxn[1]), ADDR_OF(p_curr->rxn[0]), 
                    ((ELEMS_OF(p_curr->rxn) - 1u) * SIZE_OF(p_curr->rxn[0])));

            p_curr->rxn[0] = (b_pmudi ? p_curr->pmudi_sym : S5B_SILENCE);

#if (defined(DEBUG_PCS_RX) && DEBUG_PCS_RX_LEVEL >= 1u)
            if (p_curr->rxn[0] != S5B_SILENCE || p_curr->rxn[1] != S5B_SILENCE || 
                p_curr->rxn[2] != S5B_SILENCE || p_curr->rxn[3] != S5B_SILENCE || 
                p_curr->rxn[4] != S5B_SILENCE)
            {
#if 0u
               print(DEBUG_PCS_RX, pid, DEBUG_PCS_RX_P, "PCS_RX: (RXn-4=%s) RXn-3=%s RXn-2=%s RXn-1=%s [RXn=%s]", 
                     get_name_5b(pid, p_curr->rxn[4]), get_name_5b(pid, p_curr->rxn[3]), 
                     get_name_5b(pid, p_curr->rxn[2]), get_name_5b(pid, p_curr->rxn[1]), 
                     get_name_5b(pid, p_curr->rxn[0]));
#else
               print(DEBUG_PCS_RX, pid, DEBUG_PCS_RX_P, "PCS_RX: [RXn=%s] RXn-1=%s RXn-2=%s RXn-3=%s (RXn-4=%s)", 
                     get_name_5b(pid, p_curr->rxn[0]), get_name_5b(pid, p_curr->rxn[1]), 
                     get_name_5b(pid, p_curr->rxn[2]), get_name_5b(pid, p_curr->rxn[3]), 
                     get_name_5b(pid, p_curr->rxn[4]));
#endif
            }
#endif // if (defined(DEBUG_PCS_RX) && DEBUG_PCS_RX_LEVEL >= 1u)
         }

         switch (p_curr->state)
         {
         case PCRS_WAIT_SYNC:
            if (b_rscd)
            {
               switch (p_curr->rxn[0])
               {
               case S5B_SYNC:
                  new_state = PCRS_SYNCING;
                  break;

               case S5B_SSD:
                  new_state = PCRS_WAIT_SSD;
                  break;

               case S5B_BEACON:
                  new_state = PCRS_BEACON1;
                  break;
               }
            }
            break;

         case PCRS_SYNCING:
            if (b_rscd)
            {
               rxn = p_curr->rxn[0];

               if (rxn == S5B_ESD || 
#if defined(SUPPORT_FC)
                   false)
#else // if defined(SUPPORT_FC)
                   (rxn != S5B_SSD && 
                    rxn != S5B_SYNC))
#endif // else if defined(SUPPORT_FC)
               {
                  new_state = PCRS_WAIT_SYNC;
               }
               else
               {
                  switch (rxn)
                  {
                  case S5B_SYNC:
                     new_state = PCRS_COMMIT;
                     break;

                  case S5B_SSD:
                     new_state = PCRS_WAIT_SSD;
                     break;

#if defined(SUPPORT_FC)
                  default:
                     if (rxn != S5B_ESD)
                     {
                        new_state = PCRS_BAD_SSD;
                     }
                  }
#endif // if defined(SUPPORT_FC)
               }
            }
            break;

         case PCRS_COMMIT:
            if (b_rscd)
            {
               rxn = p_curr->rxn[0];

               if (rxn == S5B_ESD || 
#if defined(SUPPORT_FC)
                   false)
#else // if defined(SUPPORT_FC)
                   (rxn != S5B_SSD && 
                    rxn != S5B_SYNC))
#endif // else if defined(SUPPORT_FC)
               {
                  new_state = PCRS_WAIT_SSD;
               }
               else
               {
                  switch (rxn)
                  {
                  case S5B_SSD:
                     new_state = PCRS_WAIT_SSD;
                     break;

#if defined(SUPPORT_FC)
                  default:
                     if (rxn != S5B_SYNC)
                     {
                        new_state = PCRS_BAD_SSD;
                     }
                  }
#endif // if defined(SUPPORT_FC)
               }
            }
            break;

         case PCRS_WAIT_SSD:
            if (b_rscd)
            {
               if (p_curr->rxn[0] == S5B_SSD)
               {
                  new_state = PCRS_PRE;
               }
               else
               {
#if defined(SUPPORT_FC)
                  new_state = PCRS_BAD_SSD;
#else // if defined(SUPPORT_FC)
                  new_state = PCRS_WAIT_SYNC;
#endif // else if defined(SUPPORT_FC)
               }
            }
            break;

         case PCRS_BAD_SSD:
            if (b_rscd)
            {
               rxn = p_curr->rxn[0];

               if (rxn == S5B_SILENCE || 
                   rxn == S5B_ESD)
               {
                  new_state = PCRS_WAIT_SYNC;
               }
            }
            break;

         case PCRS_PRE:
            if (b_rscd)
            {
               precnt = p_curr->precnt;

               if (precnt == 9u)
               {
                  new_state = PCRS_DATA;
               }
               else
               {
                  ASSERT((precnt < 9u), pid);

                  l_set_state(pid, p_states, p_curr, PCRS_PRE, true);
               }
            }
            break;

         case PCRS_DATA:
            if (b_rscd)
            {
               rxn1 = p_curr->rxn[1];
               rxn2 = p_curr->rxn[2];
               rxn3 = p_curr->rxn[3];

               b_bad_esd = (((rxn2 == S5B_ESD || rxn2 == S5B_ESDBRS) && 
                             rxn1 != S5B_ESDOK && 
                             rxn3 != S5B_ESD && 
                             rxn3 != S5B_ESDBRS) || 
                            rxn3 == S5B_SILENCE);
               b_good_esd = ((rxn3 == S5B_ESD || rxn3 == S5B_ESDBRS) && 
                             rxn2 == S5B_ESDOK);
               ASSERT((!b_bad_esd || !b_good_esd), pid);

               if (b_good_esd)
               {
                  new_state = PCRS_GOOD_ESD;
               }
               else
               {
                  if (b_bad_esd)
                  {
                     new_state = PCRS_BAD_ESD;
                  }
                  else
                  {
                     // Quasi-ELSE
                     l_set_state(pid, p_states, p_curr, PCRS_DATA, true);
                  }
               }
            }
            break;

         case PCRS_BAD_ESD:
         case PCRS_GOOD_ESD:
            if (b_rscd)
            {
               new_state = PCRS_WAIT_SYNC;
            }
            break;

         /*case PCRS_HEARTBEAT1:
            break;
         case PCRS_HEARTBEAT2:
            break;*/

         case PCRS_BEACON1:
            if (b_rscd)
            {
               new_state = ((p_curr->rxn[0] == S5B_BEACON) ? 
                            PCRS_BEACON2 : PCRS_WAIT_SYNC);
            }
            break;

         case PCRS_BEACON2:
            if (b_rscd && p_curr->rxn[0] != S5B_BEACON)
            {
               new_state = PCRS_WAIT_SYNC;
            }
            break;

         default:
            ASSERT(0u, pid);
            break;
         }
      }

      if (new_state != _PCRS_UNDEFINED)
      {
         l_set_state(pid, p_states, p_curr, new_state, false);
      }
   }
}

void PMA_UNITDATA_indication(uint pid, sym5b rx_sym)
{
   S_pcs_rx_state *p_curr = ADDR_OF(g_states.stations[pid].pcs_rx);
   bool           b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      ASSERT((!p_curr->b_pmudi), pid);

      p_curr->pmudi_sym = rx_sym;
      p_curr->b_pmudi = true;
   }
}
