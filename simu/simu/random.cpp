#include "states.h"

#include <stdlib.h>

#if (RAND_MAX == 0x7fffu)
#define RAND_BITS          15u
#elif (RAND_MAX == 0xffffu)
#define RAND_BITS          16u
#elif (RAND_MAX == 0x7fffffffu)
#define RAND_BITS          31u
#elif (RAND_MAX == 0xffffffffu)
#define RAND_BITS          32u
#elif (RAND_MAX == 0x7ffffffffffffffflu)
#define RAND_BITS          63u
#elif (RAND_MAX == 0xfffffffffffffffflu)
#define RAND_BITS          64u
#else // if (RAND_MAX == 0x8000u)
#error "Set RAND_BITS according to the value of RAND_MAX"
#endif // else if (RAND_MAX == ..)

static uint64              l_store;
static uint                l_avail;

void INIT_random(void)
{
   srand(CAST_TO(RAND_SEED, uint));

   l_store = 0u;
   l_avail = 0u;
}

uint random_get(uint bit_count)
{
   uint   avail = l_avail;
   uint   retval = 0u;
   uint64 t_uint64;
   uint   t_uint;

   ASSERT((bit_count > 0u && bit_count <= 32u), 0u);

   while (bit_count > 0u)
   {
      while (avail <= (64u - RAND_BITS))
      {
         t_uint64 = CAST_TO(rand(), uint64);

         l_store |= (t_uint64 << avail);
         avail += RAND_BITS;
      }

      t_uint = ((avail <= bit_count) ? avail : bit_count);
      bit_count -= t_uint;
      avail -= t_uint;

      retval <<= t_uint;
      retval |= (l_store & ((1u << t_uint) - 1u));
      l_store >>= t_uint;
   }

   l_avail = avail;

   return retval;
}
