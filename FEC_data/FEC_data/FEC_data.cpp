#include <memory.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>

#if defined(WIN32)
#include <Windows.h>
#endif // if defined(WIN32)

//#define DEBUG_PRINT
//#define DEBUG_DOC
//#define TEST

#define ELEMS_OF(v)     (SIZE_OF(v) / SIZE_OF((v)[0u]))
#define FATAL()         l_fatal(__LINE__)
#define SIZE_OF(s)      sizeof(s)
#define ADDR_OF(v)      (&(v))

typedef unsigned int    uint;

#if !defined(DEBUG_DOC)
#define P_K             (P_N - 2u)
// P_POLY_FIELD: field generator polynomial: 1+x^2+x^5
#define P_POLY_FIELD    0b100101
#define P_M             5u

// ERROR_TYPE:
// 0: no error
// 1: E(x)=23x^9
#define ERROR_TYPE      1u
#else // !defined(DEBUG_DOC)
#define P_K             (P_N - 4u)
// P_POLY_FIELD: field generator polynomial: 1+x+x^4
#define P_POLY_FIELD    0b10011
#define P_M             4u

// ERROR_TYPE:
// 0: no error
// 1: E(x)=13x^9+2x^2
// 2: E(x)=13x^10+2x^2
// 3: E(x)=13x^9 (single error)
// 4: E(x)=7x^9+2x^2 (error that causes S3 to be 0)
// 5: E(x)=7x+9
// 6: E(x)=13x^10+3x^9+2x^8+x^7+2x^2
#define ERROR_TYPE      1u
#define UNIT_TEST
#endif // else !defined(DEBUG_DOC)

// Sanity checks
#if (!defined(P_POLY_FIELD) || \
     P_POLY_FIELD <= 0u || \
     !defined(P_M) || \
     P_M <= 0u || \
     P_M > 15u || \
     (P_POLY_FIELD & (1u << P_M)) == 0u || \
     (P_POLY_FIELD & ((1u << P_M) - 1u)) == 0u || \
     (P_POLY_FIELD & (~((1u << (P_M + 1u)) - 1u))) != 0u)
#error "Something wrong with P_POLY_FIELD or with P_M"
#endif // P_POLY_FIELD and with P_M check

#define P_M2            (1u << P_M)
// We guarantee existence with this
#define P_N             (P_M2 - 1u)

// Sanity checks
#if (!defined(P_N) || \
     !defined(P_K) || \
     P_N <= 0u || \
     P_K <= 0u || \
     P_N <= P_K || \
     (P_N - P_K) < 2u)
#error "Something wrong with P_N or with P_K"
#else // P_N and P_K check
#define P_R             (P_N - P_K)
#define P_T             (P_R / 2u)
#endif // else P_N and P_K check

#if (!defined(RAND_MAX) || RAND_MAX < (1u << 5u))
#error "Incorrect RAND_MAX"
#endif // if (!defined(RAND_MAX) || RAND_MAX < (1u << 5u))

#if !defined(DEBUG_DOC)
// P_POLY_CODE: code generator polynomial: x^2+3x+2
const uint P_POLY_CODE[P_R + 1] = 
{
   1u, 3u, 2u
};
#else // !defined(DEBUG_DOC)
// P_POLY_CODE: code generator polynomial: x^4+15x^3+3x^2+x+12
const uint P_POLY_CODE[P_R + 1] = 
{
   1u, 15u, 3u, 1u, 12u
};
#endif // else !defined(DEBUG_DOC)

typedef struct s_di_sfs 
{
   uint                 first;
   uint                 a7;
} S_di_sfs;

static const uint lC_nfss[] = 
{
   0x01u, 
   0x02u, 
   0x03u, 
   0x04u, 
   0x05u, 
   0x06u, 
   0x08u, 
   0x09u, 
   0x0au, 
   0x0bu, 
   0x0cu, 
   0x0eu, 
   0x0fu, 
   0x10u, 
   0x11u, 
   0x12u, 
   0x13u, 
   0x14u, 
   0x15u, 
   0x16u, 
   0x17u, 
   0x18u, 
   0x19u, 
   0x1au, 
   0x1bu, 
   0x1cu, 
   0x1du, 
   0x1eu
};

static const uint lC_nfss_9[] = 
{
   /*0x00u, */
   0x02u, 
   0x04u, 
   0x08u, 
   0x0au, 
   0x0eu, 
   0x10u, 
   0x12u, 
   0x14u, 
   0x16u/*, 
   0x18u, 
   0x1au, 
   0x1cu*/
};

static const uint lC_nfss_6_00[] = 
{
   /*0x00u, */
   0x02u, 
   0x04u, 
   0x06u, 
   0x08u, 
   0x0au, 
   0x0cu/*, 
   0x0eu*/
};

static const uint lC_nfss_6_01[] = 
{
   0x11u, 
   0x13u, 
   0x15u, 
   0x17u, 
   0x19u, 
   0x1bu/*, 
   0x1du*/
};

static const uint lC_nfss_6_10[] = 
{
   0x01u, 
   0x03u, 
   0x05u, 
   0x09u, 
   0x0bu, 
   0x0fu
};

static const uint lC_nfss_6_11[] = 
{
   0x10u, 
   0x12u, 
   0x14u, 
   0x16u, 
   0x18u, 
   0x1au/*, 
   0x1cu*/
};

static const uint lC_fss[] = 
{
   // SILENCE (I): 11111
   0x1fu, 
   // ESD / HB (T): 01101
   0x0du, 
   // ESDOK / ESDBRS (R): 00111
   0x07u, 
   // Q: 00000
   0x00u
};

static const S_di_sfs lC_di_sfs[] = 
{
   {0u, 1u}, 
   {1u, 1u}, 
   {2u, 1u}, 
   {3u, 1u}, 
   {4u, 1u}, 
   {5u, 1u}, 
   {6u, 1u}, 
   {7u, 1u}, 

   {0u, 2u}, 
   {1u, 2u}, 
   {2u, 2u}, 
   {3u, 2u}, 
   {4u, 2u}, 
   {5u, 2u}, 
   {6u, 2u}, 

   {0u, 3u}, 
   {1u, 3u}, 
   {2u, 3u}, 
   {3u, 3u}, 
   {4u, 3u}, 
   {5u, 3u}, 

   {0u, 4u}, 
   {1u, 4u}, 
   {2u, 4u}, 
   {3u, 4u}, 
   {4u, 4u}, 

   {0u, 5u}, 
   {1u, 5u}, 
   {2u, 5u}, 
   {3u, 5u}, 

   {0u, 6u}, 
   {1u, 6u}, 
   {2u, 6u}, 

   {0u, 7u}, 
   {1u, 7u}, 

   {0u, 8u}
};

uint g_mul[P_M2][P_M2];
uint g_div[P_M2][P_M2];
uint g_field_lu[P_M2];
uint g_field[P_M2];

static void l_print_ts(const char *p_head)
{
   SYSTEMTIME _st;

   memset(ADDR_OF(_st), 0u, SIZE_OF(_st));

   GetSystemTime(ADDR_OF(_st));

   printf("<%s %u-%u-%u %02u:%02u:%02u:%04u>\n", 
          p_head, 
          _st.wYear, 
          _st.wMonth, 
          _st.wDay, 
          _st.wHour, 
          _st.wMinute, 
          _st.wSecond, 
          _st.wMilliseconds);
}

static void l_fatal(uint line)
{
   printf("\n[Fatal error in <%s> at line %u]\n", 
          __FILE__, line);

#if defined(WIN32)
   RaiseException(0u, 0u, 0u, 0u);
#else // if defined(WIN32)
   uint *p = 0u;

   (*p) = 0u;
#endif // else if defined(WIN32)
}

static uint l_max_u(uint a, uint b)
{
   return ((a >= b) ? a : b);
}

static uint l_min_u(uint a, uint b)
{
   return ((a <= b) ? a : b);
}

// l_clz_a(): count leading zeroes in an array of coeffcients
static uint l_clz_a(const uint *p_a, uint c_a)
{
   uint retval = 0u;

   if (p_a != NULL || 
       c_a == 0u)
   {
      while (retval < c_a && 
             p_a[retval] == 0u)
      {
         retval++;
      }
   }
   else
   {
      FATAL();
   }

   return retval;
}

