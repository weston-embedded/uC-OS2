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
*                                         Synopsys ARC EM6 Port
*
* Filename  : os_cpu_c.c
* Version   : V2.93.01
*********************************************************************************************************
* For       : Synopsys ARC EM6
* Mode      : Little-Endian, 32 registers, FPU, Code Density, Loop Counter, Stack Check
* Toolchain : MetaWare C/C++ Clang-based Compiler
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_cpu_c__c = "$Id: $";
#endif


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          AUXILIARY REGISTERS
*********************************************************************************************************
*/
                                                                /* Interrupt Controller Registers.                      */
#define  OS_CPU_AR_AUX_IRQ_CTRL                   (0x00Eu)

                                                                /* Interrupt Controller Bits.                           */
#define  OS_CPU_AR_AUX_IRQ_CTRL_L                (0x0400u)
#define  OS_CPU_AR_AUX_IRQ_CTRL_LP               (0x2000u)
#define  OS_CPU_AR_AUX_IRQ_CTRL_NR                   (14u)


/*
*********************************************************************************************************
*                                       AUXILIARY REGISTER RD/WR
*********************************************************************************************************
*/

#define  OS_CPU_AR_RD(addr)                          _lr((addr))
#define  OS_CPU_AR_WR(addr, val)                     _sr((val), (addr))


/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
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
*
*              2) This hook sets up automatic context save on interrupts.
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
void  OSInitHookBegin (void)
{
                                                                /* Set automatic context save on interrupt for:         */
    OS_CPU_AR_WR(OS_CPU_AR_AUX_IRQ_CTRL, OS_CPU_AR_AUX_IRQ_CTRL_L  | /*   loop registers.                               */
                                         OS_CPU_AR_AUX_IRQ_CTRL_LP | /*   code-density registers.                       */
                                         OS_CPU_AR_AUX_IRQ_CTRL_NR); /*   r27..r0 registers.                            */
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
*                                       INITIALIZE A TASK'S STACK
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
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when task starts executing.
*
*              2) Registers are stacked in the following order, from high to low:
*
*                   STATUS32  (0x0A)
*                   PC        (r63)
*                   JLI_BASE  (0x290)
*                   LDI_BASE  (0x291)
*                   EI_BASE   (0x292)
*                   LP_COUNT  (r60)
*                   LP_START  (0x02)
*                   LP_END    (0x03)
*                   r27
*                   r26
*                   r25
*                   .
*                   .
*                   .
*                   r0
*                   r31
*                   r30
*                   r29
*                   ACCH      (r59)
*                   ACCL      (r58)
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)
{
    OS_STK  *p_stk;
    INT32U   i;


    (void)opt;                                                  /* 'opt' is not used, prevent warning                   */

    p_stk = ptos + 1u;                                          /* Load stack pointer                                   */
                                                                /* Align the stack to 4 bytes.                          */
    p_stk = (OS_STK *)((OS_STK)(p_stk) & 0xFFFFFFFCu);

                                                                /* Registers stacked as in saved by regular interrupt.  */
    *(--p_stk) = (OS_STK)0x80000000;                            /*   STATUS32  (0x0A)                                   */
    *(--p_stk) = (OS_STK)task;                                  /*   PC        (r63)                                    */
    *(--p_stk) = (OS_STK)0x0;                                   /*   JLI_BASE  (0x290)                                  */
    *(--p_stk) = (OS_STK)0x0;                                   /*   LDI_BASE  (0x291)                                  */
    *(--p_stk) = (OS_STK)0x0;                                   /*   EI_BASE   (0x292)                                  */
    *(--p_stk) = (OS_STK)0x0;                                   /*   LP_COUNT  (r60)                                    */
    *(--p_stk) = (OS_STK)0x0;                                   /*   LP_START  (0x02)                                   */
    *(--p_stk) = (OS_STK)0x0;                                   /*   LP_END    (0x03)                                   */

    *(--p_stk) = (OS_STK)0x0;                                   /*   FP (r27)                                           */
    *(--p_stk) = (OS_STK)_core_read(26);                        /*   GP (r26)                                           */


    for (i = 25; i >= 1; --i) {                                 /*   r25..r1                                            */
       *(--p_stk) = (OS_STK)(0x01010101*i);
    }

    *(--p_stk) = (OS_STK)p_arg;                                 /*   r0                                                 */

    *(--p_stk) = (OS_STK)OS_TaskReturn;                         /*   BLINK     (r31)                                    */
    *(--p_stk) = (OS_STK)0x1E1E1E1E;                            /*   r30                                                */
    *(--p_stk) = (OS_STK)0x1D1D1D1D;                            /*   r29                                                */
    *(--p_stk) = (OS_STK)0x59595959;                            /*   ACCH      (r59)                                    */
    *(--p_stk) = (OS_STK)0x58585858;                            /*   ACCL      (r58)                                    */

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
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif
}
#endif
