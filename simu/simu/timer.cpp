#include "states.h"

typedef enum e_timer_set 
{
   TS_STOP, 
   TS_START_RESET, 
   TS_START_DONE
} E_timer_set;

#if defined(DEBUG_TIMER)
static const char *lC_names[_TID_MAX_CNT] = 
{
   "MAX_TIMER", 
   "UNJAB", 
#if defined(SUPPORT_TIMER_STD)
   "STD", 
#endif // if defined(SUPPORT_TIMER_STD)

#if defined(SUPPORT_TIMER_RSCD)
   "RSCD", 
#endif // if defined(SUPPORT_TIMER_RSCD)

   "PLCAC/BEACON", 
   "PLCAC/BEACON_DET", 
   "PLCAC/INVALID_BEACON", 
   "PLCAC/BURST", 
   "PLCAC/TO", 

   "PLCAD/COMMIT", 
   "PLCAD/PENDING", 

   "PLCAS/PLCA_STATUS"
};
#endif // if defined(DEBUG_TIMER)

#if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)
static const char *l_get_state(uint pid, clock counter, clock timeout)
{
   static char l_buff[21];
   const char  *p_retval;
   int         t_int;

   if (counter > timeout)
   {
      p_retval = "DONE";
   }
   else
   {
      if (counter == 0u)
      {
         p_retval = "STOP";
      }
      else
      {
         p_retval = l_buff;
         l_buff[20] = 0u;

#if (defined(_WIN32) || defined(_WIN64))
         t_int = sprintf_s(l_buff, SIZE_OF(l_buff), "%llu", counter);

         ASSERT((t_int > 0), pid);
#else // if (defined(_WIN32) || defined(_WIN64))
         sprintf(l_buff, "%llu", counter);

         ASSERT((p_retval[20] == 0u), pid);
#endif // else if (defined(_WIN32) || defined(_WIN64))
      }
   }

   return p_retval;
}
#endif // if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)

static bool l_timer_set(uint pid, uint tid, uint type)
{
   S_timer_state *p_timer = ADDR_OF(g_states.stations[pid].timers[tid -= _TID_MAX_MIN]);
   bool          b_retval = (pid < PHY_CNT && 
                             tid < _TID_MAX_CNT && 
                             tid < ELEMS_OF(g_states.stations[pid].timers) && 
#if defined(DEBUG_TIMER)
                             tid < ELEMS_OF(lC_names) && 
                             lC_names[tid] != NULL && 
#endif // if defined(DEBUG_TIMER)
                             p_timer->timeout < CAST_TO(-1, clock) && 
                             (p_timer->timeout != 0u || 
                              type == TS_STOP));
   clock          value = 0u;

   ASSERT(b_retval, pid);

   if (b_retval)
   {
      switch (type)
      {
      case TS_STOP:
         value = 0u;

#if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)
         print(DEBUG_TIMER, pid, DEBUG_TIMER_P, "TIM: STOP(%s, %s/%llu)", 
               lC_names[tid], l_get_state(pid, p_timer->counter, p_timer->timeout), 
               p_timer->timeout);
#endif // if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)
         break;

      case TS_START_RESET:
         value = 1u;

#if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)
         print(DEBUG_TIMER, pid, DEBUG_TIMER_P, "TIM: START(%s, %s->%llu/%llu)", 
               lC_names[tid], l_get_state(pid, p_timer->counter, p_timer->timeout), 
               value, p_timer->timeout);
#endif // if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)
         break;

      case TS_START_DONE:
         value = (p_timer->timeout + 1u);

#if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)
         print(DEBUG_TIMER, pid, DEBUG_TIMER_P, "TIM: START(%s, %llu->DONE/%llu)", 
               lC_names[tid], p_timer->counter, p_timer->timeout);
#endif // if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 1u)
         break;

      default:
         ASSERT(0u, pid);

         b_retval = false;
         break;
      }

      if (b_retval)
      {
         p_timer->counter = value;
      }
   }
   else
   {
      b_retval = false;
   }

   return b_retval;
}

bool timer_start(uint pid, uint tid, bool b_indone)
{
   return l_timer_set(pid, tid, (b_indone ? TS_START_DONE : TS_START_RESET));
}

bool timer_stop(uint pid, uint tid)
{
   return l_timer_set(pid, tid, TS_STOP);
}

bool timer_isdone(uint pid, uint tid)
{
   bool          b_retval = false;
   bool          b_ok = (pid < PHY_CNT && 
                         tid >= _TID_MAX_MIN && tid <= _TID_MAX_MAX && 
                         (tid - _TID_MAX_MIN) < ELEMS_OF(g_states.stations[pid].timers));
   S_timer_state *p_timer = ADDR_OF(g_states.stations[pid].timers[tid -= _TID_MAX_MIN]);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      if (p_timer->timeout != 0u && 
          p_timer->counter > p_timer->timeout)
      {
         b_retval = true;

#if defined(DEBUG_TIMER)
         print(DEBUG_TIMER, pid, DEBUG_TIMER_P, "TIM: %s_done", lC_names[tid]);
#endif // if defined(DEBUG_TIMER)
      }
      else
      {
         if (p_timer->counter != 0u)
         {
#if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 2u)
            print(DEBUG_TIMER, pid, DEBUG_TIMER_P, "TIM: %s_not_done (%llu/%llu)", 
                  lC_names[tid], p_timer->counter, p_timer->timeout);
#endif // if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 2u)
         }
         else
         {
#if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 2u)
            print(DEBUG_TIMER, pid, DEBUG_TIMER_P, "TIM: %s_not_done (%llu)", 
                  lC_names[tid], p_timer->timeout);
#endif // if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 2u)

            //ASSERT(0u, pid);
         }
      }
   }

   return b_retval;
}

