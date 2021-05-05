/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                           MPC57xx VLE Port
*                                             GNU Toolchain
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  "../../../../Source/os.h"
#include  "os_cpu.h"


/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*
* Note(s) : (1) C++ compilers MUST 'extern'ally declare ALL C function prototypes & variable/object
*               declarations for correct C language linkage.
*********************************************************************************************************
*/

#ifdef __cplusplus
extern  "C" {                                                   /* See Note #1.                                         */
#endif


/*
*********************************************************************************************************
*                                               GLOBALS
*********************************************************************************************************
*/

        OS_STK   OS_CPU_ISRStk[OS_CPU_ISR_STK_SIZE];
        OS_STK  *OS_CPU_ISRStkBase;                             /* 8-byte aligned base of the ISR stack.                */
        INT32U   OS_CPU_ISRNestingCtr;                          /* Total Nesting Counter: Kernel Aware and Fast IRQs    */

extern  char     _SDA_BASE_[];                                  /* Defined by the linker, as per the PPC System V ABI.  */
extern  char     _SDA2_BASE_[];                                 /* Defined by the linker, as per the PPC EABI.          */


/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  INT16U  OSTmrCtr;
#endif


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : None.
*
* Note(s)    : (1) This function initializes the ISR stack.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSInitHookBegin (void)
{
    INT32U   i;
    OS_STK  *p_stk;


    p_stk = &OS_CPU_ISRStk[0u];                                 /* Clear the ISR stack                                  */
    for (i = 0u; i < OS_CPU_ISR_STK_SIZE; i++) {
        *p_stk++ = (OS_STK)0u;
    }

    p_stk = &OS_CPU_ISRStk[OS_CPU_ISR_STK_SIZE-1u];
    p_stk = (OS_STK *)((INT32U)p_stk & 0xFFFFFFF8u);            /* Align top of stack to 8-bytes (EABI).                */

    OS_CPU_ISRStkBase    = p_stk;
    OS_CPU_ISRNestingCtr = 0;

#if OS_TMR_EN > 0u
    OSTmrCtr = 0u;
#endif
}
#endif


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSInitHookEnd (void)
{

}
#endif


/*
*********************************************************************************************************
*                                         TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : p_tcb        Pointer to the task control block of the task being created.
*
* Note(s)    : None.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskCreateHook (OS_TCB  *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskCreateHook(p_tcb);
#else
    (void)p_tcb;                                                /* Prevent compiler warning                             */
#endif
}
#endif


/*
*********************************************************************************************************
*                                         TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : p_tcb        Pointer to the task control block of the task being deleted.
*
* Note(s)    : None.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskDelHook (OS_TCB  *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskDelHook(p_tcb);
#else
    (void)p_tcb;                                                /* Prevent compiler warning                             */
#endif
}
#endif


/*
*********************************************************************************************************
*                                           IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskIdleHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskIdleHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                          TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : p_tcb        Pointer to the task control block of the task that is returning.
*
* Note(s)    : None.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskReturnHook (OS_TCB  *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskReturnHook(p_tcb);
#else
    (void)p_tcb;                                                /* Prevent compiler warning                             */
#endif
}
#endif


