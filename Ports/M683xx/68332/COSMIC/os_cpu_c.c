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
*                                          683xx Specific code
*                                             COSMIC C V4.1
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#include <ucos_ii.h>

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
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
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
*              pdata         is a pointer to a user supplied data area that will be passed to the task
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
* Note(s)    : 1) The initial value of the Status Register (SR) is OS_INITIAL_SR sets the 683xx processor
*                 to run in SUPERVISOR mode.  It is assumed that all uC/OS-II tasks run in supervisor
*                 mode.
*              2) You can pass the above options in the 'opt' argument.  You MUST only use the upper
*                 8 bits of 'opt' because the lower bits are reserved by uC/OS-II.  If you make changes
*                 to the code below, you will need to ensure that it doesn't affect the behaviour of
*                 OSTaskIdle() and OSTaskStat().
*              3) Registers are initialized to make them easy to differentiate with a debugger.
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{
    INT32U  *pstk32;
    INT16U  *pstk16;


    opt       = opt;                                  /* 'opt' is not used, prevent warning            */
                                                      /* Load stack pointer and align on 32-bit bound  */
    pstk32    = (INT32U *)((INT32U)ptos & 0xFFFFFFFCL);
                                                      /* -- SIMULATE CALL TO FUNCTION WITH ARGUMENT -- */
    *--pstk32 = (INT32U)pdata;                        /*    pdata                                      */
    *--pstk32 = (INT32U)task;                         /*    Task return address                        */
    pstk16    = (INT16U *)pstk32;                     /* Switch to 16-bit wide stack                   */
                                                      /* ------ SIMULATE INTERRUPT STACK FRAME ------- */
    *--pstk16 = (INT16U)(0x0080 + 4 * OS_TRAP_NBR);   /*    Format/Vector                              */
    pstk32    = (INT32U *)pstk16;                     /* Switch to 32-bit wide stack                   */
    *--pstk32 = (INT32U)task;                         /*    Task return address                        */
    pstk16    = (INT16U *)pstk32;                     /* Switch to 16-bit wide stack                   */
    *--pstk16 = (INT16U)OS_INITIAL_SR;                /*    Initial Status Register value for the task */
    pstk32    = (INT32U *)pstk16;                     /* Switch to 32-bit wide stack                   */
                                                      /* ------- SAVE ALL PROCESSOR REGISTERS -------- */
    *--pstk32 = (INT32U)0x00A600A6L;                  /* Register A6                                   */
    *--pstk32 = (INT32U)0x00A500A5L;                  /* Register A5                                   */
    *--pstk32 = (INT32U)0x00A400A4L;                  /* Register A4                                   */
    *--pstk32 = (INT32U)0x00A300A3L;                  /* Register A3                                   */
    *--pstk32 = (INT32U)0x00A200A2L;                  /* Register A2                                   */
    *--pstk32 = (INT32U)0x00A100A1L;                  /* Register A1                                   */
    *--pstk32 = (INT32U)0x00A000A0L;                  /* Register A0                                   */
    *--pstk32 = (INT32U)0x00D700D7L;                  /* Register D7                                   */
    *--pstk32 = (INT32U)0x00D600D6L;                  /* Register D6                                   */
    *--pstk32 = (INT32U)0x00D500D5L;                  /* Register D5                                   */
    *--pstk32 = (INT32U)0x00D400D4L;                  /* Register D4                                   */
    *--pstk32 = (INT32U)0x00D300D3L;                  /* Register D3                                   */
    *--pstk32 = (INT32U)0x00D200D2L;                  /* Register D2                                   */
    *--pstk32 = (INT32U)0x00D100D1L;                  /* Register D1                                   */
    *--pstk32 = (INT32U)0x00D000D0L;                  /* Register D0                                   */
    return ((OS_STK *)pstk32);                        /* Return pointer to new top-of-stack            */
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
#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)
{
}
#endif
