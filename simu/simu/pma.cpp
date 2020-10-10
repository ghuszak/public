#include "states.h"

void INIT_pma(uint pid)
{
   S_pma_state *p_curr = ADDR_OF(g_states.stations[pid].pma);
   bool        b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));
   }
}

void RESET_pma(uint pid, bool b_assert)
{
   bool b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      g_states.stations[pid].pma.b_reset = b_assert;
   }
}
