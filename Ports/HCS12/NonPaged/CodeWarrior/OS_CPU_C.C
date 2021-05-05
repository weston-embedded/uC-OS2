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
*                                         MC9S12 Specific code
*                                             (Metrowerks)
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                         PROTOTYPES
*********************************************************************************************************
*/

void  OSTaskAbort(void);

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
  ptcb=ptcb;
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
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
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
* Note(s)    : 1) XIRQ interrupts are disabled when your task starts executing. You can change this by
*                 clearing BIT6 in the CCR.
*              2) The STOP instruction is disabled when your task starts executing.  You can change this
*                 by clearing BIT7 in the CCR.
*              3) The other interrupts (i.e. maskable interrupts) are enabled when your task starts
*                 executing.  You can change this by setting BIT4 in the CCR.
*              4) You can change pass the above options in the 'opt' argument.  You MUST only use the
*                 upper 8 bits of 'opt' because the lower bits are reserved by uC/OS-II.  If you make
*                 changes to the code below, you will need to ensure that it doesn't affect the behaviour
*                 of OSTaskIdle() and OSTaskStat().
*              5) Registers are initialized to make them easy to differentiate with a debugger.
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    INT8U  *bstk;


    (void)&opt;                                     /* 'opt' is not used, prevent warning                     */

    bstk    = (INT8U *)ptos;                        /* Load stack pointer                                     */


    *--bstk = (INT8U)(((INT32U)OSTaskAbort) >>  8); /* Task return address low  (simulated calling function)  */
    *--bstk = (INT8U)(((INT32U)OSTaskAbort) >> 16); /* Task return address high (simulated calling function)  */

    *--bstk = (INT8U)(((INT32U)task) >> 8);         /* Task start address PCL                                 */
    *--bstk = (INT8U)(((INT32U)task) >> 16);        /* Task start address PCH.                                */

    *--bstk = (INT8U)0x22;                          /* Y Register low                                         */
    *--bstk = (INT8U)0x22;                          /* Y Register high                                        */

    *--bstk = (INT8U)0x11;                          /* X Register low                                         */
    *--bstk = (INT8U)0x11;                          /* X Register high                                        */

    *--bstk = (INT8U)(((INT16U)p_arg) >> 8);        /* A register / D register high task argument             */
    *--bstk = (INT8U) ((INT16U)p_arg);              /* B register / D register low  task argument             */

    *--bstk = (0xC0);                               /* CCR Register (Disable STOP instruction and XIRQ)       */

    return ((OS_STK *)bstk);                        /* Return pointer to new top-of-stack                     */
}


/*
*********************************************************************************************************
*                                           TASK ABORT HANDLER
*
* Description: This function serves as the caller function for all tasks.  If a task accidently returns,
*              the caller will regain control of the CPU and delete the task preventing a system crash..
*
* Arguments  : none
*
* Note(s)    : In uC/OS-II, a task should NEVER exit and should instead call OSTaskDel(OS_PRIO_SELF)
*              when no longer desired.
*********************************************************************************************************
*/

void  OSTaskAbort (void)
{
    OSTaskDel(OS_PRIO_SELF);
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
#if OS_CPU_HOOKS_EN > 0
void  OSTaskSwHook (void)
{
}
#endif

/*
*********************************************************************************************************
*                                           OSTCBInit() HOOK
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
    ptcb = ptcb;                                           /* Prevent Compiler warning                 */
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
#if OS_CPU_HOOKS_EN > 0
void  OSTimeTickHook (void)
{
}
#endif