static clock l_us_to_tick(uint pid, clock us)
{
   clock t_clk = (us * TIMER_CLK);

   ASSERT((us != 0u && 
           (t_clk / us) == TIMER_CLK && 
           (t_clk % 1000000u) == 0u), pid);

   return (t_clk / 1000000u);
}

static clock l_bt_to_tick(uint pid, clock bt)
{
   clock t_clk = (bt * TIMER_CLK);

   ASSERT((bt != 0u && 
           (t_clk / bt) == TIMER_CLK && 
           (t_clk % BR) == 0u), pid);

   return (t_clk / BR);
}

void INIT_timer(uint pid)
{
   bool          b_ok = (pid < PHY_CNT);
   S_timer_state *p_timer;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(ADDR_OF(g_states.stations[pid].timers), 0u, SIZE_OF(g_states.stations[pid].timers));

      // xmit_max_timer: duration is 2 ms
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_MAX_TIMER - _TID_MAX_MIN]);
      p_timer->timeout = l_us_to_tick(pid, 2000u);

#if defined(SUPPORT_TIMER_STD)
      // STD: duration is 1 5B symbol
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_STD - _TID_MAX_MIN]);
#if ((BR % 4u) != 0u || (TIMER_CLK / (BR / 4u)) == 0u || (TIMER_CLK % (BR / 4u)) != 0u)
#error "Lack of precision"
#endif // if ((BR % 4u) != 0u || (TIMER_CLK / (BR / 4u)) == 0u || (TIMER_CLK % (BR / 4u)) != 0u)
      p_timer->timeout = (TIMER_CLK / (BR / 4u));
#endif // if defined(SUPPORT_TIMER_STD)

#if defined(SUPPORT_TIMER_RSCD)
      // RSCD: duration is 1 5B symbol
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_RSCD - _TID_MAX_MIN]);
#if ((BR % 4u) != 0u || (TIMER_CLK / (BR / 4u)) == 0u || (TIMER_CLK % (BR / 4u)) != 0u)
#error "Lack of precision"
#endif // if ((BR % 4u) != 0u || (TIMER_CLK / (BR / 4u)) == 0u || (TIMER_CLK % (BR / 4u)) != 0u)
      p_timer->timeout = (TIMER_CLK / (BR / 4u));
#endif // if defined(SUPPORT_TIMER_RSCD)

      // unjab_timer: duration is 16 ms
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_UNJAB - _TID_MAX_MIN]);
      p_timer->timeout = l_us_to_tick(pid, 16000u);

      // PLCA control beacon_timer: duration is 20 bit times (2.0 us)
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAC_BEACON - _TID_MAX_MIN]);
      p_timer->timeout = l_bt_to_tick(pid, 20u);

      // PLCA control beacon_det_timer: duration is 22 bit times (2.2 us)
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAC_BEACON_DET - _TID_MAX_MIN]);
      p_timer->timeout = l_bt_to_tick(pid, 22u);

      // PLCA control invalid_beacon_timer: duration is 4000 ns
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAC_INVALID_BEACON - _TID_MAX_MIN]);
      p_timer->timeout = l_us_to_tick(pid, 4u);

      // PLCA control burst_timer: duration is 128 bit times (12.8 us)
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAC_BURST - _TID_MAX_MIN]);
      p_timer->timeout = l_bt_to_tick(pid, 128u);

      // PLCA control to_timer: duration is 24 bit times (2.4 us)
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAC_TO - _TID_MAX_MIN]);
      p_timer->timeout = l_bt_to_tick(pid, 24u);

      // PLCA data commit_timer: duration is 288 bit times (28.8 us)
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAD_COMMIT - _TID_MAX_MIN]);
      p_timer->timeout = l_bt_to_tick(pid, 288u);

      // PLCA data pending_timer: duration is 512 bit times (51.1 us)
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAD_PENDING - _TID_MAX_MIN]);
      p_timer->timeout = l_bt_to_tick(pid, 512u);

      // PLCA status plca_status_timer: duration is 2*((to_timer*plca_node_count)+beacon_timer)
      p_timer = ADDR_OF(g_states.stations[pid].timers[TID_PLCAS_PLCA_STATUS - _TID_MAX_MIN]);
      p_timer->timeout = l_bt_to_tick(pid, (2u * ((24u * PHY_CNT) + 20u)));
   }
}

void CLK_timer(uint pid)
{
   bool          b_ok = (pid < PHY_CNT);
   clock         t_cnt, t_to;
   S_timer_state *p_timer;
   uint          i;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      p_timer = ADDR_OF(g_states.stations[pid].timers[0]);

      for (i = 0u; i < ELEMS_OF(g_states.stations[pid].timers); p_timer++, i++)
      {
         if ((t_cnt = p_timer->counter) != 0u)
         {
            t_to = p_timer->timeout;

            ASSERT((t_to != 0u && 
                    t_to < CAST_TO(-1, clock) && 
                    t_cnt <= (t_to + 1u)), pid);

            /*p_timer->counter = ((t_cnt <= t_to) ? 
                                (t_cnt + 1u) : 0u);*/

            if (t_cnt <= t_to)
            {
               p_timer->counter = (++t_cnt);

#if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 2u)
               if (t_cnt > t_to)
               {
                  print(DEBUG_TIMER, pid, DEBUG_TIMER_P, "TIM: %s_expired", lC_names[i]);
               }
#endif // if (defined(DEBUG_TIMER) && DEBUG_TIMER_LEVEL >= 2u)
            }
         }
      }
   }
}
