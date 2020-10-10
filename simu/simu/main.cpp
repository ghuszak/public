// TODO: fill-in correctly when 3.1 is implemented
// Based on 2019 March 22, 1st Sponsor Ballot: IEEE P802.3cg/D3.0 (IEEE_P8023cg_D3p0.pdf)
#include "states.h"

#if (defined(_WIN32) || defined(_WIN64))
#include <Windows.h>
#else // if (defined(_WIN32) || defined(_WIN64))
#include <inttypes.h>
#include <ncurses.h>
#endif // else if (defined(_WIN32) || defined(_WIN64))

static bool                lb_error;
static clock               l_clk_print_last[PHY_CNT + 2u];
static uint64              l_print_idx;
static clock               l_clk;

#if (defined(_WIN32) || defined(_WIN64))
#define F_CLK              "llu"

static LARGE_INTEGER       l_pc_print_first;
static LARGE_INTEGER       l_pf;

#if !defined(OUTPUT_LINUX_LIKE)
static LARGE_INTEGER       l_pc_print_last;
#endif // if !defined(OUTPUT_LINUX_LIKE)
#else // if (defined(_WIN32) || defined(_WIN64))
#define F_CLK              PRIu64
#endif // else if (defined(_WIN32) || defined(_WIN64))

void print(FILE *f, uint pid, uint pid_filter, const char *p_str, ...)
{
   uint            i, j;
   va_list         p_args;
   clock           *p_clk = NULL;
#if (!defined(OUTPUT_LINUX_LIKE) && (defined(_WIN32) || defined(_WIN64)))
   LARGE_INTEGER   t_c, t_d, t_t1, t_t2;
   bool            b_qpc = QueryPerformanceCounter(ADDR_OF(t_c));
#endif // if (!defined(OUTPUT_LINUX_LIKE) && (defined(_WIN32) || defined(_WIN64)))

   if (pid_filter == 0u || pid == (pid_filter - 1u))
   {
      if (pid < PHY_CNT)
      {
         p_clk = ADDR_OF(l_clk_print_last[pid + 1u]);

         fprintf(f, "pid=%u\t", (pid + 1u));
      }
      else
      {
         if (pid == PHY_CNT)
         {
            p_clk = ADDR_OF(l_clk_print_last[0]);

            fprintf(f, "-----\t");
         }
         else
         {
            p_clk = ADDR_OF(l_clk_print_last[PHY_CNT + 1u]);

            fprintf(f, "<%03u>\t", pid);
         }
      }

      fprintf(f, "%020" F_CLK "\tclk=%020" F_CLK "\tdclk=%020" F_CLK "\t", 
              (++l_print_idx), l_clk, (l_clk - (*p_clk)));
      (*p_clk) = l_clk;

#if (!defined(OUTPUT_LINUX_LIKE) && (defined(_WIN32) || defined(_WIN64)))
      if (b_qpc)
      {
         t_c.QuadPart -= l_pc_print_first.QuadPart;

         t_d.QuadPart = (t_c.QuadPart - l_pc_print_last.QuadPart);
         t_t1.QuadPart = (t_c.QuadPart * 1000000u);
         t_t2.QuadPart = (t_d.QuadPart * 1000000u);
         l_pc_print_last = t_c;

         if (t_t1.QuadPart >= t_c.QuadPart && 
             t_t2.QuadPart >= t_d.QuadPart)
         {
            fprintf(f, "us=%020llu\tdus=%020llu\t", 
                    (t_t1.QuadPart / l_pf.QuadPart), 
                    (t_t2.QuadPart / l_pf.QuadPart));
         }
      }
#endif // if (!defined(OUTPUT_LINUX_LIKE) && (defined(_WIN32) || defined(_WIN64)))

      j = ((pid < PHY_CNT) ? (pid + 1u) : 0u);
      i = 0u;

      while (i < j)
      {
         fprintf(f, " ");
         i++;
      }

      va_start(p_args, p_str);
      vfprintf(f, p_str, p_args);
      va_end(p_args);

      fprintf(f, "\r\n");
   }
}

bool _assert(uint expression, uint pid, const char *p_func, const char *p_file, int line)
{
   bool b_retval = (expression != 0u);

   if (!b_retval)
   {
      const char *p_bs = strrchr(p_file, '\\');
      const char *p_fs = strrchr(p_file, '/');
      const char *p_f = ((p_bs != NULL) ? 
                         ((p_fs != NULL) ? 
                          ((p_bs > p_fs) ? p_bs : p_fs) : p_bs) : 
                         p_fs);

      print(stderr, pid, 0u, "ASSERT FAILED: in %s(), see <%s> line %u", p_func, (p_f + 1u), line);
      lb_error = true;
   }

   return b_retval;
}

