#include "states.h"

typedef struct s_canaries 
{
   uint32                  canary_1;
   uint32                  canary_2;
   uint32                  canary_3;
   uint32                  canary_4;
   uint32                  canary_5;
   uint32                  canary_6;
   uint32                  canary_7;
   uint32                  canary_8;
   uint32                  canary_9;
   uint32                  canary_10;
   uint32                  canary_11;
   uint32                  canary_12;
   uint32                  canary_13;
   uint32                  canary_14;
} S_canaries;

S_states g_states;

const S_canaries lC_canaries[PHY_CNT] = 
{
   {/*.canary_1 = */0xee7778f6u, /*.canary_2 = */0xba621133u, /*.canary_3 = */0x440996edu, /*.canary_4 = */0x8e6eb312u, 
    /*.canary_5 = */0x3b0245b0u, /*.canary_6 = */0x786749ccu, /*.canary_7 = */0x58ba8c6eu, /*.canary_8 = */0x890aa114u, 
    /*.canary_9 = */0x642b49b3u, /*.canary_10 = */0x19b2f601u, /*.canary_11 = */0x4d114964u, /*.canary_12 = */0x1dd264c3u, 
    /*.canary_13 = */0x1c1c3fd4u, /*.canary_14 = */0x32ab440fu},
#if (PHY_CNT > 1)
   {/*.canary_1 = */0xa848ef37u, /*.canary_2 = */0xa8df48d5u, /*.canary_3 = */0x602885d3u, /*.canary_4 = */0x6827b4bbu, 
    /*.canary_5 = */0x06f2afe6u, /*.canary_6 = */0xd05e2947u, /*.canary_7 = */0xc12b99dfu, /*.canary_8 = */0x2b64af06u, 
    /*.canary_9 = */0xd75598a7u, /*.canary_10 = */0x6ad1031fu, /*.canary_11 = */0x4bd5454fu, /*.canary_12 = */0x2f305762u,
    /*.canary_13 = */0x69ed4218u, /*.canary_14 = */0x3fbc13f2u},
#if (PHY_CNT > 2)
   {/*.canary_1 = */0x3340b759u, /*.canary_2 = */0x380f8b52u, /*.canary_3 = */0xef530be4u, /*.canary_4 = */0x3eb6fe05u, 
    /*.canary_5 = */0x0c3e1077u, /*.canary_6 = */0x73e7afa5u, /*.canary_7 = */0xd6c93a7au, /*.canary_8 = */0x16dca8eeu, 
    /*.canary_9 = */0xfbb5de47u, /*.canary_10 = */0x6e6d62afu, /*.canary_11 = */0x5efc5e2cu, /*.canary_12 = */0x244b7b98u,
    /*.canary_13 = */0x5eef35a7u, /*.canary_14 = */0x1f915d29u},
#if (PHY_CNT > 3)
   {/*.canary_1 = */0x85373758u, /*.canary_2 = */0x7dba4c6fu, /*.canary_3 = */0xa2782c37u, /*.canary_4 = */0xb1532a70u, 
    /*.canary_5 = */0xc2b98fd1u, /*.canary_6 = */0x41630cceu, /*.canary_7 = */0x10ff911cu, /*.canary_8 = */0xa61cb0ecu, 
    /*.canary_9 = */0xb2e1152eu, /*.canary_10 = */0x9c6b64efu, /*.canary_11 = */0x68fd394au, /*.canary_12 = */0x1c4e537eu,
    /*.canary_13 = */0x2bd22f45u, /*.canary_14 = */0x66f908f3u},
#if (PHY_CNT > 4)
   {/*.canary_1 = */0xe318ef6bu, /*.canary_2 = */0x430d970au, /*.canary_3 = */0x0266eb21u, /*.canary_4 = */0x879d34a6u, 
    /*.canary_5 = */0x581faa98u, /*.canary_6 = */0xc6ae4dc1u, /*.canary_7 = */0xa60f6497u, /*.canary_8 = */0x7ac21855u, 
    /*.canary_9 = */0x2fc5bc80u, /*.canary_10 = */0xc0e41927u, /*.canary_11 = */0x06b77814u, /*.canary_12 = */0x14d858cdu,
    /*.canary_13 = */0x51224daau, /*.canary_14 = */0x7d9a4cacu},
#if (PHY_CNT > 5)
   {/*.canary_1 = */0x12f55ca3u, /*.canary_2 = */0xed6279cbu, /*.canary_3 = */0xdf40a372u, /*.canary_4 = */0xea5eea66u, 
    /*.canary_5 = */0x23a84193u, /*.canary_6 = */0x3447be64u, /*.canary_7 = */0x92ce79a2u, /*.canary_8 = */0x465505c1u, 
    /*.canary_9 = */0xea1dd39du, /*.canary_10 = */0x49f9f19fu, /*.canary_11 = */0x5b983c74u, /*.canary_12 = */0x73f241f7u,
    /*.canary_13 = */0x78211e07u, /*.canary_14 = */0x55d969c7u},
#if (PHY_CNT > 6)
   {/*.canary_1 = */0xe92e79ccu, /*.canary_2 = */0x37f4d171u, /*.canary_3 = */0x47fe8cc2u, /*.canary_4 = */0x3f5c49a2u, 
    /*.canary_5 = */0xfbb9feb5u, /*.canary_6 = */0x1090d78du, /*.canary_7 = */0x5e3033a2u, /*.canary_8 = */0xf5cb2b0eu, 
    /*.canary_9 = */0xfaea535du, /*.canary_10 = */0x8408f8f2u, /*.canary_11 = */0x22952fa8u, /*.canary_12 = */0x402f566du,
    /*.canary_13 = */0x7a2e6615u, /*.canary_14 = */0x08e73e5fu},
#if (PHY_CNT > 7)
   {/*.canary_1 = */0x37440152u, /*.canary_2 = */0xfebf549eu, /*.canary_3 = */0x080c878au, /*.canary_4 = */0xd585c06au, 
    /*.canary_5 = */0xe5f3afefu, /*.canary_6 = */0x91b1df39u, /*.canary_7 = */0x70b6c4d6u, /*.canary_8 = */0xc74b071eu, 
    /*.canary_9 = */0x3bbf4065u, /*.canary_10 = */0xa940c85du, /*.canary_11 = */0x107f2700u, /*.canary_12 = */0x65b454e3u, 
    /*.canary_13 = */0x10072b22u, /*.canary_14 = */0x4a2920b3u}
#endif // if (PHY_CNT > 7)
#endif // if (PHY_CNT > 6)
#endif // if (PHY_CNT > 5)
#endif // if (PHY_CNT > 4)
#endif // if (PHY_CNT > 3)
#endif // if (PHY_CNT > 2)
#endif // if (PHY_CNT > 1)
};

