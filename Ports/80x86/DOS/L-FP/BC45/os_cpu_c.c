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
*                                          80x86 Specific code
*                                LARGE MEMORY MODEL WITH FLOATING-POINT
*                                          Borland C/C++ V4.51
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>

INT8U  const  OSTickDOSCtrReload = (INT8U)((FP32)OS_TICKS_PER_SEC / (FP32)18.20648 + (FP32)0.5);


/*
*********************************************************************************************************
*                                             LOCAL CONSTANTS
*
* Note(s) : 1) OS_NTASKS_FP  establishes the number of tasks capable of supporting floating-point.  One
*              task is removed for the idle task because it doesn't do floating-point at all.
*           2) OS_FP_STORAGE_SIZE  currently allocates 128 bytes of storage even though the 80x86 FPU
*              only require 108 bytes to save the FPU context.  I decided to allocate 128 bytes for
*              future expansion.
*********************************************************************************************************
*/

#define  OS_NTASKS_FP         (OS_MAX_TASKS + OS_N_SYS_TASKS - 1)
#define  OS_FP_STORAGE_SIZE   128

/*
*********************************************************************************************************
*                                             LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_MEM  *OSFPPartPtr;          /* Pointer to memory partition holding FPU storage areas        */

                                       /* I used INT32U to ensure that storage is aligned on a ...     */
                                       /* ... 32-bit boundary.                                         */
static  INT32U   OSFPPart[OS_NTASKS_FP][OS_FP_STORAGE_SIZE / sizeof(INT32U)];


/*
*********************************************************************************************************
*                                        INITIALIZE FP SUPPORT
*
* Description: This function is called to initialize the memory partition needed to support context
*              switching the Floating-Point registers.  This function MUST be called AFTER calling
*              OSInit().
*
* Arguments  : none
*
* Returns    : none
*
* Note(s)    : 1) Tasks that are to use FP support MUST be created with OSTaskCreateExt().
*              2) For the 80x86 FPU, 108 bytes are required to save the FPU context.  I decided to
*                 allocate 128 bytes for future expansion.  Also, I used INT32U to ensure that storage
*                 is aligned on a 32-bit boundary.
*              3) I decided to 'change' the 'Options' attribute for the statistic task in case you
*                 use OSTaskStatHook() and need to perform floating-point operations in this function.
*                 This only applies if OS_TaskStat() was created with OSTaskCreateExt().
*********************************************************************************************************
*/

