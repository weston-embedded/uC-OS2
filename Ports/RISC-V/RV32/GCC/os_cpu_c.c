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
*                                              RISC-V Port
*
* Filename  : os_cpu_c.c
* Version   : V2.93.01
*********************************************************************************************************
* For       : RISC-V RV32
* Toolchain : GNU C Compiler
*********************************************************************************************************
* Note(s)   : Hardware FP is not supported.
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <ucos_ii.h>

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
void  OSTaskCreateHook (OS_TCB  *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskCreateHook(p_tcb);
#else
    (void)ptcb;                                /* Prevent compiler warning                             */
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
void  OSTaskDelHook (OS_TCB  *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskDelHook(p_tcb);
#else
    (void)ptcb;                                /* Prevent compiler warning                             */
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
void  OSTaskReturnHook (OS_TCB  *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TaskReturnHook(p_tcb);
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
*              p_tos         is a pointer to the top of stack.  It is assumed that 'ptos' points to
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
* Note(s)    : (1) Interrupts are enabled when task starts executing.
*
*              (2) There is no need to save register x0 since it is a hard-wired zero.
*
*              (3) RISC-V calling convention register usage:
*
*                    +-------------+-------------+----------------------------------+
*                    |  Register   |   ABI Name  | Description                      |
*                    +-------------+-------------+----------------------------------+
*                    |  x31 - x28  |   t6 - t3   | Temporaries                      |
*                    +-------------+-------------+----------------------------------+
*                    |  x27 - x18  |  s11 - s2   | Saved registers                  |
*                    +-------------+-------------+----------------------------------+
*                    |  x17 - x12  |   a7 - a2   | Function arguments               |
*                    +-------------+-------------+----------------------------------+
*                    |  x11 - x10  |   a1 - a0   | Function arguments/return values |
*                    +-------------+-------------+----------------------------------+
*                    |     x9      |     s1      | Saved register                   |
*                    +-------------+-------------+----------------------------------+
*                    |     x8      |    s0/fp    | Saved register/frame pointer     |
*                    +-------------+-------------+----------------------------------+
*                    |   x7 - x5   |   t2 - t0   | Temporaries                      |
*                    +-------------+-------------+----------------------------------+
*                    |     x4      |     tp      | Thread pointer                   |
*                    +-------------+-------------+----------------------------------+
*                    |     x3      |     gp      | Global pointer                   |
*                    +-------------+-------------+----------------------------------+
*                    |     x2      |     sp      | Stack pointer                    |
*                    +-------------+-------------+----------------------------------+
*                    |     x1      |     ra      | return address                   |
*                    +-------------+-------------+----------------------------------+
*                    |     x0      |    zero     | Hard-wired zero                  |
*                    +-------------+-------------+----------------------------------+
*
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void   (*task)(void *p_arg),
                        void    *p_arg,
                        OS_STK  *p_tos,
                        INT16U  opt)
{
    OS_STK *p_stk;


    (void) opt;                                /* 'opt' is not used, prevent warning                   */

    p_stk = p_tos + 1u;                        /* Load stack pointer and align it to 16-bytes          */
    p_stk = (OS_STK *)((OS_STK)(p_stk) & 0xFFFFFFF0u);

    *(--p_stk) = (OS_STK) task;                /* Entry Point                                          */

    *(--p_stk) = (OS_STK) 0x31313131uL;        /* t6                                                   */
    *(--p_stk) = (OS_STK) 0x30303030uL;        /* t5                                                   */
    *(--p_stk) = (OS_STK) 0x29292929uL;        /* t4                                                   */
    *(--p_stk) = (OS_STK) 0x28282828uL;        /* t3                                                   */
                                               /* Saved Registers                                      */
    *(--p_stk) = (OS_STK) 0x27272727uL;        /* s11                                                  */
    *(--p_stk) = (OS_STK) 0x26262626uL;        /* s10                                                  */
    *(--p_stk) = (OS_STK) 0x25252525uL;        /* s9                                                   */
    *(--p_stk) = (OS_STK) 0x24242424uL;        /* s8                                                   */
    *(--p_stk) = (OS_STK) 0x23232323uL;        /* s7                                                   */
    *(--p_stk) = (OS_STK) 0x22222222uL;        /* s6                                                   */
    *(--p_stk) = (OS_STK) 0x21212121uL;        /* s5                                                   */
    *(--p_stk) = (OS_STK) 0x20202020uL;        /* s4                                                   */
    *(--p_stk) = (OS_STK) 0x19191919uL;        /* s3                                                   */
    *(--p_stk) = (OS_STK) 0x18181818uL;        /* s2                                                   */
                                               /* Function Arguments                                   */
    *(--p_stk) = (OS_STK) 0x17171717uL;        /* a7                                                   */
    *(--p_stk) = (OS_STK) 0x16161616uL;        /* a6                                                   */
    *(--p_stk) = (OS_STK) 0x15151515uL;        /* a5                                                   */
    *(--p_stk) = (OS_STK) 0x14141414uL;        /* a4                                                   */
    *(--p_stk) = (OS_STK) 0x13131313uL;        /* a3                                                   */
    *(--p_stk) = (OS_STK) 0x12121212uL;        /* a2                                                   */
                                               /* Function Arguments/return values                     */
    *(--p_stk) = (OS_STK) 0x11111111uL;        /* a1                                                   */
    *(--p_stk) = (OS_STK) p_arg;               /* a0                                                   */
    *(--p_stk) = (OS_STK) 0x09090909uL;        /* s1   : Saved register                                */
    *(--p_stk) = (OS_STK) 0x08080808uL;        /* s0/fp: Saved register/Frame pointer                  */
                                               /* Temporary registers                                  */
    *(--p_stk) = (OS_STK) 0x07070707uL;        /* t2                                                   */
    *(--p_stk) = (OS_STK) 0x06060606uL;        /* t1                                                   */
    *(--p_stk) = (OS_STK) 0x05050505uL;        /* t0                                                   */

    *(--p_stk) = (OS_STK) 0x04040404uL;        /* tp: Thread pointer                                   */
    *(--p_stk) = (OS_STK) 0x03030303uL;        /* gp: Global pointer                                   */
    *(--p_stk) = (OS_STK) 0x02020202uL;        /* sp: Stack  pointer                                   */
    *(--p_stk) = (OS_STK) OS_TaskReturn;       /* ra: return address                                   */

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
void  OSTCBInitHook (OS_TCB  *p_tcb)
{
#if OS_APP_HOOKS_EN > 0u
    App_TCBInitHook(p_tcb);
#else
    (void)ptcb;                                /* Prevent compiler warning                             */
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
*                                          SYS TICK HANDLER
*
* Description: Handle the system tick (SysTick) interrupt, which is used to generate the uC/OS-II tick
*              interrupt.
*
* Arguments  : None.
*
* Note(s)    : This function is defined with weak linking in 'riscv_hal_stubs.c' so that it can be
*              overridden by the kernel port with same prototype
*********************************************************************************************************
*/

void  SysTick_Handler (void)
{
#if OS_CRITICAL_METHOD == 3u                   /* Allocate storage for CPU status register             */
    OS_CPU_SR cpu_sr;
#endif

    OS_ENTER_CRITICAL();
    OSIntEnter();                              /* Tell uC/OS-II that we are starting an ISR            */
    OS_EXIT_CRITICAL();

    OSTimeTick();                              /* Call uC/OS-II's OSTimeTick()                         */

    OSIntExit();                               /* Tell uC/OS-II that we are leaving the ISR            */
}
