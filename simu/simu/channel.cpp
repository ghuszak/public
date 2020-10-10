#include "states.h"

static bool l_get_state(uint pid)
{
   uint found = 0u;
   bool b_retval;
   uint i = 0u;

   while (i < PHY_CNT)
   {
      switch (g_states.tx_drivers.bi_d[i])
      {
      case PMS_Z:
         break;

      case PMS_0:
         b_retval = false;
         found++;
         break;

      case PMS_1:
         b_retval = true;
         found++;
         break;

      default:
         ASSERT(0u, pid);
         break;
      }

      i++;
   }

   ASSERT((found == 1u), pid);

   return ((found == 1u) ? b_retval : false);
}

void INIT_channel(void)
{
   uint i;

   memset(ADDR_OF(g_states.tx_drivers), 0u, SIZE_OF(g_states.tx_drivers));

   for (i = 0u; i < PHY_CNT; i++)
   {
      g_states.tx_drivers.bi_d[i] = PMS_Z;
   }
}

void channel_write(uint pid, E_pma_state state)
{
   bool                b_ok = (pid < PHY_CNT && state >= _PMS_MIN && state <= _PMS_MAX);
   S_tx_drivers_states *p_curr = ADDR_OF(g_states.tx_drivers);
   bool                b_z_curr, b_z_new, b_high;
   bool                b_z = (state == PMS_Z);
   uint                cnt_curr, cnt_new;
   E_pma_state         state_curr;

   ASSERT(b_ok, pid);

   if (b_ok && state != (state_curr = p_curr->bi_d[pid]))
   {
      ASSERT((state_curr >= _PMS_MIN && state_curr <= _PMS_MAX), pid);

#if defined(DEBUG_CHANNEL_W)
      print(DEBUG_CHANNEL_W, pid, DEBUG_CHANNEL_W_P, "CHW: %s", 
            ((state == PMS_Z) ? 
             "Z" : 
             ((state == PMS_1) ? 
              "1" : 
              ((state == PMS_0) ? 
               "0" : 
               "?"))));
#endif // if defined(DEBUG_CHANNEL_W)

      p_curr->bi_d[pid] = state;

      b_z_curr = (state_curr == PMS_Z);
      b_z_new = (b_z != b_z_curr);
      b_high = false;

      if (b_z_new)
      {
         cnt_curr = p_curr->active_count;
         cnt_new = cnt_curr;

         if (b_z)
         {
            ASSERT((cnt_new != 0u), pid);

            if (cnt_new != 0u)
            {
               cnt_new--;
            }
         }
         else
         {
            ASSERT((cnt_new < PHY_CNT), pid);

            if (cnt_new < PHY_CNT)
            {
               cnt_new++;
            }
         }

         if (cnt_curr != cnt_new)
         {
            if ((cnt_curr > 1u) != (cnt_new > 1u))
            {
               if (cnt_curr <= 1u)
               {
                  MSG_ERR(pid, "BEGIN PHY-COL");
               }
               else
               {
                  MSG_OK(pid, "END PHY-COL");

                  b_high = l_get_state(pid);
               }
            }
            else
            {
               if (cnt_new == 1u && cnt_curr == 0u)
               {
                  b_high = (state == PMS_1);
               }
            }

            p_curr->active_count = cnt_new;
         }
      }
      else
      {
         if (!b_z_curr)
         {
            b_high = (state == PMS_1);
         }
      }

      p_curr->b_high = b_high;
   }
}

E_pma_state channel_read(uint pid, bool *pb_col)
{
   bool        b_col = false;
   E_pma_state retval;

   switch (g_states.tx_drivers.active_count)
   {
   case 0u:
      // When no drivers are active, read 0
      retval = PMS_Z;
      break;

   case 1u:
      retval = (g_states.tx_drivers.b_high ? PMS_1 : PMS_0);
      break;

   default:
      // In case of physical collision, return random value
      retval = ((random_get(1u) != 0u) ? PMS_1 : PMS_0);
      b_col = true;
      break;
   }

   (*pb_col) = b_col;

#if defined(DEBUG_CHANNEL_R)
   print(DEBUG_CHANNEL_R, pid, DEBUG_CHANNEL_R_P, 
         "CHR: %s (%s)", 
         ((retval == PMS_Z) ? "Z" : 
          ((retval == PMS_1) ? "1" : "0")), 
         (b_col ? "1" : "0"));
#endif // if defined(DEBUG_CHANNEL_R)

   return retval;
}
