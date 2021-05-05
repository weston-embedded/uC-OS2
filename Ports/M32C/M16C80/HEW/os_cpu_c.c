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
*                                           Renesas M32C Port
*
* Filename  : os_cpu_c.c
* Version   : V2.93.01
*********************************************************************************************************
* For       : Renesas M32C
* Toolchain : IAR's EW for M32C
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>

#if      OS_VIEW_MODULE > 0
#include <os_viewc.h>
#include <os_view.h>
#endif

/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0
static  INT16U  OSTmrCtr;
#endif

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
#if OS_TMR_EN > 0
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
#if OS_VIEW_MODULE > 0
    OSView_TaskCreateHook(ptcb);
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
***********************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              pdata         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            last valid entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                            'ptos' will contain the LOWEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the HIGHEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when your task starts executing.
*
*              2) Task stack frame is as follows.  The stack is 16 bits wide.
*
*                 SP ->     pdata (L)
*                           pdata (H)
*                           task  (L)
*                           task  (H)
*                           R0
*                           R1
*                           R2
*                           R3
*                           A0    (L)
*                           A0    (H)
*                           A1    (L)
*                           A1    (H)
*                           FB	  (L)
*                           FB	  (H)
*                           task  (L)
*                           task  (H)
*                 ptos ->   FLG
***********************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{
    INT16U  *pstk16;


    pstk16     = (INT16U *)ptos;                  /* Load top of stack for this task                           */
												  /* Simulate call to 'task()' with argument                   */
    *pstk16--  = (INT32U)pdata >> 16L;            /* ... Push of argument 'pdata'                              */
    *pstk16--  = (INT32U)pdata & 0x0000FFFFL;
    *pstk16--  = (INT32U)task  >> 16L;            /* ... Push of the task start address                        */
    *pstk16--  = (INT32U)task  & 0x0000FFFFL;

												  /* Simulate ISR entry and push all registers onto ISP        */
    *pstk16--  = (INT16U)0x0040;                  /* ... FLG register: IPL=0, ISP selected, Interrupts enabled */
    *pstk16--  = (INT32U)task  >> 16L;            /* ... Push of the task start address                        */
    *pstk16--  = (INT32U)task  & 0x0000FFFFL;

												  /* Save registers onto stack frame                           */
    *pstk16--  = (INT16U)0xFBFB;				  /* ... FB register                                           */
    *pstk16--  = (INT16U)0xFBFB;
    *pstk16--  = (INT16U)0x3B3B;				  /* ... SB register                                           */
    *pstk16--  = (INT16U)0x3B3B;
    *pstk16--  = (INT16U)0xA1A1;				  /* ... A1 register                                           */
    *pstk16--  = (INT16U)0xA1A1;
    *pstk16--  = (INT16U)0xA0A0;				  /* ... A0 register                                           */
    *pstk16--  = (INT16U)0xA0A0;
    *pstk16--  = (INT16U)0x3333;				  /* ... R3 register                                           */
    *pstk16--  = (INT16U)0x2222;				  /* ... R2 register                                           */
    *pstk16--  = (INT16U)0x1111;				  /* ... R1 register                                           */
    *pstk16    = (INT16U)0x0000;				  /* ... R0 register                                           */

    return ((OS_STK *)pstk16);
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
#if OS_VIEW_MODULE > 0
    OSView_TaskSwHook();
#endif
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
    (void)ptcb;                                                         /* Prevent Compiler warning                                 */
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
#if OS_VIEW_MODULE > 0
    OSView_TickHook();
#endif

#if OS_TMR_EN > 0
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif
}
#endif