static uint l_power(uint b, uint e, uint *p_r)
{
   uint b_retval = (b != 0u && p_r != NULL);
   uint result = 1u;
   uint prev_r = 0u;

   while (b_retval && 
          (e--) != 0u && 
          (result *= b) >= prev_r)
   {
      prev_r = result;
   }

   if (b_retval)
   {
      (*p_r) = result;
   }

   return b_retval;
}

static uint l_mod5(uint r, uint p)
{
   while (r != 0u && 
          p != 0u)
   {
      r /= 5u;
      p--;
   }

   return (r % 5u);
}

static uint l_cpy_a(uint *p_to, uint c_to, const uint *p_from, uint c_from)
{
   uint s_to = (SIZE_OF(uint) * c_to);
   uint s_from = (SIZE_OF(uint) * c_from);
   uint b_retval = (s_to >= c_to && 
                    s_from >= c_from && 
                    p_to != NULL && 
                    p_from != NULL && 
                    c_from != 0u && 
                    c_to >= c_from);

   if (b_retval)
   {
      memset(p_to, 0u, s_to);
      memcpy(ADDR_OF(p_to[c_to - c_from]), p_from, s_from);
   }
   else
   {
      FATAL();
   }

   return b_retval;
}

static uint l_cpy_s(uint *p_to, uint c_to, const uint *p_from, uint c_from)
{
   uint s_from = (SIZE_OF(uint) * c_from);
   uint b_retval = (p_to != NULL && 
                    p_from != NULL && 
                    c_to >= c_from && 
                    s_from > c_from);

   if (b_retval)
   {
      memcpy(p_to, p_from, s_from);
   }

   return b_retval;
}

static void l_print_binary(uint v, uint w)
{
   uint i = 0u;

   while (i < w)
   {
      printf("%c", (((v & (1u << (w - (++i)))) != 0) ? '1' : '0'));
   }
}

// Add 2 GF elements
static uint l_add_e(uint a, uint b, uint *p_r)
{
   uint b_retval = (a < P_M2 && 
                    b < P_M2 && 
                    p_r != NULL);

   if (b_retval)
   {
      (*p_r) = (a ^ b);
   }
   else
   {
      FATAL();
   }

   return b_retval;
}

// Add 2 GF elements
static uint l_sub_e(uint mn, uint sh, uint *p_r)
{
   return l_add_e(mn, sh, p_r);
}

// Multiply 2 GF elements
static uint l_mul_e(uint a, uint b, uint *p_r)
{
   uint b_retval = (a < ELEMS_OF(g_mul) && 
                    b < ELEMS_OF(g_mul) && 
                    p_r != NULL);

   if (b_retval)
   {
      (*p_r) = g_mul[a][b];
   }
   else
   {
      FATAL();
   }

   return b_retval;
}

// Divide 2 GF elements
static uint l_div_e(uint dd, uint dr, uint *p_q)
{
   uint b_retval = (dd < ELEMS_OF(g_div) && 
                    dr < ELEMS_OF(g_div) && 
                    p_q != NULL && 
                    dr != 0u);

   if (b_retval)
   {
      (*p_q) = g_div[dd][dr];
   }
   else
   {
      FATAL();
   }

   return b_retval;
}

// Add 2 polynomials over a GF
static uint l_add_p(const uint *p_a, uint c_a, const uint *p_b, uint c_b, uint *p_r, uint c_r)
{
   uint i, j, _r;
   uint b_retval = ((j = (c_a + c_b)) > l_max_u(c_a, c_b) && 
                    p_a != NULL && 
                    p_b != NULL && 
                    p_r != NULL && 
                    c_a != 0u && 
                    c_b != 0u && 
                    c_r != 0u);

   if (b_retval)
   {
      memset(p_r, 0u, (SIZE_OF(uint) * c_r));
      i = 0u;

      while (b_retval && i < j)
      {
         _r = ((i < c_a) ? p_a[(c_a - i) - 1u] : 0u);

         if ((i >= c_b || 
              l_add_e(_r, p_b[(c_b - i) - 1u], ADDR_OF(_r))) && 
             (_r == 0u || 
              i < c_r))
         {
            if (_r != 0u)
            {
               p_r[(c_r - i) - 1u] = _r;
            }
         }
         else
         {
            b_retval = 0u;
         }

         i++;
      }
   }

   return b_retval;
}

// Multiply 2 polynomials over a GF
static uint l_mul_p(const uint *p_a, uint c_a, const uint *p_b, uint c_b, uint *p_r, uint c_r)
{
   uint b_retval = ((c_a + c_b) > l_max_u(c_a, c_b) && 
                    p_a != NULL && 
                    p_b != NULL && 
                    p_r != NULL && 
                    c_r != 0u);
   uint w_a, w_b;
   uint i, j, k;
   uint a, b;
   uint _r;

   if (b_retval)
   {
      memset(p_r, 0u, (SIZE_OF(uint) * c_r));
      w_a = (c_a - l_clz_a(p_a, c_a));
      w_b = (c_b - l_clz_a(p_b, c_b));
      i = 0u;

      while (b_retval && 
             i < w_a)
      {
         j = 0u;

         while (b_retval && 
                j < w_b)
         {
            if ((a = p_a[(c_a - i) - 1u]) != 0u && 
                (b = p_b[(c_b - j) - 1u]) != 0u && 
                ((k = (i + j)) >= c_r || 
                 !l_mul_e(a, b, ADDR_OF(_r)) || 
                 !l_add_e(p_r[(c_r - k) - 1u], _r, ADDR_OF(p_r[(c_r - k) - 1u]))))
            {
               b_retval = 0u;
            }
            else
            {
               j++;
            }
         }

         i++;
      }
   }

   return b_retval;
}

// Divide 2 polynomials over a GF
static uint l_div_p(uint b_debug, const uint *p_dd, uint c_dd, const uint *p_dr, uint c_dr, uint *p_q, uint c_q, uint *p_r, uint c_r)
{
#if defined(DEBUG_PRINT)
   uint j;
#endif // if defined(DEBUG_PRINT)
   uint i;
   uint r, _v;
   uint w_r, w_dr;
   uint z_r, z_dr, z_i;
   uint b_uq = (p_q != NULL);
   uint b_retval = (p_dd != NULL && 
                    c_dd != 0u && 
                    p_dr != NULL && 
                    c_dr != 0u && 
                    (z_dr = l_clz_a(p_dr, c_dr)) < c_dr && 
                    (b_uq || 
                     c_q == 0u) && 
                    p_r != NULL && 
                    l_cpy_a(p_r, c_r, p_dd, c_dd));

   if (b_retval)
   {
#if defined(DEBUG_PRINT)
      if (b_debug)
      {
         printf("x^ ");

         for (j = 0u; j < c_dd; j++)
         {
            printf(" %-2u", ((c_dd - j) - 1u));
         }

         printf("\n   ");

         for (j = 0u; j < c_dd; j++)
         {
            printf(" %-2u", p_dd[j]);
         }

         printf("\n");
      }
#endif // if defined(DEBUG_PRINT)

      if (b_uq)
      {
         memset(p_q, 0u, (c_q * SIZE_OF(uint)));
      }

      w_dr = (c_dr - z_dr);
      z_i = 0u;

      while (b_retval && 
             (w_r = (c_r - (z_r = l_clz_a(p_r, c_r)))) >= w_dr)
             //(w_r = (c_r - (z_r = (z_i++)))) >= w_dr)
      {
         if ((!b_uq || 
              (i = (w_r - w_dr)) < c_q) && 
             l_div_e(p_r[z_r], p_dr[z_dr], ADDR_OF(r)))
         {
#if defined(DEBUG_PRINT)
            if (b_debug)
            {
               printf("x%-2u ", r);

               for (j = 0u; j < z_r; j++)
               {
                  printf("   ");
               }

               for (j = 0u; j < c_dr; j++)
               {
                  if (l_mul_e(p_dr[j], r, ADDR_OF(_v)))
                  {
                     printf("%-2u ", _v);
                  }
                  else
                  {
                     printf("-- ");
                  }
               }

               printf("\n    ");

               for (j = 0u; j < z_r; j++)
               {
                  printf("   ");
               }
            }
#endif // if defined(DEBUG_PRINT)

            if (b_uq)
            {
               p_q[(c_q - i) - 1u] = r;
            }

            i = 0u;

            do 
            {
               if (l_mul_e(p_dr[z_dr + i], r, ADDR_OF(_v)) && 
                   l_add_e(p_r[z_r + i], _v, ADDR_OF(p_r[z_r + i])))
               {
#if defined(DEBUG_PRINT)
                  if (b_debug)
                  {
                     if (i != 0u || 
                         p_r[z_r + i] != 0u)
                     {
                        printf("%-2u ", p_r[z_r + i]);
                     }
                     else
                     {
                        printf("   ");
                     }
                  }
#endif // if defined(DEBUG_PRINT)
               }
               else
               {
                  b_retval = 0u;
               }
            } while (b_retval && (++i) < w_dr);

#if defined(DEBUG_PRINT)
            if (b_debug)
            {
               if (w_r >= (w_dr + 1u))
               {
                  printf("%-2u ", p_r[z_r + i]);
               }

               printf("\n");
            }
#endif // if defined(DEBUG_PRINT)
         }
         else
         {
            b_retval = 0u;
         }
      }
   }

   if (!b_retval)
   {
      FATAL();
   }

   return b_retval;
}