/*
*********************************************************************************************************
*                                         STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : None.
*
* Note(s)    : None.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTaskStatHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskStatHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                      INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : p_task       Pointer to the task entry point address.
*
*              p_arg        Pointer to a user supplied data area that will be passed to the task
*                           when the task first executes.
*
*              p_tos        Pointer to the top of stack.  It is assumed that 'p_tos' points to
*                           a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                           'p_tos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                           OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                           of the stack.
*
*              opt          Options used to alter the behavior of OSTaskStkInit().
*                           (see uCOS_II.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when task starts executing.
*
*              2) The stack frame used to save and restore context is shown below:
*
*                                 Low Address +-----------+ Top of Stack
*                                             | Backchain |
*                                             +-----------+
*                                             |  R1 (LR)  |
*                                             +-----------+
*                                             |    MSR    |
*                                             +-----------+
*                                             |   SRR0    |
*                                             +-----------+
*                                             |   SRR1    |
*                                             +-----------+
*                                             |    CTR    |
*                                             +-----------+
*                                             |    XER    |
*                                             +-----------+
*                                             |  SPEFSCR  |
*                                             +-----------+
*                                             |    R0     |
*                                             +-----------+
*                                             |    R2     |
*                                             +-----------+
*                                             |    R3     |
*                                             +-----------+
*                                                  ...
*                                             +-----------+
*                                             |    R31    |
*                                High Address +-----------+ Bottom of Stack
*
*
*
*              3) The back chain always points to the location of the previous frame's back chain.
*                 The first stack frame shall contain a null back chain, as per the PPC ABI.
*
*                                    Frame 1  +-----------+
*                                             | Backchain | ------+
*                                             +-----------+       |
*                                             |           |       |
*                                             | REGISTERS |       |
*                                             |           |       |
*                                             +-----------+       |
*                                                                 |
*                                                                 |
*                                    Frame 0  +-----------+       |
*                                             |   NULL    | <<----+
*                                             +-----------+
*                                             |  R1 (LR)  |
*                                             +-----------+
*
**********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void    (*p_task)(void *p_arg),
                        void     *p_arg,
                        OS_STK   *p_tos,
                        INT16U    opt)
{
    OS_STK  *p_stk;


    (void)opt;                                                  /* Prevent compiler warning                             */
                                                                /* ---------------------------------------------------- */
                                                                /* --------------- INITIAL STACK FRAME ---------------- */
                                                                /* ---------------------------------------------------- */
    p_stk = p_tos - 1u;                                         /* Initial Stack is 2 words: LR and Backchain.          */

                                                                /* Align top of stack to 8-bytes (EABI).                */
    p_stk = (OS_STK *)((INT32U)p_stk & 0xFFFFFFF8u);

    *(p_stk+1) = 0x00000000u;                                   /* LR: null for the initial frame.                      */
    *p_stk--   = 0x00000000u;                                   /* Backchain: null for the initial frame.               */

                                                                /* ---------------------------------------------------- */
                                                                /* ----------------- TASK STACK FRAME ----------------- */
                                                                /* ---------------------------------------------------- */
    *p_stk-- = 0x1F1F1F1Fu;                                     /* R31                                                  */
    *p_stk-- = 0x1E1E1E1Eu;                                     /* R30                                                  */
    *p_stk-- = 0x1D1D1D1Du;                                     /* R29                                                  */
    *p_stk-- = 0x1C1C1C1Cu;                                     /* R28                                                  */
    *p_stk-- = 0x1B1B1B1Bu;                                     /* R27                                                  */
    *p_stk-- = 0x1A1A1A1Au;                                     /* R26                                                  */
    *p_stk-- = 0x19191919u;                                     /* R25                                                  */
    *p_stk-- = 0x18181818u;                                     /* R24                                                  */
    *p_stk-- = 0x17171717u;                                     /* R23                                                  */
    *p_stk-- = 0x16161616u;                                     /* R22                                                  */
    *p_stk-- = 0x15151515u;                                     /* R21                                                  */
    *p_stk-- = 0x14141414u;                                     /* R20                                                  */
    *p_stk-- = 0x13131313u;                                     /* R19                                                  */
    *p_stk-- = 0x12121212u;                                     /* R18                                                  */
    *p_stk-- = 0x11111111u;                                     /* R17                                                  */
    *p_stk-- = 0x10101010u;                                     /* R16                                                  */
    *p_stk-- = 0x0F0F0F0Fu;                                     /* R15                                                  */
    *p_stk-- = 0x0E0E0E0Eu;                                     /* R14                                                  */
    *p_stk-- = (INT32U)_SDA_BASE_;                              /* R13                                                  */
    *p_stk-- = 0x0C0C0C0Cu;                                     /* R12                                                  */
    *p_stk-- = 0x0B0B0B0Bu;                                     /* R11                                                  */
    *p_stk-- = 0x0A0A0A0Au;                                     /* R10                                                  */
    *p_stk-- = 0x09090909u;                                     /* R9                                                   */
    *p_stk-- = 0x08080808u;                                     /* R8                                                   */
    *p_stk-- = 0x07070707u;                                     /* R7                                                   */
    *p_stk-- = 0x06060606u;                                     /* R6                                                   */
    *p_stk-- = 0x05050505u;                                     /* R5                                                   */
    *p_stk-- = 0x04040404u;                                     /* R4                                                   */
    *p_stk-- = (INT32U)p_arg;                                   /* R3: Task Argument                                    */
    *p_stk-- = (INT32U)_SDA2_BASE_;                             /* R2                                                   */
    *p_stk-- = 0x00000000u;                                     /* R0                                                   */

    *p_stk-- = 0x00000000u;                                     /* Condition Register                                   */

    *p_stk-- = 0x00000000u;                                     /* SPEFSCR                                              */
    *p_stk-- = 0x00000000u;                                     /* XER                                                  */
    *p_stk-- = 0x00000000u;                                     /* CTR                                                  */
    *p_stk-- = 0x00008000u;                                     /* SRR1: External Interrupts enabled after RFI.         */
    *p_stk-- = (INT32U)p_task;                                  /* SRR0: PC after RFI.                                  */
    *p_stk-- = 0x00000000u;                                     /* MSR: Interrupts disabled until RFI.                  */

    *p_stk-- = (INT32U)OS_TaskReturn;                           /* LR used to catch a task return.                      */
    *p_stk   = (INT32U)p_stk + (40u * 4u);                      /* Backchain                                            */

    return(p_stk);
}


/*
*********************************************************************************************************
*                                          TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : None.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_TASK_SW_HOOK_EN > 0u)
void  OSTaskSwHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskSwHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                          OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : p_tcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSTCBInitHook (OS_TCB *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TCBInitHook(p_tcb);
#else
    (void)p_tcb;                                                /* Prevent compiler warning                             */
#endif
}
#endif


/*
*********************************************************************************************************
*                                              TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : None.
*
* Note(s)    : 1) This function is assumed to be called from the Tick ISR.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_TIME_TICK_HOOK_EN > 0u)
void  OSTimeTickHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TimeTickHook();
#endif

#if OS_TMR_EN > 0u
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0u;
        OSTmrSignal();
    }
#endif
}
#endif


/*
*********************************************************************************************************
*                                   EXTERNAL C LANGUAGE LINKAGE END
*********************************************************************************************************
*/

#ifdef __cplusplus
}                                                               /* End of 'extern'al C lang linkage.                    */
#endif
