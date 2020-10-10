#if !defined(__PMA_H__)
#define __PMA_H__

typedef enum e_pma_state 
{
   PMS_UNDEFINED = 0u, 

   PMS_Z = 1u, 
   PMS_0 = 2u, 
   PMS_1 = 3u, 

   _PMS_MIN = PMS_Z, 
   _PMS_MAX = PMS_1, 
   _PMS_CNT = ((_PMS_MAX - _PMS_MIN) + 1u)
} E_pma_state;

typedef struct s_pma_state 
{
   // b_reset: pma_reset
   // true: ON
   // false: OFF
   bool                    b_reset;
} S_pma_state;

void INIT_pma(uint pid);
void RESET_pma(uint pid, bool b_assert);

#endif // if !defined(__PMA_H__)
