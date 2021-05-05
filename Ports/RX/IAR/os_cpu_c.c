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
*                                      Renesas RX Specific Code
*
* Filename  : os_cpu_c.c
* Version   : V2.93.01
*********************************************************************************************************
* For       : Renesas RX
* Toolchain : IAR Embedded Workbench for Renesas RX
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <ucos_ii.h>
#include  <cpu_core.h>


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define  PSW_INIT                                 0x00030000u

                                                                /* ------ SOFTWARE INTERRUPT ACTIVATION REGISTERS ----- */
#define  ICU_BASE_ADDR                            0x00087000u
#define  ICU_REG_IR027_SWINT                     (*(CPU_REG08 *)(ICU_BASE_ADDR + 0x01Bu))
#define  ICU_REG_IER03_SWINT                     (*(CPU_REG08 *)(ICU_BASE_ADDR + 0x203u))
#define  ICU_REG_SWINTR                          (*(CPU_REG08 *)(ICU_BASE_ADDR + 0x2E0u))
#define  ICU_REG_IPR003_SWINT                    (*(CPU_REG08 *)(ICU_BASE_ADDR + 0x303u))

#define  ICU_SWINTR_BIT_SWINT                     DEF_BIT_00
#define  ICU_IER03_BIT_SWINT                      DEF_BIT_03


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

#if OS_CPU_HOOKS_EN > 0u
void  OSInitHookBegin (void)
{
#if(OS_TMR_EN > 0)
    OSTmrCtr = 0;
#endif

    ICU_REG_IR027_SWINT  = 0u;                                  /* Ensure the software interrupt is clear.              */
    ICU_REG_IPR003_SWINT = 1u;                                  /* Pended context switches must be level 1 IPL          */
    ICU_REG_IER03_SWINT  = ICU_IER03_BIT_SWINT;                 /* Enable the software interrupt.                       */
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
#if OS_APP_HOOKS_EN > 0
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
#if OS_APP_HOOKS_EN > 0
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
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

#if OS_CPU_HOOKS_EN > 0u
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

#if OS_CPU_HOOKS_EN > 0u
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
*                            (see uCOS_II.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when your task starts executing.
*
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void    (*task)(void *p_arg),
                        void     *p_arg,
                        OS_STK   *ptos,
                        INT16U    opt)
{
    OS_STK  *p_stk;


   (void)opt;                                                   /* Prevent compiler warning                             */

    p_stk    = ptos;                                            /* Load stack pointer                                   */
    *--p_stk = (OS_STK)PSW_INIT;                                /* PSW                                                  */
    *--p_stk = (OS_STK)task;                                    /* PC of task                                           */
    *--p_stk = 0x15151515u;                                     /* R15                                                  */
    *--p_stk = 0x14141414u;                                     /* R14                                                  */
    *--p_stk = 0x13131313u;                                     /* R13                                                  */
    *--p_stk = 0x12121212u;                                     /* R12                                                  */
    *--p_stk = 0x11111111u;                                     /* R11                                                  */
    *--p_stk = 0x10101010u;                                     /* R10                                                  */
    *--p_stk = 0x09090909u;                                     /* R9                                                   */
    *--p_stk = 0x08080808u;                                     /* R8                                                   */
    *--p_stk = 0x07070707u;                                     /* R7                                                   */
    *--p_stk = 0x06060606u;                                     /* R6                                                   */
    *--p_stk = 0x05050505u;                                     /* R5                                                   */
    *--p_stk = 0x04040404u;                                     /* R4                                                   */
    *--p_stk = 0x03030303u;                                     /* R3                                                   */
    *--p_stk = 0x02020202u;                                     /* R2                                                   */
    *--p_stk = (OS_STK)p_arg;                                   /* Pass p_arg in R1                                     */

#if (((__VER__>=250) && (__FPU__==1)) || \
((__VER__<250) && (__CORE__!=__RX100__) && (__CORE__!=__RX200__)))
    *--p_stk = 0x00000100u;                                     /* FPSW is NOT available for RX100 and RX200 families   */
#endif

    *--p_stk = 0x00009ABCu;                                     /* ACC (mid, lower word)                                */
    *--p_stk = 0x12345678u;                                     /* ACC (high)                                           */

    return (p_stk);
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
    OS_TRACE_TASK_SWITCHED_IN(OSTCBHighRdy);
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

#if OS_CPU_HOOKS_EN > 0u
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
*                                            Context Switch
*
* Description: This function is called to switch task context (called from task)
*
* Arguments  : None.
*********************************************************************************************************
*/

void  OSCtxSw (void)
{
    ICU_REG_SWINTR = ICU_SWINTR_BIT_SWINT;                      /* Trigger the pended interrupt request                 */
}


/*
*********************************************************************************************************
*                                       Context Switch from ISR
*
* Description: This function is called to switch task context (called from isr)
*
* Arguments  : None.
*********************************************************************************************************
*/

void  OSIntCtxSw (void)
{
    ICU_REG_SWINTR = ICU_SWINTR_BIT_SWINT;                      /* Trigger the pended interrupt request                 */
}
