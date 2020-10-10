#include "states.h"

void INIT_pcs(uint pid)
{
   S_pcs_state *p_state = ADDR_OF(g_states.stations[pid].pcs);
   bool        b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_state, 0u, SIZE_OF(*p_state));
   }

#if defined(SUPPORT_SCRAMBLING)
   INIT_scrambling(pid);
#endif // if defined(SUPPORT_SCRAMBLING)
}

void RESET_pcs(uint pid, bool b_assert)
{
   bool b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok && g_states.stations[pid].pcs.b_reset != b_assert)
   {
#if defined(SUPPORT_SCRAMBLING)
      if (b_assert)
      {
         RESET_scrambling(pid);
      }
#endif // if defined(SUPPORT_SCRAMBLING)

      g_states.stations[pid].pcs.b_reset = b_assert;
   }
}
