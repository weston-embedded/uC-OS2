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
*                                   Freescale MPC8349E Specific code
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS


/*
*********************************************************************************************************
*                                             INCLUDES
*********************************************************************************************************
*/

#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                             GLOBALS     CEDRIC : A VERIFIER (EABI)
*********************************************************************************************************
*/

extern  char    _SDA_BASE_[];
extern  char    _SDA2_BASE_[];


/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if      (OS_VERSION >= 281) && (OS_TMR_EN > 0)
  static  INT16U  OSTmrCtr;
#endif                                                                  /* #if (OS_VERSION >= 281) && (OS_TMR_EN > 0)               */



/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_VERSION > 203)
void  OSInitHookBegin (void)
{
#if (OS_VERSION >= 281) && (OS_TMR_EN > 0)
    OSTmrCtr = 0;
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
#if (OS_CPU_HOOKS_EN > 0) && (OS_VERSION > 203)
void  OSInitHookEnd (void)
{
}
#endif


/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void  OSTaskCreateHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                                         /* Prevent compiler warning                                 */
#endif
}
#endif


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void  OSTaskDelHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                                         /* Prevent compiler warning                                 */
#endif
}
#endif


/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_VERSION >= 251)
void  OSTaskIdleHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskIdleHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void  OSTaskStatHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskStatHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : (1) The SRR1 Register holds the original value of the MSR register. This value is copied
*                  to the MSR register by execute the rfi operation at the end of the ISR.
*
*              (2) Ensure any changes to the order of stack initialization is congruent with the Stack
*                  Frame definitions in OS_CPU_A.s.
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    STK *       stkp;
    OS_CPU_SR   msr;                                                    /* Local: Initial MSR                                       */
    OS_CPU_SR   srr1;                                                   /* Local: Initial SRR1                                      */

    (void)opt;                                                          /* 'opt' is not used, prevent warning                       */

    msr     =  CPU_SR_Rd();                                             /* get the MSB reg value                                    */
    srr1    = (msr | 0x8000);                                           /* set MSR[EE] bit to enable interrupts                     */

                                                                        /* Leave buffer area for locals "above the stack" in ...    */
    ptos   -= OS_STK_RSVD_SIZE;                                         /* case the compiler prolog puts variables above the stack  */
    stkp    = (STK *)((INT32U) ptos & (INT32U) 0xFFFFFFF0);          /* 16-byte align task's stack pointer (EABI)                */

    stkp--;                                                             /* set structure within stack space         */

    /*****************************************************************/
    /* Ensure any changes to the order of these initializations      */
    /* are congruent with the Stack Frame definitions in OS_CPU_A.s  */
    /*****************************************************************/
                                                                        /* to improve debugging, fill all registers with the name */
    stkp->R31 = 0x31L;                                                  /* R31 */
    stkp->R30 = 0x30L;                                                  /* R30 */
    stkp->R29 = 0x29L;                                                  /* R29 */
    stkp->R28 = 0x28L;                                                  /* R28 */
    stkp->R27 = 0x27L;                                                  /* R27 */
    stkp->R26 = 0x26L;                                                  /* R26 */
    stkp->R25 = 0x25L;                                                  /* R25 */
    stkp->R24 = 0x24L;                                                  /* R24 */
    stkp->R23 = 0x23L;                                                  /* R23 */
    stkp->R22 = 0x22L;                                                  /* R22 */
    stkp->R21 = 0x21L;                                                  /* R21 */
    stkp->R20 = 0x20L;                                                  /* R20 */
    stkp->R19 = 0x19L;                                                  /* R19 */
    stkp->R18 = 0x18L;                                                  /* R18 */
    stkp->R17 = 0x17L;                                                  /* R17 */
    stkp->R16 = 0x16L;                                                  /* R16 */
    stkp->R15 = 0x15L;                                                  /* R15 */
    stkp->R14 = 0x14L;                                                  /* R14 */
    stkp->R13 = (INT32U)&_SDA_BASE_;                                    /* R13 */
    stkp->R12 = 0x12L;                                                  /* R12 */
    stkp->R11 = 0x11L;                                                  /* R11 */
    stkp->R10 = 0x10L;                                                  /* R10 */
    stkp->R09 = 0x09L;                                                  /* R9 */
    stkp->R08 = 0x08L;                                                  /* R8 */
    stkp->R07 = 0x07L;                                                  /* R7 */
    stkp->R06 = 0x06L;                                                  /* R6 */
    stkp->R05 = 0x05L;                                                  /* R5 */
    stkp->R04 = 0x04L;                                                  /* R4 */
    stkp->R03 = (INT32U)p_arg;                                          /* R3 */
    stkp->R02 = (INT32U)&_SDA2_BASE_;                                   /* R2 */
    stkp->R00 = 0x00L;                                                  /* R0 */

 #ifdef OS_SAVE_CONTEXT_WITH_FPRS
    stkp->FPSCR = FPSCR_INIT;
    stkp->F31 = 0;                                                      /* F31 */
    stkp->F30 = 0;                                                      /* F30 */
    stkp->F29 = 0;                                                      /* F29 */
    stkp->F28 = 0;                                                      /* F28 */
    stkp->F27 = 0;                                                      /* F27 */
    stkp->F26 = 0;                                                      /* F26 */
    stkp->F25 = 0;                                                      /* F25 */
    stkp->F24 = 0;                                                      /* F24 */
    stkp->F23 = 0;                                                      /* F23 */
    stkp->F22 = 0;                                                      /* F22 */
    stkp->F21 = 0;                                                      /* F21 */
    stkp->F20 = 0;                                                      /* F20 */
    stkp->F19 = 0;                                                      /* F19 */
    stkp->F18 = 0;                                                      /* F18 */
    stkp->F17 = 0;                                                      /* F17 */
    stkp->F16 = 0;                                                      /* F16 */
    stkp->F15 = 0;                                                      /* F15 */
    stkp->F14 = 0;                                                      /* F14 */
    stkp->F13 = 0;                                                      /* F13 */
    stkp->F12 = 0;                                                      /* F12 */
    stkp->F11 = 0;                                                      /* F11 */
    stkp->F10 = 0;                                                      /* F10 */
    stkp->F09 = 0;                                                      /* F9 */
    stkp->F08 = 0;                                                      /* F8 */
    stkp->F07 = 0;                                                      /* F7 */
    stkp->F06 = 0;                                                      /* F6 */
    stkp->F05 = 0;                                                      /* F5 */
    stkp->F04 = 0;                                                      /* F4 */
    stkp->F03 = 0;                                                      /* F3 */
    stkp->F02 = 0;                                                      /* F2 */
    stkp->F01 = 0;                                                      /* F1 */
    stkp->F00 = 0;                                                      /* F0 */
#endif
    stkp->CR     = 0;
    stkp->CTR_   = 0;
    stkp->XER_   = 0;
    stkp->CSRR0_ = 0;
    stkp->CSRR1_ = 0;

    stkp->LR_   = (INT32U)task;                                         /* LR */
    stkp->BLK   = 0;                                                    /* BLANK for 0xA8 size */
    stkp->SRR0_ = (INT32U)task;                                         /* SRRO */
    stkp->SRR1_ = (INT32U)srr1;                                         /* SRR1 */
    stkp->R01   = (INT32U)ptos;                                         /* R1 */

    return (OS_STK *)stkp;
}


/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void  OSTaskSwHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskSwHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_VERSION > 203)
void  OSTCBInitHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                                         /* Prevent compiler warning                                 */
#endif
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TimeTickHook();
#endif

#if (OS_VERSION >= 281) && (OS_TMR_EN > 0)
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif
}
#endif
