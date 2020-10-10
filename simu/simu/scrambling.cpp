#include "states.h"

static uint l_seeds[PHY_CNT];
static bool l_init[PHY_CNT];

#if defined(SUPPORT_SCRAMBLING)
void INIT_scrambling(uint pid)
{
   S_scrambling_state *p_state = ADDR_OF(g_states.stations[pid].scrambling);
   bool               b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_state, 0u, SIZE_OF(*p_state));

      l_seeds[pid] = random_get(15u);
      l_init[pid] = true;
   }
}

void RESET_scrambling(uint pid)
{
   S_scrambling_state *p_state = ADDR_OF(g_states.stations[pid].scrambling);
   bool               b_ok = (pid < PHY_CNT && l_init[pid]);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      p_state->s_lfsr = l_seeds[pid];
      ASSERT((p_state->s_lfsr != 0u), pid);

      p_state->d_lfsr = 0u;
   }
}

sym4b scramble(uint pid, sym4b data)
{
   S_station_states *p_states = ADDR_OF(g_states.stations[pid]);
   bool             b_ok = (pid < PHY_CNT && 
                            !p_states->pcs.b_reset && 
                            l_init[pid]);
   sym4b            retval = 0u;
   uint             t_lfsr;
#if !defined(SUPPORT_SCRAMBLING_OPTIMIZATION)
   bool             b_set;
   uint             i;
#endif // if !defined(SUPPORT_SCRAMBLING_OPTIMIZATION)

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      // Scrambling is LSB first
      t_lfsr = p_states->scrambling.s_lfsr;

#if defined(SUPPORT_SCRAMBLING_OPTIMIZATION)
      retval = ((data ^ (t_lfsr ^ (t_lfsr >> 3u))) & 0x0fu);
      t_lfsr >>= 4u;
      t_lfsr |= (retval << (17u - 4u));
#else // if defined(SUPPORT_SCRAMBLING_OPTIMIZATION)
      for (i = 0u; i < 4u; i++)
      {
         b_set = (((data ^ (t_lfsr ^ (t_lfsr >> 3u))) & 1u) != 0u);
         t_lfsr >>= 1u;
         data >>= 1u;

         if (b_set)
         {
            retval |= (1u << i);
            t_lfsr |= 0x10000u;
         }
      }
#endif // else if defined(SUPPORT_SCRAMBLING_OPTIMIZATION)

      p_states->scrambling.s_lfsr = t_lfsr;

#if defined(DEBUG_SCRAMBLING)
      print(DEBUG_SCRAMBLING, pid, DEBUG_SCRAMBLING_P, 
            "SCRAMBLE(%u)==%u", data, retval);
#endif // if defined(DEBUG_SCRAMBLING)
   }

   return retval;
}

sym4b descramble(uint pid, sym4b data)
{
   S_station_states *p_states = ADDR_OF(g_states.stations[pid]);
   bool             b_ok = (pid < PHY_CNT && 
                            !p_states->pcs.b_reset && 
                            l_init[pid]);
   sym4b            retval = 0u;
   uint             t_lfsr;
#if !defined(SUPPORT_SCRAMBLING_OPTIMIZATION)
   uint             i;
#endif // if !defined(SUPPORT_SCRAMBLING_OPTIMIZATION)

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      t_lfsr = p_states->scrambling.d_lfsr;

#if defined(SUPPORT_SCRAMBLING_OPTIMIZATION)
      retval = ((data ^ (t_lfsr ^ (t_lfsr >> 3u))) & 0x0fu);
      t_lfsr >>= 4u;
      t_lfsr |= (data << (17u - 4u));
#else // if defined(SUPPORT_SCRAMBLING_OPTIMIZATION)
      for (i = 0u; i < 4u; i++)
      {
         if (((data ^ (t_lfsr ^ (t_lfsr >> 3u))) & 1u) != 0u)
         {
            retval |= (1u << i);
         }

         t_lfsr >>= 1u;

         if ((data & 1u) != 0u)
         {
            t_lfsr |= 0x10000u;
         }

         data >>= 1u;
      }
#endif // else if defined(SUPPORT_SCRAMBLING_OPTIMIZATION)

      p_states->scrambling.d_lfsr = t_lfsr;

#if defined(DEBUG_SCRAMBLING)
      print(DEBUG_SCRAMBLING, pid, DEBUG_SCRAMBLING_P, "DESCRAMBLE(%u)==%u", data, retval);
#endif // if defined(DEBUG_SCRAMBLING)
   }

   return retval;
}
#endif // if defined(SUPPORT_SCRAMBLING)