static uint l_solve_p(const uint *p_p, uint c_p, uint ae, uint *p_r)
{
   uint j = (c_p - l_clz_a(p_p, c_p));
   uint b_retval = ((p_p != NULL || 
                     c_p == 0u) && 
                    p_r != NULL && 
                    ae < P_N);
   uint result = 0u;
   uint i = 0u;
   uint _v;

   while (b_retval && 
          i < j)
   {
      if (!l_mul_e(p_p[(c_p - i) - 1u], 
                   g_field[(((P_N - ae) * i) % P_N) + 1u], 
                   ADDR_OF(_v)) || 
            !l_add_e(result, _v, ADDR_OF(result)))
      {
         b_retval = 0u;
      }

      i++;
   }

   if (b_retval)
   {
      (*p_r) = result;
   }

   return b_retval;
}

static void l_gen_field(void)
{
   uint i = 1u;
   uint v = 1u;
#if defined(DEBUG_PRINT)
   uint j, k, l;
#endif // if defined(DEBUG_PRINT)

   memset(ADDR_OF(g_field_lu[0u]), 0u, SIZE_OF(g_field_lu));
   memset(ADDR_OF(g_field[0u]), 0u, SIZE_OF(g_field));

   while (i < ELEMS_OF(g_field))
   {
      g_field[i] = v;

      if ((++i) < ELEMS_OF(g_field) && 
          ((v <<= 1u) & P_M2) != 0u)
      {
         v ^= P_POLY_FIELD;
      }
   }

#if defined(DEBUG_PRINT)
   printf("index form  polynomial form   binary form    decimal form\n");
#endif // if defined(DEBUG_PRINT)

   for (i = 0u; i < ELEMS_OF(g_field); i++)
   {
      v = g_field[i];

      g_field_lu[v] = i;

#if defined(DEBUG_PRINT)
      printf("%-12u", i);

      if (v != 0u)
      {
         j = 0u;
         k = 0u;
         l = 0u;

         do 
         {
            if ((v & 1u) != 0u)
            {
               if ((k++) != 0u)
               {
                  printf("+");
                  l++;
               }

               printf("%c", 
                     ((j == 0u) ? '1' : 'a'));
               l++;

               if (j > 1u)
               {
                  printf("^%u", j);
                  l += 2u;
               }
            }

            v >>= 1u;
            j++;
         } while (v != 0u && 
                  j < P_M);
      }
      else
      {
         printf("0");
         l = 1u;
      }

      printf("%-*s", (18u - l), "");
      l_print_binary(g_field[i], P_M);
      printf("%-*s%u\n", (15u - P_M), "", g_field[i]);
#endif // if defined(DEBUG_PRINT)
   }
}

static void l_gen_add_sub(void)
{
// Addition and subtraction are equivalent to 
// XOR, so we use no lookup table for these
#if defined(DEBUG_PRINT)
   uint i, j, r;

   printf("Addition/subtraction table:\n    ");

   for (j = 0u; j < P_M2; j++)
   {
      printf("%-4u", j);
   }

   for (i = 0u; i < P_M2; i++)
   {
      printf("\n% 2u: ", i);

      for (j = 0u; j < P_M2; j++)
      {
         if (l_add_e(i, j, ADDR_OF(r)))
         {
            printf("%-4u", r);
         }
         else
         {
            printf("----");
         }
      }
   }

   printf("\n");
#endif // if defined(DEBUG_PRINT)
}

static void l_gen_mul(void)
{
   uint i, j;

#if defined(DEBUG_PRINT)
   printf("Multiplication table:\n    ");

   for (j = 0u; j < ELEMS_OF(g_mul[0u]); j++)
   {
      printf("%-4u", j);
   }
#endif // if defined(DEBUG_PRINT)

   memset(ADDR_OF(g_mul[0u]), 0u, SIZE_OF(g_mul));

   for (i = 0u; i < ELEMS_OF(g_mul); i++)
   {
#if defined(DEBUG_PRINT)
      printf("\n% 2u: ", i);
#endif // if defined(DEBUG_PRINT)

      for (j = 0u; j < ELEMS_OF(g_mul[0u]); j++)
      {
         // Use the logarithmic technique
         g_mul[i][j] = ((i == 0u || j == 0u) ? 0u : 
                        g_field[(((g_field_lu[i] + g_field_lu[j]) - 2u) % 
                                 (P_M2 - 1u)) + 
                                1u]);

#if defined(DEBUG_PRINT)
         printf("%-4u", g_mul[i][j]);
#endif // if defined(DEBUG_PRINT)
      }
   }

#if defined(DEBUG_PRINT)
   printf("\n");
#endif // if defined(DEBUG_PRINT)
}

static void l_gen_div(void)
{
   uint i, j, v, dd, dr;

#if defined(DEBUG_PRINT)
   printf("Division table (dividend: ver, divisor: hor):\n    ");

   for (j = 0u; j < ELEMS_OF(g_div[0u]); j++)
   {
      printf("%-4u", j);
   }
#endif // if defined(DEBUG_PRINT)

   memset(ADDR_OF(g_div[0u]), 0u, SIZE_OF(g_div));

   for (i = 0u; i < ELEMS_OF(g_div); i++)
   {
#if defined(DEBUG_PRINT)
      printf("\n% 2u: ", i);
#endif // if defined(DEBUG_PRINT)

      for (j = 0u; j < ELEMS_OF(g_div[0u]); j++)
      {
         if (j == 0u)
         {
#if defined(DEBUG_PRINT)
            printf("--  ");
#endif // if defined(DEBUG_PRINT)
         }
         else
         {
            if (i == 0u)
            {
               v = 0u;
            }
            else
            {
               // Use the logarithmic technique
               dd = g_field_lu[i];
               dr = g_field_lu[j];

               if (dd < dr)
               {
                  dd += (P_M2 - 1u);
               }

               v = g_field[((dd - dr) % 
                            (P_M2 - 1u)) + 
                           1u];
            }

            g_div[i][j] = v;

#if defined(DEBUG_PRINT)
            printf("%-4u", g_div[i][j]);
#endif // if defined(DEBUG_PRINT)
         }
      }
   }

#if defined(DEBUG_PRINT)
   printf("\n");
#endif // if defined(DEBUG_PRINT)
}

static void l_print_u(uint v, uint b_nf)
{
   if (b_nf)
   {
      printf(" ");
   }

   printf("%-2u", v);
}

static void l_print_a(const char *p_head, const uint *p_a, uint c, uint b_hw)
{
   uint _c;
   uint hw;
   uint i;

   if (p_head != NULL && 
       (p_a != NULL || 
        c == 0u))
   {
      printf(p_head);

      hw = 0u;

      for (i = 0u; i < c; i++)
      {
         if ((_c = p_a[i]) != 0)
         {
            hw++;
         }

         l_print_u(_c, (i != 0u));
      }

      if (b_hw)
      {
         printf(" (%u)", hw);
      }

      printf("\n");
   }
}

static void l_print_p(const char *p_head, const uint *p_a, uint c)
{
   uint _v, _e;
   uint b_nf;
   uint i;

   if (p_head != NULL && 
       (p_a != NULL || 
        c == 0u))
   {
      printf("%s(x)=", p_head);

      if (c != 0u)
      {
         b_nf = 0u;

         for (i = 0u; i < c; i++)
         {
            if ((_v = p_a[i]) != 0u)
            {
               if (b_nf)
               {
                  printf("+");
               }

               switch (_e = ((c - i) - 1u))
               {
               case 0u:
                  printf("%u", _v);
                  break;

               case 1u:
                  printf("%ux", _v);
                  break;

               default:
                  printf("%ux^%u", 
                         _v, 
                         _e);
                  break;
               }

               b_nf = 1u;
            }
         }
      }
      else
      {
         printf("0");
      }

      printf("\n");
   }
}

