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
*                                    Freescale MPC56xx Specific code
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
*                                             GLOBALS
*********************************************************************************************************
*/

extern  char    _SDA_BASE_[];
extern  char    _SDA2_BASE_[];


/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if (OS_VERSION >= 29200u) && (OS_TMR_EN > 0u)
static  INT16U  OSTmrCtr;
#endif                                                          /* #if (OS_VERSION >= 29200) && (OS_TMR_EN > 0)         */


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

#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION > 29200u)
void  OSInitHookBegin (void)
{
#if (OS_VERSION >= 29200u) && (OS_TMR_EN > 0u)
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

#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION > 29200u)
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

#if OS_CPU_HOOKS_EN > 0u
void  OSTaskCreateHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
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

#if OS_CPU_HOOKS_EN > 0u
void  OSTaskDelHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
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

#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION >= 29200u)
void  OSTaskIdleHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskIdleHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                            TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : ptcb      is a pointer to the task control block of the task that is returning.
*
* Note(s)    : none
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0u
void  OSTaskReturnHook (OS_TCB  *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskReturnHook(ptcb);
#else
    (void)ptcb;
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

OS_STK  *OSTaskStkInit (void (*task)(void  *pd),
                        void    *p_arg,
                        OS_STK  *ptos,
                        INT16U   opt)
{
    OS_STK     *p_stk;                                          /* Local: Stack pointer                                 */
    OS_CPU_SR   msr;                                            /* Local: Initial MSR                                   */
    OS_CPU_SR   srr1;                                           /* Local: Initial SRR1                                  */


    (void)opt;                                                  /* 'opt' is not used, prevent warning                   */

    msr    = OS_CPU_SR_Rd();                                    /* Get the MSB Reg value                                */
    srr1   = msr | 0x8000u;                                     /* Set MSR[EE] bit to enable interrupts                 */

    p_stk  = (OS_STK *)((INT32U)ptos & 0xFFFFFFF0u);            /* 16-byte align task's stack pointer (EABI)            */

                                                                /* Leave buffer area for locals "above the stack" in ...*/
    p_stk -= OS_STK_RSVD_SIZE;                                  /* Case the compiler prolog puts Var. above the stack   */

                                                                /* Stack Frame Initialization                           */
    *--p_stk = (INT32U)msr;                                     /* MSR                                                  */
    *--p_stk = 0u;                                              /* EABI padding                                         */
    *--p_stk = 0u;                                              /* EABI padding                                         */
    *--p_stk = 0u;                                              /* SPEFSCR                                              */
    *--p_stk = (INT32U)task;                                    /* LR                                                   */
    *--p_stk = 0u;                                              /* CR                                                   */
    *--p_stk = 0u;                                              /* XER                                                  */
    *--p_stk = 0u;                                              /* CTR                                                  */
    *--p_stk = 0u;                                              /* USPRG0                                               */
    *--p_stk = (INT32U)srr1;                                    /* SRR1                                                 */
    *--p_stk = (INT32U)task;                                    /* SRR0                                                 */
    *--p_stk = 0u;                                              /* R0                                                   */

#ifdef OS_SAVE_CONTEXT_WITH_FPRS                                /* Push 64-bit Register's Initial Value to the Stack    */
    *--p_stk = 0x31uL;                                          /* R31                                                  */
    *--p_stk = 0x3100uL;
    *--p_stk = 0x30uL;                                          /* R30                                                  */
    *--p_stk = 0x3000uL;
    *--p_stk = 0x29uL;                                          /* R29                                                  */
    *--p_stk = 0x2900uL;
    *--p_stk = 0x28uL;                                          /* R28                                                  */
    *--p_stk = 0x2800uL;
    *--p_stk = 0x27uL;                                          /* R27                                                  */
    *--p_stk = 0x2700uL;
    *--p_stk = 0x26uL;                                          /* R26                                                  */
    *--p_stk = 0x2600uL;
    *--p_stk = 0x25uL;                                          /* R25                                                  */
    *--p_stk = 0x2500uL;
    *--p_stk = 0x24uL;                                          /* R24                                                  */
    *--p_stk = 0x2400uL;
    *--p_stk = 0x23uL;                                          /* R23                                                  */
    *--p_stk = 0x2300uL;
    *--p_stk = 0x22uL;                                          /* R22                                                  */
    *--p_stk = 0x2200uL;
    *--p_stk = 0x21uL;                                          /* R21                                                  */
    *--p_stk = 0x2100uL;
    *--p_stk = 0x20uL;                                          /* R20                                                  */
    *--p_stk = 0x2000uL;
    *--p_stk = 0x19uL;                                          /* R19                                                  */
    *--p_stk = 0x1900uL;
    *--p_stk = 0x18uL;                                          /* R18                                                  */
    *--p_stk = 0x1800uL;
    *--p_stk = 0x17uL;                                          /* R17                                                  */
    *--p_stk = 0x1700uL;
    *--p_stk = 0x16uL;                                          /* R16                                                  */
    *--p_stk = 0x1600uL;
    *--p_stk = 0x15uL;                                          /* R15                                                  */
    *--p_stk = 0x1500uL;
    *--p_stk = 0x14uL;                                          /* R14                                                  */
    *--p_stk = 0x1400uL;
    *--p_stk = (INT32U)&_SDA_BASE_;                             /* R13                                                  */
    *--p_stk = 0x0000uL;
    *--p_stk = 0x12uL;                                          /* R12                                                  */
    *--p_stk = 0x1200uL;
    *--p_stk = 0x11uL;                                          /* R11                                                  */
    *--p_stk = 0x1100uL;
    *--p_stk = 0x10uL;                                          /* R10                                                  */
    *--p_stk = 0x1000uL;
    *--p_stk = 0x9uL;                                           /* R09                                                  */
    *--p_stk = 0x9000uL;
    *--p_stk = 0x8uL;                                           /* R08                                                  */
    *--p_stk = 0x8000uL;
    *--p_stk = 0x7uL;                                           /* R07                                                  */
    *--p_stk = 0x7000uL;
    *--p_stk = 0x6uL;                                           /* R06                                                  */
    *--p_stk = 0x6000uL;
    *--p_stk = 0x5uL;                                           /* R05                                                  */
    *--p_stk = 0x5000uL;
    *--p_stk = 0x4uL;                                           /* R04                                                  */
    *--p_stk = 0x4000uL;
    *--p_stk = (INT32U)p_arg;                                   /* R03                                                  */
    *--p_stk = 0x0000uL;
    *--p_stk = (INT32U)&_SDA2_BASE_;                            /* R02                                                  */
    *--p_stk = 0x0000uL;
#else
    *--p_stk = 0x31uL;                                          /* R31                                                  */
    *--p_stk = 0x30uL;                                          /* R30                                                  */
    *--p_stk = 0x29uL;                                          /* R29                                                  */
    *--p_stk = 0x28uL;                                          /* R28                                                  */
    *--p_stk = 0x27uL;                                          /* R27                                                  */
    *--p_stk = 0x26uL;                                          /* R26                                                  */
    *--p_stk = 0x25uL;                                          /* R25                                                  */
    *--p_stk = 0x24uL;                                          /* R24                                                  */
    *--p_stk = 0x23uL;                                          /* R23                                                  */
    *--p_stk = 0x22uL;                                          /* R22                                                  */
    *--p_stk = 0x21uL;                                          /* R21                                                  */
    *--p_stk = 0x20uL;                                          /* R20                                                  */
    *--p_stk = 0x19uL;                                          /* R19                                                  */
    *--p_stk = 0x18uL;                                          /* R18                                                  */
    *--p_stk = 0x17uL;                                          /* R17                                                  */
    *--p_stk = 0x16uL;                                          /* R16                                                  */
    *--p_stk = 0x15uL;                                          /* R15                                                  */
    *--p_stk = 0x14uL;                                          /* R14                                                  */
    *--p_stk = (INT32U)&_SDA_BASE_;                             /* R13                                                  */
    *--p_stk = 0x12uL;                                          /* R12                                                  */
    *--p_stk = 0x11uL;                                          /* R11                                                  */
    *--p_stk = 0x10uL;                                          /* R10                                                  */
    *--p_stk = 0x9uL;                                           /* R09                                                  */
    *--p_stk = 0x8uL;                                           /* R08                                                  */
    *--p_stk = 0x7uL;                                           /* R07                                                  */
    *--p_stk = 0x6uL;                                           /* R06                                                  */
    *--p_stk = 0x5uL;                                           /* R05                                                  */
    *--p_stk = 0x4uL;                                           /* R04                                                  */
    *--p_stk = (INT32U)p_arg;                                   /* R03                                                  */
    *--p_stk = (INT32U)&_SDA2_BASE_;                            /* R02                                                  */
#endif
    *--p_stk = 0u;                                              /* BLANK for 0xA0 size                                  */
    *--p_stk = (INT32U)ptos;                                    /* Stack Ptr                                            */

    return(p_stk);
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
*                                           OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION > 29200u)
void  OSTCBInitHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
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

#if (OS_CPU_HOOKS_EN > 0u) && (OS_TIME_TICK_HOOK_EN > 0u)
void  OSTimeTickHook (void)
{
#if OS_APP_HOOKS_EN > 0u
    App_TimeTickHook();
#endif

#if (OS_VERSION >= 29200u) && (OS_TMR_EN > 0u)
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0u;
        OSTmrSignal();
    }
#endif
}
#endif