void  OSFPInit (void)
{
    INT8U    err;
#if OS_TASK_STAT_EN && OS_TASK_CREATE_EXT_EN
    OS_TCB  *ptcb;
    void    *pblk;
#endif


    OSFPPartPtr = OSMemCreate(&OSFPPart[0][0], OS_NTASKS_FP, OS_FP_STORAGE_SIZE, &err);

#if OS_TASK_STAT_EN && OS_TASK_CREATE_EXT_EN       /* CHANGE 'OPTIONS' for OS_TaskStat()               */
    ptcb            = OSTCBPrioTbl[OS_STAT_PRIO];
    ptcb->OSTCBOpt |= OS_TASK_OPT_SAVE_FP;         /* Allow floating-point support for Statistic task  */
    pblk            = OSMemGet(OSFPPartPtr, &err); /* Get storage for FPU registers                    */
    if (pblk != (void *)0) {                       /* Did we get a memory block?                       */
        ptcb->OSTCBExtPtr = pblk;                  /* Yes, Link to task's TCB                          */
        OSFPSave(pblk);                            /*      Save the FPU registers in block             */
    }
#endif
}


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
    OSFPInit();
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
*              2) I decided to change the options on the statistic task to allow for floating-point in
*                 case you decide to do math. in OSTaskStatHook().
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0
void OSTaskCreateHook (OS_TCB *ptcb)
{
    INT8U  err;
    void  *pblk;


    if (ptcb->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {  /* See if task needs FP support                      */
        pblk = OSMemGet(OSFPPartPtr, &err);      /* Yes, Get storage for FPU registers                */
        if (pblk != (void *)0) {                 /*      Did we get a memory block?                   */
            ptcb->OSTCBExtPtr = pblk;            /*      Yes, Link to task's TCB                      */
            OSFPSave(pblk);                      /*           Save the FPU registers in block         */
        }
    }
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
void OSTaskDelHook (OS_TCB *ptcb)
{
    if (ptcb->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {            /* See if task had FP support               */
        if (ptcb->OSTCBExtPtr != (void *)0) {              /* Yes, OSTCBExtPtr must not be NULL        */
            OSMemPut(OSFPPartPtr, ptcb->OSTCBExtPtr);      /*      Return memory block to free pool    */
        }
    }
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
#if OS_CPU_HOOKS_EN && OS_VERSION >= 251
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
void OSTaskStatHook (void)
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
* Note(s)    : Interrupts are enabled when your task starts executing. You can change this by setting the
*              PSW to 0x0002 instead.  In this case, interrupts would be disabled upon task startup.  The
*              application code would be responsible for enabling interrupts at the beginning of the task
*              code.  You will need to modify OSTaskIdle() and OSTaskStat() so that they enable
*              interrupts.  Failure to do this will make your system crash!
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    INT16U *stk;


    opt    = opt;                           /* 'opt' is not used, prevent warning                      */
    stk    = (INT16U *)ptos;                /* Load stack pointer                                      */
    *stk-- = (INT16U)FP_SEG(p_arg);         /* Simulate call to function with argument                 */
    *stk-- = (INT16U)FP_OFF(p_arg);
    *stk-- = (INT16U)FP_SEG(task);
    *stk-- = (INT16U)FP_OFF(task);
    *stk-- = (INT16U)0x0202;                /* SW = Interrupts enabled                                 */
    *stk-- = (INT16U)FP_SEG(task);          /* Put pointer to task   on top of stack                   */
    *stk-- = (INT16U)FP_OFF(task);
    *stk-- = (INT16U)0xAAAA;                /* AX = 0xAAAA                                             */
    *stk-- = (INT16U)0xCCCC;                /* CX = 0xCCCC                                             */
    *stk-- = (INT16U)0xDDDD;                /* DX = 0xDDDD                                             */
    *stk-- = (INT16U)0xBBBB;                /* BX = 0xBBBB                                             */
    *stk-- = (INT16U)0x0000;                /* SP = 0x0000                                             */
    *stk-- = (INT16U)0x1111;                /* BP = 0x1111                                             */
    *stk-- = (INT16U)0x2222;                /* SI = 0x2222                                             */
    *stk-- = (INT16U)0x3333;                /* DI = 0x3333                                             */
    *stk-- = (INT16U)0x4444;                /* ES = 0x4444                                             */
    *stk   = _DS;                           /* DS = Current value of DS                                */
    return ((OS_STK *)stk);
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
void OSTaskSwHook (void)
{
    INT8U  err;
    void  *pblk;
                                                           /* Save FPU context of preempted task       */
    if (OSRunning == TRUE) {                               /* Don't save on OSStart()!                 */
        if (OSTCBCur->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {    /* See if task used FP                      */
            pblk = OSTCBCur->OSTCBExtPtr;                  /* Yes, Get pointer to FP storage area      */
            if (pblk != (void *)0) {                       /*      Make sure we have storage           */
                OSFPSave(pblk);                            /*      Save the FPU registers in block     */
            }
        }
    }
                                                           /* Restore FPU context of new task          */
    if (OSTCBHighRdy->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {    /* See if new task uses FP                  */
        pblk = OSTCBHighRdy->OSTCBExtPtr;                  /* Yes, Get pointer to FP storage area      */
        if (pblk != (void *)0) {                           /*      Make sure we have storage           */
            OSFPRestore(pblk);                             /*      Get contents of FPU registers       */
        }
    }
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
void OSTimeTickHook (void)
{
}
#endif