// Is v a valid symbol?
static uint l_is_vs(uint v)
{
   return (v < (1u << P_M));
}

// Is v a forbidden symbol (considering those thare are forbidden in the parity part)
static uint l_is_fs_p(uint v)
{
   return (// SILENCE (I): 11111
           v == 0x1fu || 
           // ESD / HB (T): 01101
           v == 0x0du || 
           // ESDOK / ESDBRS (R): 00111
           v == 0x07u);
}

// Find an element in an array
static uint l_find(uint v, const uint *p_from, uint c_from)
{
   uint retval = ((p_from == NULL) ? 
                  c_from : 
                  0u);

   while (retval < c_from && 
          p_from[retval] != v)
   {
      retval++;
   }

   return retval;
}

static uint l_encode(const uint *p_m, uint c_m, uint *p_r, uint c_r)
{
   uint b_retval = 0u;
   uint t_dd[P_N];
   uint t_r[P_N];
   uint t_z;

   if (p_m != NULL && 
       c_m == P_K && 
       p_r != NULL && 
       c_r == P_R)
   {
#if defined(DEBUG_PRINT)
      printf("Encoding:\n");

      l_print_a("M=", p_m, c_m, 0u);
      l_print_a("P=", ADDR_OF(P_POLY_CODE[0u]), ELEMS_OF(P_POLY_CODE), 0u);
#endif // if defined(DEBUG_PRINT)

      memset(ADDR_OF(t_dd[0u]), 0u, SIZE_OF(t_dd));
      memcpy(ADDR_OF(t_dd[0u]), p_m, (SIZE_OF(uint) * P_K));

      if (l_div_p(1u, 
                  ADDR_OF(t_dd[0u]), ELEMS_OF(t_dd), 
                  ADDR_OF(P_POLY_CODE[0u]), ELEMS_OF(P_POLY_CODE), 
                  NULL, 0u, 
                  ADDR_OF(t_r[0u]), ELEMS_OF(t_r)))
      {
#if defined(DEBUG_PRINT)
         l_print_a("R=", ADDR_OF(t_r[0u]), ELEMS_OF(t_r), 0u);
#endif // if defined(DEBUG_PRINT)

         t_z = l_clz_a(ADDR_OF(t_r[0u]), ELEMS_OF(t_r));

         if (t_z >= P_K && 
             t_z <= P_N)
         {
            memcpy(p_r, ADDR_OF(t_r[P_K]), (SIZE_OF(uint) * P_R));

            b_retval = 1u;
         }
      }
   }

   if (!b_retval)
   {
      FATAL();
   }

   return b_retval;
}

static uint l_esc_p(uint *p_r, uint c_r)
{
   uint b_ok = (p_r != NULL && 
                c_r != 0u);
   uint i = 0u;
   uint _c;

   while (b_ok && 
          l_is_vs(_c = p_r[i]) && 
          i < c_r)
   {
      if (l_is_fs_p(_c))
      {
         p_r[i] = (_c & ((uint)-2));
      }

      i++;
   }

   return (b_ok && i == c_r);
}

