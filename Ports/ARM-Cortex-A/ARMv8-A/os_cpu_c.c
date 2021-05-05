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
*                                            ARMv8-A Port
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
* For      : ARMv8-A Cortex-A
* Mode     : ARM64
**********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>

#if (OS_CPU_HOOKS_EN == 0u)
#error "ERROR: OS_CPU_HOOKS_EN must be set to 1"
#endif


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/

#define  OS_CPU_STK_ALIGN_BYTES     (16u)


/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*/

#if (OS_TMR_EN > 0u)
static  INT16U  OSTmrCtr;
#endif


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                             (BEGINNING)
*
* Description : This function is called by OSInit() at the beginning of OSInit().
*
* Argument(s) : none
*
* Note(s)     : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/

void  OSInitHookBegin (void)
{
    INT32U   size;
    OS_STK  *pstk;

                                                                /* Clear exception stack for stack checking.            */
    pstk = &OS_CPU_ExceptStk[0];
    size = OS_CPU_EXCEPT_STK_SIZE;
    while (size > 0u) {
        size--;
       *pstk++ = (OS_STK)0;
    }
                                                                /* Align the ISR stack to 16-bytes                      */
    OS_CPU_ExceptStkBase = (OS_STK *)(OS_CPU_ExceptStk + OS_CPU_EXCEPT_STK_SIZE - 1u);
    OS_CPU_ExceptStkBase = (OS_STK *)((CPU_STK)OS_CPU_ExceptStkBase & ~(OS_CPU_STK_ALIGN_BYTES - 1u));


#if (OS_TMR_EN > 0u)
    OSTmrCtr = 0u;
#endif
}


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                                (END)
*
* Description : This function is called by OSInit() at the end of OSInit().
*
* Argument(s) : none
*
* Note(s)     : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/

void  OSInitHookEnd (void)
{
#if (OS_CPU_INT_DIS_MEAS_EN > 0u)
    OS_CPU_IntDisMeasInit();
#endif
}


/*
*********************************************************************************************************
*                                         TASK CREATION HOOK
*
* Description : This function is called when a task is created.
*
* Argument(s) : ptcb    is a pointer to the task control block of the task being created.
*
* Note(s)     : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  OSTaskCreateHook (OS_TCB *ptcb)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
#endif
}


/*
*********************************************************************************************************
*                                         TASK DELETION HOOK
*
* Description : This function is called when a task is deleted.
*
* Argument(s) : ptcb    is a pointer to the task control block of the task being deleted.
*
* Note(s)     : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  OSTaskDelHook (OS_TCB *ptcb)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                                 /* Prevent compiler warning                             */
#endif
}


/*
*********************************************************************************************************
*                                           IDLE TASK HOOK
*
* Description : This function is called by the idle task.  This hook has been added to allow you to do
*               such things as STOP the CPU to conserve power.
*
* Argument(s) : none
*
* Note(s)     : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

void  OSTaskIdleHook (void)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskIdleHook();
#endif
}


/*
*********************************************************************************************************
*                                          TASK RETURN HOOK
*
* Description : This function is called if a task accidentally returns.  In other words, a task should
*               either be an infinite loop or delete itself when done.
*
* Argument(s) : ptcb      is a pointer to the task control block of the task that is returning.
*
* Note(s)     : none
*********************************************************************************************************
*/

void  OSTaskReturnHook (OS_TCB  *ptcb)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskReturnHook(ptcb);
#else
    (void)ptcb;
#endif
}


/*
*********************************************************************************************************
*                                         STATISTIC TASK HOOK
*
* Description : This function is called every second by uC/OS-II's statistics task.  This allows your
*               application to add functionality to the statistics task.
*
* Argument(s) : none
*
* Note(s)     : none
*********************************************************************************************************
*/

void  OSTaskStatHook (void)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskStatHook();
#endif
}


