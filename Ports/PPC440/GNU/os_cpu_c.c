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
*                                              PowerPC440
*                                          GNU  C/C++ Compiler
*
* Filename  : os_cpu_c.c
* Version   : V2.93.01
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>


extern   void  *_SDA_BASE_;
extern   void  *_SDA2_BASE_;


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

#if OS_APP_HOOKS_EN > 0
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;
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
* Note(s)    : 1) Interrupts are enabled when your task starts executing.
*
*                 OSTCBHighRdy->OSTCBStkPtr + 0x00     RMSR   (IE=1)       (LOW Memory)
*                                           + 0x04     R2
*                                           + 0x08     R3
*                                           + 0x0C     R4
*                                           + 0x10     R5     (p_arg passed to task)
*                                           + 0x14     R6
*                                           + 0x18     R7
*                                           + 0x1C     R8
*                                           + 0x20     R9
*                                           + 0x24     R10
*                                           + 0x28     R11
*                                           + 0x2C     R12
*                                           + 0x30     R13
*                                           + 0x34     R14
*                                           + 0x38     R15
*                                           + 0x3C     R17
*                                           + 0x40     R18
*                                           + 0x44     R19
*                                           + 0x48     R20
*                                           + 0x4C     R21
*                                           + 0x50     R22
*                                           + 0x54     R23
*                                           + 0x58     R24
*                                           + 0x5C     R25
*                                           + 0x60     R26
*                                           + 0x64     R27
*                                           + 0x68     R28
*                                           + 0x6C     R29
*                                           + 0x70     R30
*                                           + 0x74     R31                (HIGH Memory)
*                                           + 0x78     Empty
*                 ptos --------->           + 0x7C     Empty
*
*              2) R16 is not saved as part of the task context since it is used by the debugger.
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    INT32U  *pstk;
    INT32U   msr;


                                      /* Obtain the current value of the MSR                           */
    __asm__ __volatile__("mfmsr %0\n" : "=r" (msr));
                                      /* Interrupts will be enabled when the task is started           */
    msr    |= 0x00028000;

    opt     = opt;                    /* 'opt' is not used, prevent warning                            */
    pstk    = (INT32U *)ptos;         /* Load stack pointer                                            */
    pstk--;                           /* Make sure we point to free entry ...                          */
    pstk--;                           /* ... compiler uses top-of-stack so free an extra one.          */
    *pstk-- = (INT32U)0x31313131;     /* r31                                                           */
    *pstk-- = (INT32U)0x30303030;     /* r30                                                           */
    *pstk-- = (INT32U)0x29292929;     /* r29                                                           */
    *pstk-- = (INT32U)0x28282828;     /* r28                                                           */
    *pstk-- = (INT32U)0x27272727;     /* r27                                                           */
    *pstk-- = (INT32U)0x26262626;     /* r26                                                           */
    *pstk-- = (INT32U)0x25252525;     /* r25                                                           */
    *pstk-- = (INT32U)0x24242424;     /* r24                                                           */
    *pstk-- = (INT32U)0x23232323;     /* r23                                                           */
    *pstk-- = (INT32U)0x22222222;     /* r22                                                           */
    *pstk-- = (INT32U)0x21212121;     /* r21                                                           */
    *pstk-- = (INT32U)0x20202020;     /* r20                                                           */
    *pstk-- = (INT32U)0x19191919;     /* r19                                                           */
    *pstk-- = (INT32U)0x18181818;     /* r18                                                           */
    *pstk-- = (INT32U)0x17171717;     /* r17                                                           */
    *pstk-- = (INT32U)0x16161616;     /* r16                                                           */
    *pstk-- = (INT32U)0x15151515;     /* r15                                                           */
    *pstk-- = (INT32U)0x14141414;     /* r14                                                           */
    *pstk-- = (INT32U)_SDA_BASE_;     /* r13                                                           */
    *pstk-- = (INT32U)0x12121212;     /* r12                                                           */
    *pstk-- = (INT32U)0x11111111;     /* r11                                                           */
    *pstk-- = (INT32U)0x10101010;     /* r10                                                           */
    *pstk-- = (INT32U)0x09090909;     /* r09                                                           */
    *pstk-- = (INT32U)0x08080808;     /* r08                                                           */
    *pstk-- = (INT32U)0x07070707;     /* r07                                                           */
    *pstk-- = (INT32U)0x06060606;     /* r06                                                           */
    *pstk-- = (INT32U)0x05050505;     /* r05                                                           */
    *pstk-- = (INT32U)0x04040404;     /* r04                                                           */
    *pstk-- = (INT32U)p_arg;          /* r03                                                           */
    *pstk-- = (INT32U)_SDA2_BASE_;    /* r02                                                           */
    *pstk-- = (INT32U)0x00000000;     /* r00                                                           */
    *pstk-- = (INT32U)0x0F0F0F0F;     /* User SPR General-Purpose Register 0                           */
    *pstk-- = (INT32U)0x0F0F0F0F;     /* Condition Register                                            */
    *pstk-- = (INT32U)0x0F0F0F0F;     /* Fixed-Point Exception Register                                */
    *pstk-- = (INT32U)0x0F0F0F0F;     /* Count Register                                                */
    *pstk-- = (INT32U)0x0F0F0F0F;     /* Link Register                                                 */
    *pstk-- = (INT32U)task;           /* Return address                                                */
    *pstk-- = msr;                    /* Machine-State Register; interrupts are enabled                */
    pstk--;

    return ((OS_STK *)pstk);          /* Return new top of stack                                       */
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
#if OS_APP_HOOKS_EN > 0
    App_TimeTickHook();
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