static uint l_decode(const uint *p_m, uint c_m, uint *p_o, uint c_o)
{
   uint b_ok = (p_m != NULL && 
                c_m == P_N && 
                (p_o == NULL || 
                 c_o == c_m));
   uint dd[(2u * P_T) + 1u];
   uint isv[ELEMS_OF(dd)];
   uint iv[ELEMS_OF(dd)];
   uint dr[ELEMS_OF(dd)];
   uint _r[ELEMS_OF(dd)];
   uint _q[ELEMS_OF(dd)];
   uint _m[ELEMS_OF(dd)];
   uint _t[ELEMS_OF(dd)];
   uint O[ELEMS_OF(dd)];
   uint gO[ELEMS_OF(O)];
   uint L[ELEMS_OF(dd)];
   uint gL[ELEMS_OF(L)];
   uint dL[ELEMS_OF(L)];
   uint b_retval = 0u;
   uint S[2u * P_T];
   uint C[P_N];
   uint E[P_N];
   uint _n, _d;
   uint a, _v;
   uint i, j;
   uint c_z;

#if defined(DEBUG_PRINT)
   printf("Decoding:\n");

   l_print_a("M'=", p_m, c_m, 0u);
#endif // if defined(DEBUG_PRINT)

   // First calculate syndrome using Horner's method
   memset(ADDR_OF(S[0u]), 0u, SIZE_OF(S));
   i = 0u;

   while (b_ok && 
          i < ELEMS_OF(S))
   {
      a = g_field[i + 1u];
      _v = 0u;
      j = 0u;

      while (b_ok && 
             j < P_N)
      {
         if (!l_add_e(_v, p_m[j++], ADDR_OF(_v)) || 
             (j < P_N && 
              !l_mul_e(_v, a, ADDR_OF(_v))))
         {
            b_ok = 0u;
         }
      }

      S[ELEMS_OF(S) - (++i)] = _v;
   }

   if (b_ok && 
       l_cpy_a(ADDR_OF(dr[0u]), 
               ELEMS_OF(dr), 
               ADDR_OF(S[0u]), 
               ELEMS_OF(S)), 
      (p_o == NULL || 
       l_cpy_a(p_o, 
               c_o, 
               p_m, 
               c_m)))
   {
#if defined(DEBUG_PRINT)
      l_print_a("Syndrome=", ADDR_OF(S[0u]), ELEMS_OF(S), 1u);
#endif // if defined(DEBUG_PRINT)

      _v = (ELEMS_OF(dr) - l_clz_a(ADDR_OF(dr[0u]), ELEMS_OF(dr)));

      if (_v <= P_T)
      {
         if (_v == 0u)
         {
#if defined(DEBUG_PRINT)
            printf("No errors found\n");
#endif // if defined(DEBUG_PRINT)

            b_retval = 1u;
         }
         else
         {
            b_ok = 0u;
         }
      }
      else
      {
         // Error location polynomial calculation using the Euclidean algorithm
         memset(ADDR_OF(isv[0u]), 0u, SIZE_OF(isv));
         memset(ADDR_OF(iv[0u]), 0u, SIZE_OF(iv));
         memset(ADDR_OF(dd[0u]), 0u, SIZE_OF(dd));
         iv[ELEMS_OF(iv) - 1u] = 1u;
         dd[0u] = 1u;

         do 
         {
            if (!l_div_p(0u, 
                         ADDR_OF(dd[0u]), ELEMS_OF(dd), 
                         ADDR_OF(dr[0u]), ELEMS_OF(dr), 
                         ADDR_OF(_q[0u]), ELEMS_OF(_q), 
                         ADDR_OF(_r[0u]), ELEMS_OF(_r)) || 
                !l_mul_p(ADDR_OF(iv[0u]), ELEMS_OF(iv), 
                         ADDR_OF(_q[0u]), ELEMS_OF(_q), 
                         ADDR_OF(_m[0u]), ELEMS_OF(_m)) || 
                !l_add_p(ADDR_OF(isv[0u]), ELEMS_OF(isv), 
                         ADDR_OF(_m[0u]), ELEMS_OF(_m), 
                         ADDR_OF(_t[0u]), ELEMS_OF(_t)) || 
                !l_cpy_a(ADDR_OF(dd[0u]), ELEMS_OF(dd), 
                         ADDR_OF(dr[0u]), ELEMS_OF(dr)) || 
                !l_cpy_a(ADDR_OF(dr[0u]), ELEMS_OF(dr), 
                         ADDR_OF(_r[0u]), ELEMS_OF(_r)) || 
                !l_cpy_a(ADDR_OF(isv[0u]), ELEMS_OF(isv), 
                         ADDR_OF(iv[0u]), ELEMS_OF(iv)) || 
                !l_cpy_a(ADDR_OF(iv[0u]), ELEMS_OF(iv), 
                         ADDR_OF(_t[0u]), ELEMS_OF(_t)))
            {
               b_ok = 0u;
            }
         }  while (b_ok && 
                   (ELEMS_OF(dr) - l_clz_a(ADDR_OF(dr[0u]), ELEMS_OF(dr))) > P_T);

         if (b_ok && 
             l_cpy_a(ADDR_OF(gL[0u]), ELEMS_OF(gL), 
                     ADDR_OF(iv[0u]), ELEMS_OF(iv)) && 
             l_cpy_a(ADDR_OF(gO[0u]), ELEMS_OF(gO), 
                     ADDR_OF(dr[0u]), ELEMS_OF(dr)))
         {
#if defined(DEBUG_PRINT)
            l_print_p("gamma*Lambda", ADDR_OF(gL[0u]), ELEMS_OF(gL));
            l_print_p("gamma*Omega", ADDR_OF(gO[0u]), ELEMS_OF(gO));
#endif // if defined(DEBUG_PRINT)

            memset(ADDR_OF(dr[0u]), 0u, SIZE_OF(dr));

            if ((dr[ELEMS_OF(dr) - 1u] = gL[ELEMS_OF(gL) - 1u]) != 0u)
            {
               if (l_cpy_a(ADDR_OF(dd[0u]), ELEMS_OF(dd), 
                           ADDR_OF(gL[0u]), ELEMS_OF(gL)) && 
                   l_div_p(0u, 
                           ADDR_OF(dd[0u]), ELEMS_OF(dd), 
                           ADDR_OF(dr[0u]), ELEMS_OF(dr), 
                           ADDR_OF(L[0u]), ELEMS_OF(L), 
                           ADDR_OF(_r[0u]), ELEMS_OF(_r)) && 
                   l_clz_a(ADDR_OF(_r[0u]), ELEMS_OF(_r)) == ELEMS_OF(_r) && 
                   L[ELEMS_OF(L) - 1u] == 1u && 
                   l_cpy_a(ADDR_OF(dd[0u]), ELEMS_OF(dd), 
                           ADDR_OF(gO[0u]), ELEMS_OF(gO)) && 
                   l_div_p(0u, 
                           ADDR_OF(dd[0u]), ELEMS_OF(dd), 
                           ADDR_OF(dr[0u]), ELEMS_OF(dr), 
                           ADDR_OF(O[0u]), ELEMS_OF(O), 
                           ADDR_OF(_r[0u]), ELEMS_OF(_r)) && 
                   l_clz_a(ADDR_OF(_r[0u]), ELEMS_OF(_r)) == ELEMS_OF(_r) && 
                   (ELEMS_OF(L) - l_clz_a(ADDR_OF(L[0u]), ELEMS_OF(L))) != 0u)
               {
#if defined(DEBUG_PRINT)
                  l_print_p("Lambda", ADDR_OF(L[0u]), ELEMS_OF(L));
                  l_print_p("Omega", ADDR_OF(O[0u]), ELEMS_OF(O));

                  printf("Chien search\nx     sum\n");
#endif // if defined(DEBUG_PRINT)

                  // Chien search
                  memset(ADDR_OF(C[0u]), 0u, SIZE_OF(C));
                  i = P_N;

                  while (b_ok && 
                         (i--) != 0u)
                  {
                     if (l_solve_p(ADDR_OF(L[0u]), ELEMS_OF(L), i, ADDR_OF(C[i])))
                     {
#if defined(DEBUG_PRINT)
                        if (i != 0u)
                        {
                           printf("a^-%-2u ", i);
                        }
                        else
                        {
                           printf("a     ");
                        }

                        printf("%u\n", C[i]);
#endif // if defined(DEBUG_PRINT)
                     }
                     else
                     {
                        b_ok = 0u;
                     }
                  }

                  if (b_ok)
                  {
#if defined(DEBUG_PRINT)
                     printf("Forney algorithm\n");
#endif // if defined(DEBUG_PRINT)

                     memset(ADDR_OF(dL[0u]), 0u, SIZE_OF(dL));

                     // Forney algorithm
                     for (i = 1u; i < ELEMS_OF(dL); i++)
                     {
                        if (((i + ELEMS_OF(L)) & 1u) != 0u)
                        {
                           dL[i] = L[i - 1u];
                        }
                     }

#if defined(DEBUG_PRINT)
                     l_print_p("Lambda'", ADDR_OF(dL[0u]), ELEMS_OF(dL));
#endif // if defined(DEBUG_PRINT)

                     memset(ADDR_OF(E[0u]), 0u, SIZE_OF(E));
                     c_z = 0u;
                     i = 0u;

                     while (b_ok && 
                            i < ELEMS_OF(E))
                     {
                        if (C[i] == 0u)
                        {
                           if (l_solve_p(ADDR_OF(O[0u]), ELEMS_OF(O), i, ADDR_OF(_n)) && 
                               l_solve_p(ADDR_OF(dL[0u]), ELEMS_OF(dL), i, ADDR_OF(_d)) && 
                               l_div_e(_n, _d, ADDR_OF(_v)) && 
                               l_mul_e(g_field[i + 1u], _v, ADDR_OF(E[(ELEMS_OF(E) - i) - 1u])))
                           {
                              c_z++;
                           }
                           else
                           {
                              b_ok = 0u;
                           }
                        }

                        i++;
                     }

                     if (b_ok && 
                         c_z != 0u && 
                         c_z <= P_T)
                     {
#if defined(DEBUG_PRINT)
                        l_print_a("E'=      ", ADDR_OF(E[0u]), ELEMS_OF(E), 1u);
#endif // if defined(DEBUG_PRINT)

                        if (p_o != NULL)
                        {
                           j = l_min_u(ELEMS_OF(E), c_o);
                           i = 0u;

                           while (i < j && 
                                  l_add_e(p_o[i], E[i], ADDR_OF(p_o[i])))
                           {
                              i++;
                           }

                           if (i < j)
                           {
                              b_ok = 0u;
                           }
                           else
                           {
                              b_retval = 1u;
                           }
                        }
                        else
                        {
                           b_retval = 1u;
                        }
                     }
                  }
               }
               else
               {
                  b_ok = 0u;
               }
            }
         }
         else
         {
            b_ok = 0u;
         }
      }
   }
   else
   {
      b_ok = 0u;
   }

   if (!b_ok)
   {
      FATAL();
   }

   return b_retval;
}

static uint l_enc_do(const uint *p_from, uint c_from, uint *p_to, uint c_to, uint *p_s)
{
   uint b_retval = (p_from != NULL && 
                    p_to != NULL && 
                    p_s != NULL && 
                    c_from == 15u && 
                    c_to == c_from);
   uint prev_fs = 0u;
   uint mask_fs = 0u;
   uint pos_7 = 0u;
   uint cnt_7 = 0u;
   uint i = c_from;
   uint _i, _c, _v;
   uint j;

   while (b_retval && 
          (i--) != 0u)
   {
      _c = p_from[i];

      if ((_i = l_find(_c, ADDR_OF(lC_fss[0]), ELEMS_OF(lC_fss))) < ELEMS_OF(lC_fss))
      {
         mask_fs |= (1u << i);

         if (prev_fs > i)
         {
            j = (prev_fs - i);

            if (j > 6u)
            {
               pos_7 = 1u;
               cnt_7++;
            }
            else
            {
               if (pos_7 != 0u)
               {
                  pos_7++;
               }
            }

            j %= 7u;
            j *= ELEMS_OF(lC_fss);
            j += _i;

            if (j < ELEMS_OF(lC_nfss))
            {
               _c = lC_nfss[j];
            }
            else
            {
               b_retval = 0u;

               FATAL();
            }
         }
         else
         {
            _c = lC_nfss[_i];
         }

         prev_fs = i;
      }

      p_to[i] = _c;
   }

   if (b_retval)
   {
      i = 0u;

      switch (mask_fs)
      {
      // 0: No FS
      case 0u:
         if (cnt_7 != 0u)
         {
            FATAL();
         }

         _v = 0u;
         break;

      // 0x4001: 1-15
      case 0x4001u:
         if (cnt_7 != 1u)
         {
            FATAL();
         }

         _v = 16u;
         break;

      // 0x4081: 1-8-15
      case 0x4081u:
         if (cnt_7 != 2u)
         {
            FATAL();
         }

         _v = 17u;
         break;

      default:
         if (pos_7 == 0u)
         {
            if (cnt_7 != 0u)
            {
               FATAL();
            }

            _v = (prev_fs + 1u);
         }
         else
         {
            if (cnt_7 != 1u)
            {
               FATAL();
            }

            i = 0u;

            while (i < ELEMS_OF(lC_di_sfs) && 
                   (lC_di_sfs[i].first != prev_fs || 
                    lC_di_sfs[i].a7 != pos_7))
            {
               i++;
            }

            if (i < ELEMS_OF(lC_di_sfs))
            {
               _v = (18u + i);
            }
            else
            {
               _v = 0u;

               FATAL();
            }
         }
         break;
      }

      (*p_s) = _v;

      if (_v >= 54u)
      {
         FATAL();
      }
   }

   return b_retval;
}