/*
*********************************************************************************************************
*                                      INITIALIZE A TASK'S STACK
*
* Description : This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*               stack frame of the task being created. This function is highly processor specific.
*
* Argument(s) : task        is a pointer to the task code.
*
*               p_arg       is a pointer to a user supplied data area that will be passed to the task
*                           when the task first executes.
*
*               ptos        is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                           a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                           'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                           OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                           of the stack.
*
*               opt         specifies options that can be used to alter the behavior of OSTaskStkInit().
*                           (see uCOS_II.H for OS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : (1) The full stack frame is shown below. If SIMD is disabled, (OS_CPU_SIMD == 0),
*                  the stack frame will only contain the core registers.
*
*                                            [LOW MEMORY]
*                                   ******************************
*                                   -0x320              [  FPSR  ]
*                                   -0x318              [  FPCR  ]
*                                   ******************************
*                                   -0x310              [   V0   ]
*                                   -0x300              [   V1   ]
*                                   -0x2F0              [   V2   ]
*                                   -0x2E0              [   V3   ]
*                                   -0x2D0              [   V4   ]
*                                   -0x2C0              [   V5   ]
*                                   -0x2B0              [   V6   ]
*                                   -0x2A0              [   V7   ]
*                                   -0x290              [   V8   ]
*                                   -0x280              [   V9   ]
*                                   -0x270              [  V10   ]
*                                   -0x260              [  V11   ]
*                                   -0x250              [  V12   ]
*                                   -0x240              [  V13   ]
*                                   -0x230              [  V14   ]
*                                   -0x220              [  V15   ]
*                                   -0x210              [  V16   ]
*                                   -0x200              [  V17   ]
*                                   -0x1F0              [  V18   ]
*                                   -0x1E0              [  V19   ]
*                                   -0x1D0              [  V20   ]
*                                   -0x1C0              [  V21   ]
*                                   -0x1B0              [  V22   ]
*                                   -0x1A0              [  V23   ]
*                                   -0x190              [  V24   ]
*                                   -0x180              [  V25   ]
*                                   -0x170              [  V26   ]
*                                   -0x160              [  V27   ]
*                                   -0x150              [  V28   ]
*                                   -0x140              [  V29   ]
*                                   -0x130              [  V30   ]
*                                   -0x120              [  V31   ]
*                                   ******************************
*                                   -0x110              [PADDING ]
*                                   -0x108              [SPSR_ELx]
*                                   -0x100              [   LR   ]
*                                   -0x0F8              [ELR_ELx ]
*                                   ******************************
*                                   -0x0F0              [   R0   ]
*                                   -0x0E8              [   R1   ]
*                                   -0x0E0              [   R2   ]
*                                   -0x0D8              [   R3   ]
*                                   -0x0D0              [   R4   ]
*                                   -0x0C8              [   R5   ]
*                                   -0x0C0              [   R6   ]
*                                   -0x0B8              [   R7   ]
*                                   -0x0B0              [   R8   ]
*                                   -0x0A8              [   R9   ]
*                                   -0x0A0              [  R10   ]
*                                   -0x098              [  R11   ]
*                                   -0x090              [  R12   ]
*                                   -0x088              [  R13   ]
*                                   -0x080              [  R14   ]
*                                   -0x078              [  R15   ]
*                                   -0x070              [  R16   ]
*                                   -0x068              [  R17   ]
*                                   -0x060              [  R18   ]
*                                   -0x058              [  R19   ]
*                                   -0x050              [  R20   ]
*                                   -0x048              [  R21   ]
*                                   -0x040              [  R22   ]
*                                   -0x038              [  R23   ]
*                                   -0x030              [  R24   ]
*                                   -0x028              [  R25   ]
*                                   -0x020              [  R26   ]
*                                   -0x018              [  R27   ]
*                                   -0x010              [  R28   ]
*                                   -0x008              [  R29   ]
*                                   **********Stack Base**********
*                                   ******SP_BASE MOD 16 = 0******
*                                            [HIGH MEMORY]
**********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void  *p_arg),
                        void               *p_arg,
                        OS_STK             *ptos,
                        INT16U              opt)
{
    OS_STK  *p_stk;
    OS_STK   task_addr;
    INT8U    i;


    (void)opt;

                                                                /* Align stack pointer to 16 bytes                      */
    p_stk =  ptos + 1u;
    p_stk = (OS_STK *)((OS_STK)p_stk & ~(OS_CPU_STK_ALIGN_BYTES - 1ul));

    task_addr = (OS_STK)task;

    for (i = 29; i > 0; i--) {
        *--p_stk = (INT64U)i;                                  /* Reg X1-X29                                           */
    }

    *--p_stk  = (OS_STK)p_arg;                                 /* Reg X0 : argument                                    */

    *--p_stk  = (OS_STK)task_addr;                             /* Entry Point                                          */
    *--p_stk  = (OS_STK)OS_TaskReturn;                         /* Reg X30 (LR)                                         */

    *--p_stk  = (OS_STK)OS_CPU_SPSRGet();
    *--p_stk  = (OS_STK)OS_CPU_SPSRGet();

    if (OS_CPU_SIMDGet() == 1u) {
        for (i = 64; i > 0; i--) {
            *--p_stk = (INT64U)i;                               /* Reg Q0-Q31                                           */
        }

        *--p_stk = 0x0000000000000000;                          /* FPCR                                                 */
        *--p_stk = 0x0000000000000000;                          /* FPSR                                                 */
    }

    return (p_stk);
}


