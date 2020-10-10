#include "states.h"

void INIT_mac_pls(uint pid)
{
   bool b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(ADDR_OF(g_states.stations[pid].mac_pls), 0u, 
             SIZE_OF(g_states.stations[pid].mac_pls));
   }
}