static uint l_enc_s_add_1(uint s, const uint *p_from, uint c_from, uint *p_to, uint c_to)
{
   uint b_retval = (ELEMS_OF(lC_nfss_9) == 9u && 
                    p_from != NULL && 
                    p_to != NULL && 
                    c_from == 16u && 
                    c_to == c_from && 
                    s < 54u);
   uint _v;
   uint i;

   if (b_retval)
   {
      for (i = 0u; i < 15u; i++)
      {
         p_to[i] = p_from[i];
      }

      _v = (p_from[15u] & 1u);
      _v |= lC_nfss_9[s / 6u];
      p_to[15u] = _v;
   }

   return b_retval;
}

static uint l_enc_s_add_2(uint s, uint a, uint b, uint *p_s1)
{
   const uint *p_6;
   uint       _a = a;
   uint       _b = b;
   uint       _v1, _v2;
   uint       b_m, b_l;
   uint       b_retval = (ELEMS_OF(lC_nfss_9) == 9u && 
                          ELEMS_OF(lC_nfss_6_00) == 6u && 
                          ELEMS_OF(lC_nfss_6_01) == 6u && 
                          ELEMS_OF(lC_nfss_6_10) == 6u && 
                          ELEMS_OF(lC_nfss_6_11) == 6u && 
                          p_s1 != NULL && 
                          s < 54u && 
                          l_mul_e(_a, 15u, ADDR_OF(_v1)) && 
                          l_mul_e(_b, 7u, ADDR_OF(_v2)) && 
                          l_add_e(_v1, _v2, ADDR_OF(a)) && 
                          l_mul_e(_a, 14u, ADDR_OF(_v1)) && 
                          l_mul_e(_b, 6u, ADDR_OF(_v2)) && 
                          l_add_e(_v1, _v2, ADDR_OF(b)));

   if (b_retval)
   {
      b_m = ((a & 1u) != 0u);
      b_l = ((b & 1u) != 0u);

      p_6 = ((b_m == b_l) ? 
             (b_m ? ADDR_OF(lC_nfss_6_11[0u]) : ADDR_OF(lC_nfss_6_00[0u])) : 
             (b_m ? ADDR_OF(lC_nfss_6_10[0u]) : ADDR_OF(lC_nfss_6_01[0u])));

      (*p_s1) = p_6[s % 6u];
   }

   return b_retval;
}

static uint l_enc_undo(const uint *p_from, uint c_from, uint *p_to, uint c_to)
{
   uint           b_retval = (ELEMS_OF(lC_nfss_9) == 9u && 
                              ELEMS_OF(lC_nfss_6_00) == 6u && 
                              ELEMS_OF(lC_nfss_6_01) == 6u && 
                              ELEMS_OF(lC_nfss_6_10) == 6u && 
                              ELEMS_OF(lC_nfss_6_11) == 6u && 
                              p_from != NULL && 
                              p_to != NULL && 
                              c_from == 17u && 
                              c_to == 16u);
   const S_di_sfs *p_di_sfs;
   uint           i_a7 = 0u;
   uint           b_m, b_l;
   uint           s1, s2;
   const uint     *p_6;
   uint           _v;
   uint           s;
   uint           i;

   if (b_retval)
   {
      s = p_from[15u];
      p_to[15u] = (s & 1u);
      memcpy(p_to, p_from, (15u * SIZE_OF(uint)));

      s &= ((uint)-2);

      if ((s1 = l_find(s, ADDR_OF(lC_nfss_9[0]), ELEMS_OF(lC_nfss_9))) < ELEMS_OF(lC_nfss_9))
      {
         s = p_from[16u];
         b_m = ((s & 0x10u) != 0u);
         b_l = ((s & 0x01u) != 0u);

         p_6 = ((b_m == b_l) ? 
                (b_m ? ADDR_OF(lC_nfss_6_01[0u]) : ADDR_OF(lC_nfss_6_00[0u])) : 
                (b_m ? ADDR_OF(lC_nfss_6_11[0u]) : ADDR_OF(lC_nfss_6_10[0u])));

         if ((s2 = l_find(s, p_6, ELEMS_OF(lC_nfss_6_00))) < ELEMS_OF(lC_nfss_6_00) && 
             (s = ((s1 * 6u) + s2)) < 54u)
         {
            switch (s)
            {
            // 0x4081: 1-8-15
            case 17u:
               p_to[7u] = lC_fss[l_find(p_from[7u], ADDR_OF(lC_nfss[0u]), ELEMS_OF(lC_nfss)) % ELEMS_OF(lC_fss)];
               //break;

            // 0x4001: 1-15
            case 16u:
               p_to[0u] = lC_fss[l_find(p_from[0u], ADDR_OF(lC_nfss[0u]), ELEMS_OF(lC_nfss)) % ELEMS_OF(lC_fss)];
               p_to[14u] = lC_fss[l_find(p_from[14u], ADDR_OF(lC_nfss[0u]), ELEMS_OF(lC_nfss)) % ELEMS_OF(lC_fss)];
               break;

            default:
               if (s >= 18u)
               {
                  if ((s -= 18u) < ELEMS_OF(lC_di_sfs))
                  {
                     p_di_sfs = ADDR_OF(lC_di_sfs[s]);

                     s = (p_di_sfs->first + 1u);
                     i_a7 = p_di_sfs->a7;
                  }
                  else
                  {
                     b_retval = 0u;

                     FATAL();
                  }
               }
            }

            if ((s--) != 0u)
            {
               i = 0u;

               while (b_retval && 
                      s < 15u)
               {
                  _v = l_find(p_to[s], ADDR_OF(lC_nfss[0u]), ELEMS_OF(lC_nfss));
                  p_to[s] = lC_fss[_v % ELEMS_OF(lC_fss)];
                  _v /= ELEMS_OF(lC_fss);

                  if ((++i) == i_a7)
                  {
                     _v += 7u;
                  }

                  if (_v != 0u)
                  {
                     s += _v;
                  }
                  else
                  {
                     s = c_to;
                  }
               }
            }
         }
         else
         {
            b_retval = 0u;

            FATAL();
         }
      }
      else
      {
         b_retval = 0u;

         FATAL();
      }
   }

   return b_retval;
}

static uint l_enc_ver_data(const uint *p_a, uint c_a)
{
   uint b_ok = (p_a != NULL && 
                c_a > 4u);
   uint j = (c_a - 2u);
   uint i = 0u;
   uint _c;

   while (b_ok && 
          i < c_a && 
          l_is_vs(_c = p_a[i]) && 
          ((_c = l_find(_c, ADDR_OF(lC_fss[0]), ELEMS_OF(lC_fss))) >= ELEMS_OF(lC_fss) || 
           (i >= j && 
            _c == (ELEMS_OF(lC_fss) - 1u))))
   {
      i++;
   }

   return (i == c_a);
}

