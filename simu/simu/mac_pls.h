#if !defined(__MAC_PLS_H__)
#define __MAC_PLS_H__

typedef struct s_mac_pls_states 
{
   bool                    b_reset;

   S_mii_signals_tx        mii_tx;
} S_mac_pls_states;

void INIT_mac_pls(uint pid);

#endif // if !defined(__MAC_PLS_H__)
