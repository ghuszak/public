#if !defined(__PLCA_STATUS_H__)
#define __PLCA_STATUS_H__

typedef enum e_plca_status_state 
{
   _PSS_UNDEFINED = 0u, 

   PSS_INACTIVE, 
   PSS_ACTIVE, 
   PSS_HYSTERESIS, 

   _PSS_MIN = PSS_INACTIVE, 
   _PSS_MAX = PSS_HYSTERESIS, 
   _PSS_CNT = ((_PSS_MAX- -_PSS_MIN) + 1u)
} E_plca_status_state;

typedef struct s_plca_status_state 
{
   E_plca_status_state     state;
   bool                    b_reset_prev;

   // b_ok: plca_status
   // - true: OK
   // - false: FAIL
   bool                    b_ok;
} S_plca_status_state;

void INIT_plca_status(uint pid);
void CLK_plca_status(uint pid);

#endif // if !defined(__PLCA_STATUS_H__)