#if !defined(TEST)
static uint l_run(void)
{
   uint e;
   uint _v;
   uint t_s;
   uint j, k;
   uint i = 0u;
   uint t_r2[5u];
   uint t_r1[19u];
   char t_per[12u];
   uint l_per = 0u;
   uint t_msg_1_orig[16u];
   uint t_msg_4_ch_out[19u];
   uint t_msg_5_dec_in[31u];
   uint t_msg_6_dec_out[31u];
   uint t_msg_2_fs_enc_in[16u];
   uint t_msg_7_fs_dec_out[16u];
   uint t_msg_3_fs_enc_out[19u];
   uint b_retval = (l_power(5u, (ELEMS_OF(t_msg_1_orig) - 1u), ADDR_OF(j)) && 
                    (j * 100u) > j);

   //srand((int)time(NULL));
   srand(0x5eaea8da);

   while (b_retval && 
          i < j)
   {
      _v = ((uint)((i * 100.0) / j));

      if (l_per != _v)
      {
         l_per = _v;

         if (sprintf_s(t_per, SIZE_OF(t_per), "%u%%", _v) > 0)
         {
            l_print_ts(t_per);
         }
      }

      memset(ADDR_OF(t_msg_1_orig[0u]), 0u, SIZE_OF(t_msg_1_orig));
      memset(ADDR_OF(t_msg_2_fs_enc_in[0u]), 0u, SIZE_OF(t_msg_2_fs_enc_in));
      memset(ADDR_OF(t_msg_3_fs_enc_out[0u]), 0u, SIZE_OF(t_msg_3_fs_enc_out));
      memset(ADDR_OF(t_msg_4_ch_out[0u]), 0u, SIZE_OF(t_msg_4_ch_out));
      memset(ADDR_OF(t_msg_5_dec_in[0u]), 0u, SIZE_OF(t_msg_5_dec_in));
      memset(ADDR_OF(t_msg_6_dec_out[0u]), 0u, SIZE_OF(t_msg_6_dec_out));
      memset(ADDR_OF(t_msg_7_fs_dec_out[0u]), 0u, SIZE_OF(t_msg_7_fs_dec_out));
      memset(ADDR_OF(t_r1[0u]), 0u, SIZE_OF(t_r1));
      memset(ADDR_OF(t_r2[0u]), 0u, SIZE_OF(t_r2));
      t_s = 0u;

      k = 0u;

      while (b_retval && 
             k < (ELEMS_OF(t_msg_1_orig) - 1u))
      {
         if ((_v = l_mod5(i, k)) <= ELEMS_OF(lC_fss))
         {
            t_msg_1_orig[k] = ((_v < ELEMS_OF(lC_fss)) ? 
                               lC_fss[_v] : 
                               lC_nfss[rand() % ELEMS_OF(lC_nfss)]);
         }
         else
         {
            b_retval = 0u;
         }

         k++;
      }

      if (b_retval)
      {
         t_msg_1_orig[15u] = (rand() & 1u);

         b_retval = l_enc_do(ADDR_OF(t_msg_1_orig[0u]), 
                             (ELEMS_OF(t_msg_1_orig) - 1u), 
                             ADDR_OF(t_msg_2_fs_enc_in[0u]), 
                             (ELEMS_OF(t_msg_2_fs_enc_in) - 1u), 
                             ADDR_OF(t_s));

         t_msg_2_fs_enc_in[15u] = t_msg_1_orig[15u];

         b_retval = (b_retval && 
                     l_enc_s_add_1(t_s, 
                                   ADDR_OF(t_msg_2_fs_enc_in[0u]), 
                                   ELEMS_OF(t_msg_2_fs_enc_in), 
                                   ADDR_OF(t_msg_3_fs_enc_out[0u]), 
                                   (ELEMS_OF(t_msg_3_fs_enc_out) - 3u)));

         b_retval = (b_retval && 
                     l_div_p(1u, 
                             ADDR_OF(t_msg_3_fs_enc_out[0u]), 
                             (ELEMS_OF(t_msg_3_fs_enc_out) - 3u), 
                             ADDR_OF(P_POLY_CODE[0u]), 
                             ELEMS_OF(P_POLY_CODE), 
                             NULL, 
                             0u, 
                             ADDR_OF(t_r1[0u]), 
                             (ELEMS_OF(t_r1) - 3u)));

         b_retval = (b_retval && 
                     l_clz_a(ADDR_OF(t_r1[0u]), ELEMS_OF(t_r1)) >= 14u);

         b_retval = (b_retval && 
                     l_enc_s_add_2(t_s, 
                                   t_r1[14u], 
                                   t_r1[15u], 
                                   ADDR_OF(t_r1[16u])));

         b_retval = (b_retval && 
                     l_div_p(1u, 
                             ADDR_OF(t_r1[14u]), 
                             (ELEMS_OF(t_r1) - 14u), 
                             ADDR_OF(P_POLY_CODE[0u]), 
                             ELEMS_OF(P_POLY_CODE), 
                             NULL, 
                             0u, 
                             ADDR_OF(t_r2[0u]), 
                             ELEMS_OF(t_r2)));

         b_retval = (b_retval && 
                     l_clz_a(ADDR_OF(t_r2[0u]), ELEMS_OF(t_r2)) >= 3u && 
                     (t_r2[3u] & 1u) == 0u && 
                     (t_r2[4u] & 1u) == 0u);

         if (b_retval)
         {
            t_msg_3_fs_enc_out[16u] = t_r1[16u];
            t_msg_3_fs_enc_out[17u] = t_r2[3u];
            t_msg_3_fs_enc_out[18u] = t_r2[4u];
         }

         b_retval = (b_retval && 
                     l_enc_ver_data(ADDR_OF(t_msg_3_fs_enc_out[0u]), 
                                    ELEMS_OF(t_msg_3_fs_enc_out)));

         e = 0u;

         while (b_retval && 
                e < ELEMS_OF(t_msg_4_ch_out))
                //e < 1u)
         {
            memcpy(ADDR_OF(t_msg_4_ch_out[0u]), 
                   ADDR_OF(t_msg_3_fs_enc_out[0u]), 
                   SIZE_OF(t_msg_4_ch_out));

            t_msg_4_ch_out[e++] ^= ((rand() % 30u) + 1u);

            memcpy(ADDR_OF(t_msg_5_dec_in[12u]), 
                   ADDR_OF(t_msg_4_ch_out[0u]), 
                   SIZE_OF(t_msg_4_ch_out));

            b_retval = (b_retval && 
                        l_decode(ADDR_OF(t_msg_5_dec_in[0u]), 
                                 ELEMS_OF(t_msg_5_dec_in), 
                                 ADDR_OF(t_msg_6_dec_out[0u]), 
                                 ELEMS_OF(t_msg_6_dec_out)));

            b_retval = (b_retval && 
                        memcmp(ADDR_OF(t_msg_6_dec_out[12u]), 
                               ADDR_OF(t_msg_3_fs_enc_out[0u]), 
                               SIZE_OF(t_msg_3_fs_enc_out)) == 0u);

            b_retval = (b_retval && 
                        l_enc_undo(ADDR_OF(t_msg_6_dec_out[12u]), 
                                   (ELEMS_OF(t_msg_6_dec_out) - 12u - 2u), 
                                   ADDR_OF(t_msg_7_fs_dec_out[0u]), 
                                   ELEMS_OF(t_msg_7_fs_dec_out)));

            b_retval = (b_retval && 
                        memcmp(ADDR_OF(t_msg_1_orig[0u]), 
                               ADDR_OF(t_msg_7_fs_dec_out[0u]), 
                               SIZE_OF(t_msg_1_orig)) == 0u);
         }
      }

      i++;
   }

   return b_retval;
}
#endif // if !defined(TEST)

