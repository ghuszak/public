#include "states.h"

static void l_cmd_detect(uint pid, S_plca_state *p_plca, const S_pcs_rx_state *p_pcs_rx)
{
#if (PLCA_EN == 1u)
   const S_mii_signals_rx *p_mii_s = ADDR_OF(p_pcs_rx->mii_rx);
   E_plca_rtx_cmd         rx_cmd = PRTC_NONE;
   bool                   b_commit = false;
   bool                   b_dv;
   sym4b                  d;

   b_dv = p_mii_s->b_dv;

   if (!b_dv && 
       p_mii_s->b_er)
   {
      d = p_mii_s->d;
      b_commit = (d == 3u);

      if (b_commit || d == 2u)
      {
         rx_cmd = (b_commit ? PRTC_COMMIT : PRTC_BEACON);
      }
      else
      {
         ASSERT(false, pid);
      }
   }

   p_plca->b_receiving = (b_dv || b_commit);
   p_plca->rx_cmd = rx_cmd;

   p_plca->mii_rx.b_dv = p_mii_s->b_dv;
   p_plca->mii_rx.d = p_mii_s->d;
#else // if (PLCA_EN == 1u)
   p_plca->mii_rx = p_pcs_rx->mii_rx;
#endif // else if (PLCA_EN == 1u)
}

void INIT_plca(uint pid, uint8 max_bc)
{
   S_plca_state *p_curr = ADDR_OF(g_states.stations[pid].plca);
   bool         b_ok = (pid < PHY_CNT && pid <= 255u);

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      memset(p_curr, 0u, SIZE_OF(*p_curr));

      p_curr->node_id = CAST_TO(pid, uint8);
      p_curr->max_bc = max_bc;
   }
}

void RESET_plca(uint pid, bool b_assert)
{
   S_plca_state     *p_curr = ADDR_OF(g_states.stations[pid].plca);
   bool             b_ok = (pid < PHY_CNT);
   S_plca_state     t_state;

   ASSERT(b_ok, pid);

   if (b_ok && p_curr->b_reset != b_assert)
   {
      t_state = (*p_curr);

      if (b_assert)
      {
         memset(p_curr, 0u, SIZE_OF(*p_curr));

         p_curr->node_id = t_state.node_id;
         p_curr->max_bc = t_state.max_bc;
      }

      p_curr->b_reset = b_assert;
   }
}

void CLK_plca(uint pid)
{
   S_station_states *p_states = ADDR_OF(g_states.stations[pid]);
   bool             b_ok = (pid < PHY_CNT);

   ASSERT(b_ok, pid);

   if (!p_states->plca.b_reset)
   {
      l_cmd_detect(pid, ADDR_OF(p_states->plca), 
                   ADDR_OF(p_states->pcs_rx));
   }
}