/*
*********************************************************************************************************
*                                          TASK SWITCH HOOK
*
* Description : This function is called when a task switch is performed.  This allows you to perform
*               other operations during a context switch.
*
* Argument(s) : none
*
* Note(s)     : 1) Interrupts are disabled during this call.
*
*               2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task
*                  that will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to
*                  the task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if (OS_TASK_SW_HOOK_EN > 0u)
void  OSTaskSwHook (void)

{
#if (OS_CFG_DBG_EN > 0u)
    INT32U  ctx_id;
#endif

#if (OS_APP_HOOKS_EN > 0u)
    App_TaskSwHook();
#endif

}
#endif


/*
*********************************************************************************************************
*                                          OS_TCBInit() HOOK
*
* Description : This function is called by OS_TCBInit() after setting up most of the TCB.
*
* Argument(s) : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)     : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

void  OSTCBInitHook (OS_TCB *ptcb)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                            /* Prevent compiler warning                 */
#endif
}


/*
*********************************************************************************************************
*                                              TICK HOOK
*
* Description : This function is called every tick.
*
* Argument(s) : none
*
* Note(s)     : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_TIME_TICK_HOOK_EN > 0u)
void  OSTimeTickHook (void)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TimeTickHook();
#endif

#if (OS_TMR_EN > 0u)
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
*                              INTERRUPT DISABLE TIME MEASUREMENT, START
*********************************************************************************************************
*/

#if (OS_CPU_INT_DIS_MEAS_EN > 0u)
void  OS_CPU_IntDisMeasInit (void)
{
    OS_CPU_IntDisMeasNestingCtr = 0u;
    OS_CPU_IntDisMeasCntsEnter  = 0u;
    OS_CPU_IntDisMeasCntsExit   = 0u;
    OS_CPU_IntDisMeasCntsMax    = 0u;
    OS_CPU_IntDisMeasCntsDelta  = 0u;
    OS_CPU_IntDisMeasCntsOvrhd  = 0u;
    OS_CPU_IntDisMeasStart();                                   /* Measure the overhead of the functions                */
    OS_CPU_IntDisMeasStop();
    OS_CPU_IntDisMeasCntsOvrhd  = OS_CPU_IntDisMeasCntsDelta;
}


void  OS_CPU_IntDisMeasStart (void)
{
    OS_CPU_IntDisMeasNestingCtr++;
    if (OS_CPU_IntDisMeasNestingCtr == 1u) {                    /* Only measure at the first nested level               */
        OS_CPU_IntDisMeasCntsEnter = OS_CPU_IntDisMeasTmrRd();
    }
}


void  OS_CPU_IntDisMeasStop (void)
{
    OS_CPU_IntDisMeasNestingCtr--;                                      /* Decrement nesting ctr                                */
    if (OS_CPU_IntDisMeasNestingCtr == 0u) {
        OS_CPU_IntDisMeasCntsExit  = OS_CPU_IntDisMeasTmrRd();
        OS_CPU_IntDisMeasCntsDelta = OS_CPU_IntDisMeasCntsExit - OS_CPU_IntDisMeasCntsEnter;
        if (OS_CPU_IntDisMeasCntsDelta > OS_CPU_IntDisMeasCntsOvrhd) {  /* Ensure overhead < delta                              */
            OS_CPU_IntDisMeasCntsDelta -= OS_CPU_IntDisMeasCntsOvrhd;
        } else {
            OS_CPU_IntDisMeasCntsDelta  = OS_CPU_IntDisMeasCntsOvrhd;
        }
        if (OS_CPU_IntDisMeasCntsDelta > OS_CPU_IntDisMeasCntsMax) {    /* Track MAXIMUM                                        */
            OS_CPU_IntDisMeasCntsMax = OS_CPU_IntDisMeasCntsDelta;
        }
    }
}
#endif
