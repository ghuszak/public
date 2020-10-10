#if !defined(__4B5B_H__)
#define __4B5B_H__

typedef enum e_s5bs 
{
   S5B_UNUSED_00000 = 0u, 
   S5B_UNUSED_00001 = 1u, 
   S5B_UNUSED_00010 = 2u, 
   S5B_UNUSED_00011 = 3u, 
   // S5B_SSD: H
   S5B_SSD = 4u, 
   S5B_UNUSED_00101 = 5u, 
   S5B_UNUSED_00110 = 6u, 
   // S5B_ESDOK_ESDBRS: R
   S5B_ESDOK_ESDBRS = 7u, 
   S5B_ESDOK = S5B_ESDOK_ESDBRS, 
   S5B_ESDBRS = S5B_ESDOK_ESDBRS, 
   // S5B_BEACON: N
   S5B_BEACON = 8u, 
   // S5B_1
   S5B_1 = 9u, 
   // S5B_4
   S5B_4 = 10u, 
   // S5B_5
   S5B_5 = 11u, 
   S5B_UNUSED_01100 = 12u, 
   // S5B_ESD_HB: T
   S5B_ESD_HB = 13u, 
   S5B_ESD = S5B_ESD_HB, 
   S5B_HB = S5B_ESD_HB, 
   // S5B_6
   S5B_6 = 14u, 
   // S5B_7
   S5B_7 = 15u, 
   S5B_UNUSED_10000 = 16u, 
   // S5B_ESDERR: K
   S5B_ESDERR = 17u, 
   // S5B_8
   S5B_8 = 18u, 
   // S5B_9
   S5B_9 = 19u, 
   // S5B_2
   S5B_2 = 20u, 
   // S5B_3
   S5B_3 = 21u, 
   // S5B_A
   S5B_A = 22u, 
   // S5B_B
   S5B_B = 23u, 
   // S5B_SYNC_COMMIT: J
   S5B_SYNC_COMMIT = 24u, 
   S5B_SYNC = S5B_SYNC_COMMIT, 
   S5B_COMMIT = S5B_SYNC_COMMIT, 
   // S5B_ESDJAB: S
   S5B_ESDJAB = 25u, 
   // S5B_C
   S5B_C = 26u, 
   // S5B_D
   S5B_D = 27u, 
   // S5B_E
   S5B_E = 28u, 
   // S5B_F
   S5B_F = 29u, 
   // S5B_0
   S5B_0 = 30u, 
   // S5B_SILENCE: I
   S5B_SILENCE = 31u, 

   _S5B_MIN = S5B_UNUSED_00000, 
   _S5B_MAX = S5B_SILENCE, 
   _S5B_CNT = ((_S5B_MAX - _S5B_MIN) + 1u)
} E_s5bs;

sym4b conv_5b_to_4b(uint pid, sym5b sym, bool *p_valid);
const char *get_name_5b(uint pid, sym5b sym);
sym5b conv_4b_to_5b(uint pid, sym4b sym);
bool is_5b_valid(sym5b sym5b);

#endif // if !defined(__4B5B_H__)
