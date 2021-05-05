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
*                                        ATmega128 Specific code
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS
#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
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
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookBegin (void)
{
#if OS_VERSION >= 281 && OS_TMR_EN > 0
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
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
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
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
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
**********************************************************************************************************
*                                       INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created. This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack. It is assumed that 'ptos' points to the
*                            highest valid address on the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : Interrupts are enabled when your task starts executing. You can change this by setting the
*              SREG to 0x00 instead. In this case, interrupts would be disabled upon task startup. The
*              application code would be responsible for enabling interrupts at the beginning of the task
*              code. You will need to modify OSTaskIdle() and OSTaskStat() so that they enable interrupts.
*              Failure to do this will make your system crash!
*
*              The AVR return stack is placed OS_TASK_STK_SIZE_HARD bytes before the bottom of the task's
*              stack.
**********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    INT8U  *psoft_stk;
    INT8U  *phard_stk;                      /* Temp. variable used for setting up AVR hardware stack    */
    INT16U  tmp;


    (void)opt;                              /* 'opt' is not used, prevent warning                       */
    psoft_stk     = (INT8U *)ptos;
    phard_stk     = (INT8U *)ptos
                  - OSTaskStkSize           /* Task stack size                                          */
                  + OSTaskStkSizeHard;      /* AVR return stack ("hardware stack")                      */
    tmp           = (INT16U)task;
                                            /* Put task start address on top of "hardware stack"        */
    *phard_stk--  = (INT8U)(tmp & 0xFF);    /* Save PC return address                                   */
    tmp         >>= 8;
    *phard_stk--  = (INT8U)(tmp & 0xFF);

    *psoft_stk--  = (INT8U)0x00;            /* R0    = 0x00                                             */
    *psoft_stk--  = (INT8U)0x01;            /* R1    = 0x01                                             */
    *psoft_stk--  = (INT8U)0x02;            /* R2    = 0x02                                             */
    *psoft_stk--  = (INT8U)0x03;            /* R3    = 0x03                                             */
    *psoft_stk--  = (INT8U)0x04;            /* R4    = 0x04                                             */
    *psoft_stk--  = (INT8U)0x05;            /* R5    = 0x05                                             */
    *psoft_stk--  = (INT8U)0x06;            /* R6    = 0x06                                             */
    *psoft_stk--  = (INT8U)0x07;            /* R7    = 0x07                                             */
    *psoft_stk--  = (INT8U)0x08;            /* R8    = 0x08                                             */
    *psoft_stk--  = (INT8U)0x09;            /* R9    = 0x09                                             */
    *psoft_stk--  = (INT8U)0x10;            /* R10   = 0x10                                             */
    *psoft_stk--  = (INT8U)0x11;            /* R11   = 0x11                                             */
    *psoft_stk--  = (INT8U)0x12;            /* R12   = 0x12                                             */
    *psoft_stk--  = (INT8U)0x13;            /* R13   = 0x13                                             */
    *psoft_stk--  = (INT8U)0x14;            /* R14   = 0x14                                             */
    *psoft_stk--  = (INT8U)0x15;            /* R15   = 0x15                                             */
    tmp           = (INT16U)p_arg;
    *psoft_stk--  = (INT8U)tmp;             /* 'p_arg' passed in R17:R16                                */
    *psoft_stk--  = (INT8U)(tmp >> 8);
    *psoft_stk--  = (INT8U)0x18;            /* R18   = 0x18                                             */
    *psoft_stk--  = (INT8U)0x19;            /* R19   = 0x19                                             */
    *psoft_stk--  = (INT8U)0x20;            /* R20   = 0x20                                             */
    *psoft_stk--  = (INT8U)0x21;            /* R21   = 0x21                                             */
    *psoft_stk--  = (INT8U)0x22;            /* R22   = 0x22                                             */
    *psoft_stk--  = (INT8U)0x23;            /* R23   = 0x23                                             */
    *psoft_stk--  = (INT8U)0x24;            /* R24   = 0x24                                             */
    *psoft_stk--  = (INT8U)0x25;            /* R25   = 0x25                                             */
    *psoft_stk--  = (INT8U)0x26;            /* R26   = 0x26                                             */
    *psoft_stk--  = (INT8U)0x27;            /* R27   = 0x27                                             */
                                            /* R28     R29:R28 is the software stack which gets ...     */
                                            /* R29             ... in the TCB.                          */
    *psoft_stk--  = (INT8U)0x30;            /* R30   = 0x30                                             */
    *psoft_stk--  = (INT8U)0x31;            /* R31   = 0x31                                             */
    *psoft_stk--  = (INT8U)0xAA;            /* RAMPZ = 0xAA                                             */
    *psoft_stk--  = (INT8U)0x80;            /* SREG  = Interrupts enabled                               */
    tmp           = (INT16U)phard_stk;
    *psoft_stk--  = (INT8U)(tmp >> 8);      /* SPH                                                      */
    *psoft_stk    = (INT8U)(tmp & 0xFF);    /* SPL                                                      */
    return ((OS_STK *)psoft_stk);
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
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
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

#if OS_VERSION >= 281 && OS_TMR_EN > 0
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif
}
#endif