int main(void)
{
   int retval = -1;

   l_print_ts("Start");

   l_gen_field();
   l_gen_add_sub();
   l_gen_mul();
   l_gen_div();

#if !defined(TEST)
   if (l_run())
   {
      retval = 0u;
   }
   else
   {
      FATAL();
   }
#else // if !defined(TEST)
   uint _r[P_R];
   uint _R[P_N];
   uint _M[P_K] = 
   {
      1u, 2u, 3u, 4u, 
      5u, 6u, 7u, 8u, 
      9u, 10u, 11u, 
#if !defined(DEBUG_DOC)
      12u, 13u, 14u, 15u, 
      16u, 17u, 18u, 19u, 
      20u, 21u, 22u, 23u, 
      24u, 25u, 26u, 27u, 
      28u, 29u
#endif // !defined(DEBUG_DOC)
   };

#if (defined(UNIT_TEST) && defined(DEBUG_DOC))
   // [q, r] = gfdeconv(gf2exp([0, 0, 0, 0, 1]), gf2exp([15, 3, 4, 12]), gftuple([-1:14]', 4, 2))
   // gfpretty(gf2dec(q))
   // gfpretty(gf2dec(r))

   // [q, r] = gfdeconv(gf2exp([15, 3, 4, 12]), gf2exp([4, 6, 6]), gftuple([-1:14]', 4, 2))
   // gfpretty(gf2dec(q))
   // gfpretty(gf2dec(r))

   // 1 | 0 = gf2dec(gfdeconv(gf2exp([15, 3, 4, 12]), gf2exp([15, 3, 4, 12]), gftuple([-1:14]', 4, 2)))
   // [8, 7, 5, 15] | 0 = gf2dec(gfdeconv(gf2exp([15, 3, 4, 12]), gf2exp([0u]), gftuple([-1:14]', 4, 2)))
   // [15, 3, 4, 12] | 0 = gf2dec(gfdeconv(gf2exp([15, 3, 4, 12]), gf2exp([1]), gftuple([-1:14]', 4, 2)))
   // 1 | 1 = gf2dec(gfdeconv(gf2exp([15, 3, 4, 12]), gf2exp([14, 3, 4, 12]), gftuple([-1:14]', 4, 2)))
   // 1 | 1 = gf2dec(gfdeconv(gf2exp([14, 3, 4, 12]), gf2exp([15, 3, 4, 12]), gftuple([-1:14]', 4, 2)))
   // 5 | [9, 12, 3] = gf2dec(gfdeconv(gf2exp([15, 3, 4, 12]), gf2exp([15, 3, 4, 13]), gftuple([-1:14]', 4, 2)))
   // 11 | [12, 13, 14] = gf2dec(gfdeconv(gf2exp([15, 3, 4, 13]), gf2exp([15, 3, 4, 12]), gftuple([-1:14]', 4, 2)))
   // 0 | [14, 3, 4, 12] = gf2dec(gfdeconv(gf2exp([14, 3, 4, 12]), gf2exp([1, 15, 3, 4, 12]), gftuple([-1:14]', 4, 2)))

   // Addition limits:
   // r: max(len(a),len(b))
   // Multiplication limits:
   // r: (len(a)+len(b)+1)
   // Division limits:
   // q: 1 - (len(dd)-len(dr)+1)
   // r: 0 - (len(dr)-1)

   const uint dd0[5] = {1u, 0u, 0u, 0u, 0u};
   const uint dr0[4] = {12u, 4u, 3u, 15};
   uint q0[ELEMS_OF(dd0)] = {50u, 51u, 52u, 53u};
   uint r0[ELEMS_OF(dd0)] = {90u, 91u, 92u, 93u, 94u};
   uint q0_e[ELEMS_OF(q0)] = {0u, 0u, 0u, 10u, 6u};
   uint r0_e[ELEMS_OF(r0)] = {0u, 0u, 6u, 6u, 4u};
   if (!l_div_p(0u, ADDR_OF(dd0[0u]), ELEMS_OF(dd0), ADDR_OF(dr0[0u]), ELEMS_OF(dr0), ADDR_OF(q0[0u]), ELEMS_OF(q0), ADDR_OF(r0[0u]), ELEMS_OF(r0)) || 
       memcmp(ADDR_OF(q0[0u]), ADDR_OF(q0_e[0u]), SIZE_OF(q0)) != 0 || 
       memcmp(ADDR_OF(r0[0u]), ADDR_OF(r0_e[0u]), SIZE_OF(r0)) != 0)
   {
      FATAL();
   }

   const uint dd1[15] = 
   {
      1u, 2u, 3u, 4u, 
      5u, 6u, 7u, 8u, 
      9u, 10u, 11u, 
      0u, 0u, 0u, 0u
   };
   const uint dr1[5] = 
   {
      1u, 15u, 3u, 1u, 12u
   };
   uint q1[ELEMS_OF(dd1)] = {50u, 51u, 52u, 53u, 54u, 55u, 56u, 57u, 58u, 59u, 60u};
   uint r1[ELEMS_OF(dd1)] = {90u, 91u, 92u, 93u, 94u};
   uint q1_e[ELEMS_OF(q1)] = {0u, 0u, 0u, 0u, 1u, 13u, 7u, 10u, 1u, 0u, 12u, 0u, 2u, 11u, 1u};
   uint r1_e[ELEMS_OF(r1)] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 3u, 3u, 12u, 12u};
   if (!l_div_p(0u, ADDR_OF(dd1[0u]), ELEMS_OF(dd1), ADDR_OF(dr1[0u]), ELEMS_OF(dr1), ADDR_OF(q1[0u]), ELEMS_OF(q1), ADDR_OF(r1[0u]), ELEMS_OF(r1)) || 
       memcmp(ADDR_OF(q1[0u]), ADDR_OF(q1_e[0u]), SIZE_OF(q1)) != 0 || 
       memcmp(ADDR_OF(r1[0u]), ADDR_OF(r1_e[0u]), SIZE_OF(r1)) != 0)
   {
      FATAL();
   }
#endif // if (defined(UNIT_TEST) && defined(DEBUG_DOC))

   l_print_a("Message= ", ADDR_OF(_M[0u]), ELEMS_OF(_M), 0u);

   if (l_encode(ADDR_OF(_M[0u]), ELEMS_OF(_M), ADDR_OF(_r[0u]), ELEMS_OF(_r)))
   {
      memset(ADDR_OF(_R[0u]), 0u, SIZE_OF(_R));
      memcpy(ADDR_OF(_R[0u]), ADDR_OF(_M[0u]), SIZE_OF(_M));
      memcpy(ADDR_OF(_R[P_K]), ADDR_OF(_r[0u]), SIZE_OF(_r));

      l_print_a("Codeword=", ADDR_OF(_R[0u]), ELEMS_OF(_R), 0u);

#if !defined(DEBUG_DOC)
#if (ERROR_TYPE == 0u)
#elif (ERROR_TYPE == 1u)
      l_add_e(_R[(P_N - 1u) - 9u], 23u, ADDR_OF(_R[(P_N - 1u) - 9u]));
#else // if (ERROR_TYPE == ..)
#error "Uknown ERROR_TYPE value"
#endif // else if (ERROR_TYPE == ..)
#else // #if !defined(DEBUG_DOC)
#if (ERROR_TYPE == 0u)
#elif (ERROR_TYPE == 1u)
      l_add_e(_R[(P_N - 1u) - 9u], 13u, ADDR_OF(_R[(P_N - 1u) - 9u]));
      l_add_e(_R[(P_N - 1u) - 2u], 2, ADDR_OF(_R[(P_N - 1u) - 2u]));
#elif (ERROR_TYPE == 2u)
      l_add_e(_R[(P_N - 1u) - 10u], 13u, ADDR_OF(_R[(P_N - 1u) - 10u]));
      l_add_e(_R[(P_N - 1u) - 2u], 2, ADDR_OF(_R[(P_N - 1u) - 2u]));
#elif (ERROR_TYPE == 3u)
      l_add_e(_R[(P_N - 1u) - 9u], 13u, ADDR_OF(_R[(P_N - 1u) - 9u]));
#elif (ERROR_TYPE == 4u)
      l_add_e(_R[(P_N - 1u) - 9u], 7u, ADDR_OF(_R[(P_N - 1u) - 9u]));
      l_add_e(_R[(P_N - 1u) - 2u], 2, ADDR_OF(_R[(P_N - 1u) - 2u]));
#elif (ERROR_TYPE == 5u)
      l_add_e(_R[(P_N - 1u) - 1u], 7u, ADDR_OF(_R[(P_N - 1u) - 1u]));
      l_add_e(_R[P_N - 1u], 9u, ADDR_OF(_R[P_N - 1u]));
#elif (ERROR_TYPE == 6u)
      l_add_e(_R[(P_N - 1u) - 10u], 13u, ADDR_OF(_R[(P_N - 1u) - 10u]));
      l_add_e(_R[(P_N - 1u) - 9u], 3u, ADDR_OF(_R[(P_N - 1u) - 9u]));
      l_add_e(_R[(P_N - 1u) - 8u], 2u, ADDR_OF(_R[(P_N - 1u) - 8u]));
      l_add_e(_R[(P_N - 1u) - 7u], 1u, ADDR_OF(_R[(P_N - 1u) - 7u]));
      l_add_e(_R[(P_N - 1u) - 2u], 2, ADDR_OF(_R[(P_N - 1u) - 2u]));
#else // if (ERROR_TYPE == ..)
#error "Uknown ERROR_TYPE value"
#endif // else if (ERROR_TYPE == ..)
#endif // else #if !defined(DEBUG_DOC)

      l_print_a("Received=", ADDR_OF(_R[0u]), ELEMS_OF(_R), 0u);

      if (l_decode(ADDR_OF(_R[0u]), ELEMS_OF(_R), NULL, 0u))
      {
         retval = 0;
      }
   }
#endif // else if !defined(TEST)

   l_print_ts("Stop");

   return retval;
}
