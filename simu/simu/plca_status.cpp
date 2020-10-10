#include "states.h"

#if defined(DEBUG_PLCA_STATUS)
static const char *lC_names[_PSS_CNT] =
{
   "INACTIVE", 
   "ACTIVE", 
   "HYSTERESIS"
};

static const char *l_get_name(uint pid, uint E_plca_status_state)
{
   const char *p_retval = "?";
   uint       t_idx;

   if (state == _PSS_UNDEFINED)
   {
      p_retval = "<N/A>";
   }
   else
   {
      if (state >= _PSS_MIN && 
          (t_idx = (state - _PSS_MIN)) < ELEMS_OF(lC_names))
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
#endif // if defined(DEBUG_PLCA_STATUS)

static void l_set_state(uint pid, S_plca_status_state *p_curr, E_plca_status_state state)
{
#if defined(DEBUG_PLCA_STATUS)
   if (p_curr->state != state)
   {
      print(DEBUG_PLCA_STATUS, pid, DEBUG_PLCA_STATUS_P, "PLCA_STATUS: %s->%s",
            l_get_name(pid, p_curr->state), l_get_name(pid, state));
   }
#endif // if defined(DEBUG_PLCA_STATUS)

   switch (p_curr->state = state)
   {
   case PSS_INACTIVE:
      p_curr->b_ok = false;
      break;

   case PSS_ACTIVE:
      p_curr->b_ok = true;
      break;

   case PSS_HYSTERESIS:
      timer_start(pid, TID_PLCAS_PLCA_STATUS, false);
      break;

   default:
      ASSERT(0u, pid);
      break;
   }
}

void INIT_plca_status(uint pid)
{
   S_plca_status_state *p_curr = ADDR_OF(g_states.stations[pid].plca_status);
   bool                b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void CLK_plca_status(uint pid)
{
   S_station_states       *p_states = ADDR_OF(g_states.stations[pid]);
   S_plca_status_state    *p_curr = ADDR_OF(p_states->plca_status);
   const S_plca_state     *p_plca = ADDR_OF(p_states->plca);
   E_plca_status_state    new_state = _PSS_UNDEFINED;
   bool                   b_ok = (pid < PHY_CNT);
   bool                   b_en;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
#if (PLCA_EN == 1u)
      b_en = true;
#else // if (PLCA_EN == 1u)
      b_en = false;
#endif // else if (PLCA_EN == 1u)

      if (p_plca->b_reset || 
          !b_en)
      {
         if (!p_curr->b_reset_prev)
         {
            p_curr->b_reset_prev = true;
            new_state = PSS_INACTIVE;

            timer_stop(pid, TID_PLCAS_PLCA_STATUS);
         }
      }
      else
      {
         p_curr->b_reset_prev = false;

         switch (p_curr->state)
         {
         case PSS_INACTIVE:
            if (p_plca->b_active)
            {
               new_state = PSS_ACTIVE;
            }
            break;

         case PSS_ACTIVE:
            if (!p_plca->b_active)
            {
               new_state = PSS_HYSTERESIS;
            }
            break;

         case PSS_HYSTERESIS:
            if (p_plca->b_active)
            {
               new_state = PSS_ACTIVE;
            }
            else
            {
               if (timer_isdone(pid, TID_PLCAS_PLCA_STATUS))
               {
                  new_state = PSS_INACTIVE;
               }
            }
            break;

         default:
            ASSERT(0u, pid);
            break;
         }
      }

      if (new_state != _PSS_UNDEFINED)
      {
         l_set_state(pid, p_curr, new_state);
      }
   }
}
