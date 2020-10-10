#include "states.h"

#include <stdlib.h>

static const sym5b lC_4b_to_5b[16] = 
{
   S5B_0, 
   S5B_1, 
   S5B_2, 
   S5B_3, 
   S5B_4, 
   S5B_5, 
   S5B_6, 
   S5B_7, 
   S5B_8, 
   S5B_9, 
   S5B_A, 
   S5B_B, 
   S5B_C, 
   S5B_D, 
   S5B_E, 
   S5B_F
};

static const sym4b lC_5b_to_4b[32] = 
{
   0x10u, 
   0x10u, 
   0x10u, 
   0x10u, 
   0x10u, 
   0x10u, 
   0x10u, 
   0x10u, 
   0x10u, 
   1u, 
   4u, 
   5u, 
   0x10u, 
   0x10u, 
   6u, 
   7u, 
   0x10u, 
   0x10u, 
   8u, 
   9u, 
   2u, 
   3u, 
   10u, 
   11u, 
   0x10u, 
   0x10u, 
   12u, 
   13u, 
   14u, 
   15u, 
   0u, 
   0x10u
};

static const bool lC_5b_valid[32] = 
{
   false, 
   false, 
   false, 
   false, 
   true, 
   false, 
   false, 
   true, 
   true, 
   true, 
   true, 
   true, 
   false, 
   true, 
   true, 
   true, 
   false, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true, 
   true
};

static const char *lC_5b_names[32] = 
{
   "<0>", 
   "<1>", 
   "<2>", 
   "<3>", 
   "SSD", 
   "<5>", 
   "<6>", 
   "ESDOK_ESDBRS", 
   "BEACON", 
   "d1", 
   "d4", 
   "d5", 
   "<12>", 
   "ESD_HB", 
   "d6", 
   "d7", 
   "<16>", 
   "ESDERR", 
   "d8", 
   "d9", 
   "d2", 
   "d3", 
   "d10", 
   "d11", 
   "SYNC_COMMIT", 
   "ESDJAB", 
   "d12", 
   "d13", 
   "d14", 
   "d15", 
   "d0", 
   "SILENCE"
};

sym4b conv_5b_to_4b(uint pid, sym5b sym, bool *p_valid)
{
   bool  b_valid = (sym < ELEMS_OF(lC_5b_to_4b));
   sym4b t_sym;

   ASSERT(b_valid, pid);

   if (b_valid)
   {
      t_sym = lC_5b_to_4b[sym];

      if (t_sym > 0x0fu)
      {
         b_valid = false;
      }
   }

   if (p_valid)
   {
      (*p_valid) = b_valid;
   }

   return (b_valid ? t_sym : 0u);
}

const char *get_name_5b(uint pid, sym5b sym)
{
   const char *p_retval = NULL;

   if (sym >= _S5B_MIN)
   {
      sym -= _S5B_MIN;

      if (sym < ELEMS_OF(lC_5b_names))
      {
         p_retval = lC_5b_names[sym];
      }
   }

   if (p_retval == NULL)
   {
      p_retval = "?";
   }

   return p_retval;
}

sym5b conv_4b_to_5b(uint pid, sym4b sym)
{
   bool b_valid = (sym < ELEMS_OF(lC_4b_to_5b));

   ASSERT(b_valid, pid);

   return (b_valid ? lC_4b_to_5b[sym & 0x0fu] : _S5B_MIN);
}

bool is_5b_valid(sym5b sym5b)
{
   return (sym5b < ELEMS_OF(lC_5b_valid) && lC_5b_valid[sym5b]);
}
