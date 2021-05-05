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
*                                    Freescale MPC55xx Specific code
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
    OS_STK     *stkp;                                                   /* Local: Stack pointer                                     */
    OS_CPU_SR   msr;                                                    /* Local: Initial MSR                                       */
    OS_CPU_SR   srr1;                                                   /* Local: Initial SRR1                                      */


    (void)opt;                                                          /* 'opt' is not used, prevent warning                       */

    msr     = CPU_SR_Rd();                                              /* get the MSB reg value                                    */
    srr1    = msr | 0x8000;                                             /* set MSR[EE] bit to enable interrupts                     */

    stkp    = (OS_STK *)((INT32U)ptos & 0xFFFFFFF0);                    /* 16-byte align task's stack pointer (EABI)                */

                                                                        /* Leave buffer area for locals "above the stack" in ...    */
    stkp   -=  OS_STK_RSVD_SIZE;                                        /* case the compiler prolog puts variables above the stack  */

                                                                        /* Stack Frame Initialization                               */
    *--stkp = (INT32U)msr;                                              /* MSR                                                      */
    *--stkp = 0;                                                        /* EABI padding                                             */
    *--stkp = 0;                                                        /* EABI padding                                             */
    *--stkp = 0;                                                        /* SPEFSCR                                                  */
    *--stkp = (INT32U)task;                                             /* LR                                                       */
    *--stkp = 0;                                                        /* CR                                                       */
    *--stkp = 0;                                                        /* XER                                                      */
    *--stkp = 0;                                                        /* CTR                                                      */
    *--stkp = 0;                                                        /* USPRG0                                                   */
    *--stkp = (INT32U)srr1;                                             /* SRR1                                                     */
    *--stkp = (INT32U)task;                                             /* SRR0                                                     */
    *--stkp = 0;                                                        /* R0                                                       */


#ifdef OS_SAVE_CONTEXT_WITH_FPRS                                        /* Push 64-bit register's initial value to the stack        */
    *--stkp = 0x31L;                                                    /* r31                                                      */
    *--stkp = 0x3100L;
    *--stkp = 0x30L;                                                    /* r30                                                      */
    *--stkp = 0x3000L;
    *--stkp = 0x29L;                                                    /* r29                                                      */
    *--stkp = 0x2900L;
    *--stkp = 0x28L;                                                    /* r28                                                      */
    *--stkp = 0x2800L;
    *--stkp = 0x27L;                                                    /* r27                                                      */
    *--stkp = 0x2700L;
    *--stkp = 0x26L;                                                    /* r26                                                      */
    *--stkp = 0x2600L;
    *--stkp = 0x25L;                                                    /* r25                                                      */
    *--stkp = 0x2500L;
    *--stkp = 0x24L;                                                    /* r24                                                      */
    *--stkp = 0x2400L;
    *--stkp = 0x23L;                                                    /* r23                                                      */
    *--stkp = 0x2300L;
    *--stkp = 0x22L;                                                    /* r22                                                      */
    *--stkp = 0x2200L;
    *--stkp = 0x21L;                                                    /* r21                                                      */
    *--stkp = 0x2100L;
    *--stkp = 0x20L;                                                    /* r20                                                      */
    *--stkp = 0x2000L;
    *--stkp = 0x19L;                                                    /* r19                                                      */
    *--stkp = 0x1900L;
    *--stkp = 0x18L;                                                    /* r18                                                      */
    *--stkp = 0x1800L;
    *--stkp = 0x17L;                                                    /* r17                                                      */
    *--stkp = 0x1700L;
    *--stkp = 0x16L;                                                    /* r16                                                      */
    *--stkp = 0x1600L;
    *--stkp = 0x15L;                                                    /* r15                                                      */
    *--stkp = 0x1500L;
    *--stkp = 0x14L;                                                    /* r14                                                      */
    *--stkp = 0x1400L;
    *--stkp = (INT32U)&_SDA_BASE_;                                      /* r13                                                      */
    *--stkp = 0x0000L;
    *--stkp = 0x12L;                                                    /* r12                                                      */
    *--stkp = 0x1200L;
    *--stkp = 0x11L;                                                    /* r11                                                      */
    *--stkp = 0x1100L;
    *--stkp = 0x10L;                                                    /* r10                                                      */
    *--stkp = 0x1000L;
    *--stkp = 0x9L;                                                     /* r09                                                      */
    *--stkp = 0x9000L;
    *--stkp = 0x8L;                                                     /* r08                                                      */
    *--stkp = 0x8000L;
    *--stkp = 0x7L;                                                     /* r07                                                      */
    *--stkp = 0x7000L;
    *--stkp = 0x6L;                                                     /* r06                                                      */
    *--stkp = 0x6000L;
    *--stkp = 0x5L;                                                     /* r05                                                      */
    *--stkp = 0x5000L;
    *--stkp = 0x4L;                                                     /* r04                                                      */
    *--stkp = 0x4000L;
    *--stkp = (INT32U)p_arg;                                            /* r03                                                      */
    *--stkp = 0x0000L;
    *--stkp = (INT32U)&_SDA2_BASE_;                                     /* r02                                                      */
    *--stkp = 0x0000L;

#else
    *--stkp = 0x31L;                                                    /* r31                                                      */
    *--stkp = 0x30L;                                                    /* r30                                                      */
    *--stkp = 0x29L;                                                    /* r29                                                      */
    *--stkp = 0x28L;                                                    /* r28                                                      */
    *--stkp = 0x27L;                                                    /* r27                                                      */
    *--stkp = 0x26L;                                                    /* r26                                                      */
    *--stkp = 0x25L;                                                    /* r25                                                      */
    *--stkp = 0x24L;                                                    /* r24                                                      */
    *--stkp = 0x23L;                                                    /* r23                                                      */
    *--stkp = 0x22L;                                                    /* r22                                                      */
    *--stkp = 0x21L;                                                    /* r21                                                      */
    *--stkp = 0x20L;                                                    /* r20                                                      */
    *--stkp = 0x19L;                                                    /* r19                                                      */
    *--stkp = 0x18L;                                                    /* r18                                                      */
    *--stkp = 0x17L;                                                    /* r17                                                      */
    *--stkp = 0x16L;                                                    /* r16                                                      */
    *--stkp = 0x15L;                                                    /* r15                                                      */
    *--stkp = 0x14L;                                                    /* r14                                                      */
    *--stkp = (INT32U)&_SDA_BASE_;                                      /* r13                                                      */
    *--stkp = 0x12L;                                                    /* r12                                                      */
    *--stkp = 0x11L;                                                    /* r11                                                      */
    *--stkp = 0x10L;                                                    /* r10                                                      */
    *--stkp = 0x9L;                                                     /* r09                                                      */
    *--stkp = 0x8L;                                                     /* r08                                                      */
    *--stkp = 0x7L;                                                     /* r07                                                      */
    *--stkp = 0x6L;                                                     /* r06                                                      */
    *--stkp = 0x5L;                                                     /* r05                                                      */
    *--stkp = 0x4L;                                                     /* r04                                                      */
    *--stkp = (INT32U)p_arg;                                            /* r03                                                      */
    *--stkp = (INT32U)&_SDA2_BASE_;                                     /* r02                                                      */
#endif
    *--stkp = 0;                                                        /* BLANK for 0xA0 size                                      */
    *--stkp = (INT32U)ptos;                                             /* Stack Ptr                                                */

    return(stkp);
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
