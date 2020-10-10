#include "states.h"

#if (defined(_WIN32) || defined(_WIN64))
#include <conio.h>
#include <windows.h>
#else // if (defined(_WIN32) || defined(_WIN64))
#include <inttypes.h>
#include <ncurses.h>
#endif // else if (defined(_WIN32) || defined(_WIN64))

typedef struct s_mac_simu_state 
{
   uint                    rx_id;
   uint                    tx_id;

   uint                    tx_cnt;
   uint                    tx_idx;
} S_mac_simu_state;

static S_mac_simu_state    l_states[PHY_CNT];

void INIT_mac_simu(uint pid)
{
   memset(ADDR_OF(l_states), 0u, SIZE_OF(l_states));
}

bool CLK_mac_simu(uint pid)
{
#if (!defined(_WIN32) && !defined(_WIN64))
   int l_key = getch();
#endif // if (!defined(_WIN32) && !defined(_WIN64))
   int t_key = 
#if (defined(_WIN32) || defined(_WIN64))
               (_kbhit() ? _getch() : 0);
#else // if (defined(_WIN32) || defined(_WIN64))
               ((l_key != ERR) ? l_key : 0);
#endif // else if (defined(_WIN32) || defined(_WIN64))
   uint  u_key = ((t_key >= 0) ? CAST_TO(t_key, uint) : 0u);
   bool  b_ok = (pid < PHY_CNT);
   bool  b_retval = true;
   sym4b t_data;

   ASSERT(b_ok, pid);

   if (b_ok)
   {
      // Execution is stopped by ESC on Windows
      if (u_key == 27u)
      {
         b_retval = false;
      }
      else
      {
         if (u_key >= '1')
         {
            if ((u_key -= '1') < ELEMS_OF(l_states) && 
                l_states[u_key].tx_cnt == 0u)
            {
               l_states[u_key].tx_cnt = MAC_SIMU_LEN;
               l_states[u_key].tx_idx = 0u;
               l_states[u_key].tx_id = 0u;
            }
         }

         if (l_states[pid].tx_cnt != 0u)
         {
            if (l_states[pid].tx_idx < l_states[pid].tx_cnt)
            {
               switch (l_states[pid].tx_idx++)
               {
               // JJ: SYNC+SYNC
               case 0u:
                  g_states.stations[pid].mac_pls.mii_tx.b_en = true;
                  //break;

               case 1u:
                  t_data = 14u;
                  print(stdout, pid, 0u, ">>>> %u: J (%u)", (++l_states[pid].tx_id), t_data);
                  break;

               // HH: SSD+SSD
               case 2u:
               case 3u:
                  t_data = 13u;
                  print(stdout, pid, 0u, ">>>> %u: H (%u)", (++l_states[pid].tx_id), t_data);
                  break;

               // 5*5: normal preamble (scrambler syncing part)
               case 4u:
               case 5u:
               case 6u:
               case 7u:
               case 8u:
                  t_data = 12u;
                  print(stdout, pid, 0u, ">>>> %u: [P] (%u)", (++l_states[pid].tx_id), t_data);
                  break;

               // 5*5: normal preamble (scrambler synced part)
               case 9u:
               case 10u:
               case 11u:
               case 12u:
               case 13u:
                  t_data = 5u;
                  print(stdout, pid, 0u, ">>>> %u: P (%u)", (++l_states[pid].tx_id), t_data);
                  break;

               // 1*5: normal preamble (for 802.3br)
               case 14u:
                  t_data = 5;
                  print(stdout, pid, 0u, ">>>> %u: <P> (%u)", (++l_states[pid].tx_id), t_data);
                  break;

               // 1*D SOF (and for 802.3br)
               case 15u:
                  t_data = 13;
                  print(stdout, pid, 0u, ">>>> %u: SOF (%u)", (++l_states[pid].tx_id), t_data);
                  break;

               default:
                  t_data = ((l_states[pid].tx_idx == 17u) ? 0u : 
                            ((g_states.stations[pid].mac_pls.mii_tx.d + 1u) & 0x0fu));

                  print(stdout, pid, 0u, ">>>> %u: %u", (++l_states[pid].tx_id), t_data);
                  break;
               }

               g_states.stations[pid].mac_pls.mii_tx.d = t_data;
            }
            else
            {
               g_states.stations[pid].mac_pls.mii_tx.b_en = false;
               l_states[pid].tx_cnt = 0u;
            }
         }
      }
   }

   if (pid < PHY_CNT && g_states.stations[pid].MII_RX.b_dv)
   {
      print(stdout, pid, 0u, "<<<< %u: %u", (++l_states[pid].rx_id), 
            (g_states.stations[pid].MII_RX.d & 15u));
   }

   return b_retval;
}