void l_init(void)
{
   uint pid;

   lb_error = false;
   memset(ADDR_OF(l_clk_print_last), 0u, SIZE_OF(l_clk_print_last));
   l_print_idx = 0u;
   l_clk = 0u;

#if (!defined(_WIN32) && !defined(_WIN64))
   initscr();
   cbreak();
   noecho();
   scrollok(stdscr, TRUE);
   nodelay(stdscr, TRUE);
#endif // if (!defined(_WIN32) && !defined(_WIN64))

   INIT_random();
   INIT_states();
   INIT_channel();

   for (pid = 0u; pid < PHY_CNT; pid++)
   {
      INIT_timer(pid);

      INIT_pma(pid);
      INIT_pma_tx(pid);
      INIT_pma_rx(pid);

      INIT_pcs(pid);
      INIT_pcs_tx(pid);
      INIT_pcs_rx(pid);

      INIT_plca(pid, PLCA_BURST_COUNT);
      INIT_plca_ctrl(pid);
      INIT_plca_data(pid);
      INIT_plca_status(pid);

      INIT_mac_pls(pid);

      INIT_mac_simu(pid);
   }
}

int main(int argc, char *argv[], char **envp)
{
   bool          b_reset, b_mii_clk, b_pmcd;
   bool          lb_running = true;
   clock         t_mod;
   uint          pid;
#if (defined(_WIN32) || defined(_WIN64))
   LARGE_INTEGER t_li;
#endif // if (defined(_WIN32) || defined(_WIN64))

#if (defined(_WIN32) || defined(_WIN64))
   lb_error = (!QueryPerformanceCounter(ADDR_OF(l_pc_print_first)) || 
               !QueryPerformanceFrequency(ADDR_OF(l_pf)) || 
               l_pf.QuadPart == 0u);
#endif // if (defined(_WIN32) || defined(_WIN64))

   l_init();

   MSG_OK(PHY_CNT, "<BEGIN>");

   while (!lb_error && lb_running)
   {
      for (pid = 0u; pid < PHY_CNT; pid++)
      {
         b_reset = (l_clk == 0u);

         t_mod = (l_clk % (2u * 5u));
         b_mii_clk = (t_mod == 3u);
         b_pmcd = (t_mod == 1u);

         if (b_reset)
         {
            RESET_pma(pid, true);
            RESET_pcs(pid, true);
            RESET_plca(pid, true);
         }
         else
         {
            if (b_mii_clk)
            {
               lb_running = CLK_mac_simu(pid);
            }
         }

         CLK_timer(pid);

         CLK_pma_tx(pid);
         CLK_pma_rx(pid);

         CLK_plca(pid);
         CLK_plca_ctrl(pid, b_mii_clk, b_pmcd);
         CLK_plca_data(pid, b_mii_clk);
         CLK_plca_status(pid);

         if (b_reset || b_mii_clk)
         {
            CLK_pcs_tx(pid);
            CLK_pcs_rx(pid);
         }

         if (b_reset)
         {
            RESET_pma(pid, false);
            RESET_pcs(pid, false);
            RESET_plca(pid, false);
         }
      }

#if defined(_DEBUG)
      // 100: ad-hoc value (can be adjusted, as needed)
      if (b_reset || (l_clk % (100u)) == 0u)
      {
         CHECK_states();
      }
#endif // #if defined(_DEBUG)

      if ((l_clk++) >= CAST_TO(-2, clock))
      {
         lb_running = false;
      }
   }

   if (!lb_error)
   {
      MSG_OK(PHY_CNT, "<QUIT-SIGNAL>");
   }

   MSG_OK(PHY_CNT, "<END>");

#if (defined(_WIN32) || defined(_WIN64))
   if (QueryPerformanceCounter(ADDR_OF(t_li)) && 
       t_li.QuadPart >= l_pc_print_first.QuadPart)
   {
      t_li.QuadPart -= l_pc_print_first.QuadPart;
      t_li.QuadPart /= l_pf.QuadPart;

      printf("Total runtime %" F_CLK " days, %" F_CLK " hours, %" F_CLK " minutes and %" F_CLK " seconds\n", 
             (t_li.QuadPart / (60u * 60u * 24u)), 
             ((t_li.QuadPart / (60u * 60u)) % 24u), 
             ((t_li.QuadPart / 60u) % 60u), 
             (t_li.QuadPart % 60u));
   }
#endif // if (defined(_WIN32) || defined(_WIN64))

   return (lb_error ? -1 : 0);
}