void INIT_states(void)
{
   uint i;

   memset(ADDR_OF(g_states), 0u, SIZE_OF(g_states));

   for (i = 0u; i < PHY_CNT; i++)
   {
      g_states.stations[i].canary_1 = lC_canaries[i].canary_1;
      g_states.stations[i].canary_2 = lC_canaries[i].canary_2;
      g_states.stations[i].canary_3 = lC_canaries[i].canary_3;
      g_states.stations[i].canary_4 = lC_canaries[i].canary_4;
      g_states.stations[i].canary_5 = lC_canaries[i].canary_5;
      g_states.stations[i].canary_6 = lC_canaries[i].canary_6;
      g_states.stations[i].canary_7 = lC_canaries[i].canary_7;
      g_states.stations[i].canary_8 = lC_canaries[i].canary_8;
      g_states.stations[i].canary_9 = lC_canaries[i].canary_9;
      g_states.stations[i].canary_10 = lC_canaries[i].canary_10;
      g_states.stations[i].canary_11 = lC_canaries[i].canary_11;
      g_states.stations[i].canary_12 = lC_canaries[i].canary_12;
      g_states.stations[i].canary_13 = lC_canaries[i].canary_13;
      g_states.stations[i].canary_14 = lC_canaries[i].canary_14;
   }
}

bool CHECK_states(void)
{
#if defined(_DEBUG)
   const S_station_states *p_states = ADDR_OF(g_states.stations[0]);
   bool                   b_retval;
   uint                   i = 0u;

   while (i < PHY_CNT && 
          p_states->canary_1 == lC_canaries[i].canary_1 && 
          p_states->canary_2 == lC_canaries[i].canary_2 && 
          p_states->canary_3 == lC_canaries[i].canary_3 && 
          p_states->canary_4 == lC_canaries[i].canary_4 && 
          p_states->canary_5 == lC_canaries[i].canary_5 && 
          p_states->canary_6 == lC_canaries[i].canary_6 && 
          p_states->canary_7 == lC_canaries[i].canary_7 && 
          p_states->canary_8 == lC_canaries[i].canary_8 && 
          p_states->canary_9 == lC_canaries[i].canary_9 && 
          p_states->canary_10 == lC_canaries[i].canary_10 && 
          p_states->canary_11 == lC_canaries[i].canary_11 && 
          p_states->canary_12 == lC_canaries[i].canary_12 && 
          p_states->canary_13 == lC_canaries[i].canary_13 && 
          p_states->canary_14 == lC_canaries[i].canary_14)
   {
      p_states++;
      i++;
   }

   b_retval= (i == PHY_CNT);

   ASSERT(b_retval, i);

   return b_retval;
#else // #if defined(_DEBUG)
   return true;
#endif // else #if defined(_DEBUG)
}
