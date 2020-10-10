#if !defined(__SCRAMBLING_H__)
#define __SCRAMBLING_H__

typedef struct s_scrambling_state 
{
   uint                    s_lfsr;
   uint                    d_lfsr;
} S_scrambling_state;

void INIT_scrambling(uint pid);
void RESET_scrambling(uint pid);

sym4b scramble(uint pid, sym4b data);
sym4b descramble(uint pid, sym4b data);

#endif // if !defined(__SCRAMBLING_H__)
