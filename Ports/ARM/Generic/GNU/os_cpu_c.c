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
*                                           Generic ARM Port
*
* Filename  : os_cpu_c.c
* Version   : V2.93.01
*********************************************************************************************************
* For       : ARM7 or ARM9
* Mode      : ARM  or Thumb
* Toolchain : GNU GCC
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>


/*
*********************************************************************************************************
*                                             LOCAL CONSTANTS
*
* Note(s) : 1) ARM_MODE_ARM is the CPSR bit mask for ARM Mode
*           2) ARM_MODE_THUMB is the CPSR bit mask for THUMB Mode
*           3) ARM_SVC_MODE_THUMB is the CPSR bit mask for SVC MODE + THUMB Mode
*           4) ARM_SVC_MODE_ARM is the CPSR bit mask for SVC MODE + ARM Mode
            5) OS_NTASKS_FP  establishes the number of tasks capable of supporting floating-point.  One
*              task is removed for the idle task because it doesn't do floating-point at all.
*           6) OS_FP_STORAGE_SIZE  currently allocates 128 bytes of storage in order to accomodate
*              thirty-two single-precision 32-bit, or sixteen double-precision 64-bit VFP registers.
*********************************************************************************************************
*/

#define  ARM_MODE_ARM           0x00000000u
#define  ARM_MODE_THUMB         0x00000020u

#define  ARM_SVC_MODE_THUMB    (0x00000013uL + ARM_MODE_THUMB)
#define  ARM_SVC_MODE_ARM      (0x00000013uL + ARM_MODE_ARM)

#define  OS_NTASKS_FP          (OS_MAX_TASKS + OS_N_SYS_TASKS - 1u)
#define  OS_FP_STORAGE_SIZE            128u

/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  INT16U  OSTmrCtr;
#endif

#if OS_CPU_FPU_EN > 0u
static  OS_MEM  *OSFPPartPtr;                    /* Ptr to memory partition for storing FPU registers  */
static  INT32U   OSFPPart[OS_NTASKS_FP][OS_FP_STORAGE_SIZE / sizeof(INT32U)];
#endif


/*
*********************************************************************************************************
*                                        INITIALIZE FP SUPPORT
*
* Description: This function initializes the memory partition used to save FPU registers
*              during a context switch.  This function MUST be called AFTER calling
*              OSInit(). OS_CPU_FPU_EN must be defined > 0 in order to compile FPU support into the
*              build.
*
* Arguments  : none
*
* Returns    : none
*
* Note(s)    : 1) Tasks that are to use FP support MUST be created with OSTaskCreateExt().
*              2) For the ARM VFP, 128 bytes are required to save the VFP context.
*                 The INT32U data type is used to ensure that storage is aligned on a 32-bit boundary.
*              3) If you need to perform floating point operations from within the OSStatTaskHook(),
*                 then you must change the 'Options' attribute for OSTaskCreatExt() when creating
*                 the statistics task. This only applies if OS_TaskStat() was created with OSTaskCreateExt().
*********************************************************************************************************
*/

#if OS_CPU_FPU_EN > 0u
void  OS_CPU_FP_Init (void)
{
    INT8U    err;
#if (OS_TASK_STAT_EN > 0u) && (OS_TASK_CREATE_EXT_EN > 0u)
    OS_TCB  *ptcb;
    void    *pblk;
#endif


    OSFPPartPtr = OSMemCreate(&OSFPPart[0][0], OS_NTASKS_FP, OS_FP_STORAGE_SIZE, &err);

#if (OS_TASK_STAT_EN > 0u) && (OS_TASK_CREATE_EXT_EN > 0u)/* CHANGE 'OPTIONS' for OS_TaskStat()        */
    ptcb            = OSTCBPrioTbl[OS_TASK_STAT_PRIO];
    ptcb->OSTCBOpt |= OS_TASK_OPT_SAVE_FP;                /* Allow floating-point support for Stat task*/
    pblk            = OSMemGet(OSFPPartPtr, &err);        /* Get storage for VFP registers             */
    if (pblk != (void *)0) {                              /* Did we get a memory block?                */
        ptcb->OSTCBExtPtr = pblk;                         /* Yes, Link to task's TCB                   */
        OS_CPU_FP_Save(pblk);                             /*      Save the VFP registers in block      */
    }
#endif
}
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
    INT32U   size;
    OS_STK  *pstk;

                                                           /* Clear exception stack for stack checking.*/
    pstk = &OS_CPU_ExceptStk[0];
    size = OS_CPU_EXCEPT_STK_SIZE;
    while (size > 0u) {
        size--;
       *pstk = (OS_STK)0;
    }

#if OS_STK_GROWTH == 1u
    OS_CPU_ExceptStkBase = &OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE - 1u];
#else
    OS_CPU_ExceptStkBase = &OS_CPU_ExceptStk[0];
#endif

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
#if OS_CPU_INT_DIS_MEAS_EN > 0u
    OS_CPU_IntDisMeasInit();
#endif

#if OS_CPU_FPU_EN > 0u
    OS_CPU_FP_Init();                            /* Initialize support for VFP register save / restore */
#endif
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
#if OS_CPU_FPU_EN > 0u
    INT8U  err;
    void  *pblk;
#endif


#if OS_CPU_FPU_EN > 0u
    if (ptcb->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {  /* See if task needs FP support                       */
        pblk = OSMemGet(OSFPPartPtr, &err);      /* Yes, Get storage for VFP registers                 */
        if (pblk != (void *)0) {                 /*      Did we get a memory block?                    */
            ptcb->OSTCBExtPtr = pblk;            /*      Yes, Link to task's TCB                       */
            OS_CPU_FP_Save(pblk);                /*           Save the VFP registers in block          */
        }
    }
#endif

#if OS_APP_HOOKS_EN > 0u
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                  /* Prevent compiler warning                           */
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
#if OS_CPU_FPU_EN > 0u
    if (ptcb->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {            /* See if task had FP support               */
        if (ptcb->OSTCBExtPtr != (void *)0) {              /* Yes, OSTCBExtPtr must not be NULL        */
            OSMemPut(OSFPPartPtr, ptcb->OSTCBExtPtr);      /*      Return memory block to free pool    */
        }
    }
#endif

#if OS_APP_HOOKS_EN > 0u
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                            /* Prevent compiler warning                 */
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
*              2) All tasks run in SVC mode.
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)
{
    OS_STK *stk;
    INT32U  task_addr;


    opt       = opt;                             /* 'opt' is not used, prevent warning                 */
    stk       = ptos;                            /* Load stack pointer                                 */
    task_addr = (INT32U)task & ~1u;              /* Mask off lower bit in case task is thumb mode      */
    *(stk)    = (INT32U)task_addr;               /* Entry Point                                        */
    *(--stk)  = (INT32U)OS_TaskReturn;           /* R14 (LR)                                           */
    *(--stk)  = (INT32U)0x12121212uL;            /* R12                                                */
    *(--stk)  = (INT32U)0x11111111uL;            /* R11                                                */
    *(--stk)  = (INT32U)0x10101010uL;            /* R10                                                */
    *(--stk)  = (INT32U)0x09090909uL;            /* R9                                                 */
    *(--stk)  = (INT32U)0x08080808uL;            /* R8                                                 */
    *(--stk)  = (INT32U)0x07070707uL;            /* R7                                                 */
    *(--stk)  = (INT32U)0x06060606uL;            /* R6                                                 */
    *(--stk)  = (INT32U)0x05050505uL;            /* R5                                                 */
    *(--stk)  = (INT32U)0x04040404uL;            /* R4                                                 */
    *(--stk)  = (INT32U)0x03030303uL;            /* R3                                                 */
    *(--stk)  = (INT32U)0x02020202uL;            /* R2                                                 */
    *(--stk)  = (INT32U)0x01010101uL;            /* R1                                                 */
    *(--stk)  = (INT32U)p_arg;                   /* R0 : argument                                      */
    if ((INT32U)task & 0x01u) {                  /* See if task runs in Thumb or ARM mode              */
        *(--stk) = (INT32U)ARM_SVC_MODE_THUMB;   /* CPSR  (Enable IRQ and FIQ interrupts, THUMB-mode)  */
    } else {
        *(--stk) = (INT32U)ARM_SVC_MODE_ARM;     /* CPSR  (Enable IRQ and FIQ interrupts, ARM-mode)    */
    }

    return (stk);
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
#if OS_CPU_FPU_EN > 0u
    void  *pblk;
#endif

#if OS_CPU_FPU_EN > 0u                                     /* Save VFP context of preempted task       */
    if (OSRunning == OS_TRUE) {                            /* Don't save on OSStart()!                 */
        if (OSTCBCur->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {    /* See if task used FP                      */
            pblk = OSTCBCur->OSTCBExtPtr;                  /* Yes, Get pointer to FP storage area      */
            if (pblk != (void *)0) {                       /*      Make sure we have storage           */
                OS_CPU_FP_Save(pblk);                      /*      Save the VFP registers in block     */
            }
        }
    }
                                                           /* Restore VFP context of new task          */
    if (OSTCBHighRdy->OSTCBOpt & OS_TASK_OPT_SAVE_FP) {    /* See if new task uses FP                  */
        pblk = OSTCBHighRdy->OSTCBExtPtr;                  /* Yes, Get pointer to FP storage area      */
        if (pblk != (void *)0) {                           /*      Make sure we have storage           */
            OS_CPU_FP_Restore(pblk);                       /*      Get contents of VFP registers       */
        }
    }
#endif

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
    (void)ptcb;                                            /* Prevent compiler warning                 */
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
*                             INTERRUPT DISABLE TIME MEASUREMENT, START
*********************************************************************************************************
*/

#if OS_CPU_INT_DIS_MEAS_EN > 0u
void  OS_CPU_IntDisMeasInit (void)
{
    OS_CPU_IntDisMeasNestingCtr = 0u;
    OS_CPU_IntDisMeasCntsEnter  = 0u;
    OS_CPU_IntDisMeasCntsExit   = 0u;
    OS_CPU_IntDisMeasCntsMax    = 0u;
    OS_CPU_IntDisMeasCntsDelta  = 0u;
    OS_CPU_IntDisMeasCntsOvrhd  = 0u;
    OS_CPU_IntDisMeasStart();                              /* Measure the overhead of the functions    */
    OS_CPU_IntDisMeasStop();
    OS_CPU_IntDisMeasCntsOvrhd  = OS_CPU_IntDisMeasCntsDelta;
}


void  OS_CPU_IntDisMeasStart (void)
{
    OS_CPU_IntDisMeasNestingCtr++;
    if (OS_CPU_IntDisMeasNestingCtr == 1u) {               /* Only measure at the first nested level   */
        OS_CPU_IntDisMeasCntsEnter = OS_CPU_IntDisMeasTmrRd();
    }
}


void  OS_CPU_IntDisMeasStop (void)
{
    OS_CPU_IntDisMeasNestingCtr--;                                      /* Decrement nesting ctr       */
    if (OS_CPU_IntDisMeasNestingCtr == 0u) {
        OS_CPU_IntDisMeasCntsExit  = OS_CPU_IntDisMeasTmrRd();
        OS_CPU_IntDisMeasCntsDelta = OS_CPU_IntDisMeasCntsExit - OS_CPU_IntDisMeasCntsEnter;
        if (OS_CPU_IntDisMeasCntsDelta > OS_CPU_IntDisMeasCntsOvrhd) {  /* Ensure overhead < delta     */
            OS_CPU_IntDisMeasCntsDelta -= OS_CPU_IntDisMeasCntsOvrhd;
        } else {
            OS_CPU_IntDisMeasCntsDelta  = OS_CPU_IntDisMeasCntsOvrhd;
        }
        if (OS_CPU_IntDisMeasCntsDelta > OS_CPU_IntDisMeasCntsMax) {    /* Track MAXIMUM               */
            OS_CPU_IntDisMeasCntsMax = OS_CPU_IntDisMeasCntsDelta;
        }
    }
}
#endif


/*
*********************************************************************************************************
*                                     INITIALIZE EXCEPTION VECTORS
*
* Description : This function initialize exception vectors to the default handlers.
*
* Arguments   : None.
*********************************************************************************************************
*/

void  OS_CPU_InitExceptVect (void)
{
    (*(INT32U *)OS_CPU_ARM_EXCEPT_UNDEF_INSTR_VECT_ADDR)       =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_UNDEF_INSTR_HANDLER_ADDR)    = (INT32U)OS_CPU_ARM_ExceptUndefInstrHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_SWI_VECT_ADDR)               =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_SWI_HANDLER_ADDR)            = (INT32U)OS_CPU_ARM_ExceptSwiHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_PREFETCH_ABORT_VECT_ADDR)    =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_PREFETCH_ABORT_HANDLER_ADDR) = (INT32U)OS_CPU_ARM_ExceptPrefetchAbortHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_DATA_ABORT_VECT_ADDR)        =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_DATA_ABORT_HANDLER_ADDR)     = (INT32U)OS_CPU_ARM_ExceptDataAbortHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_ADDR_ABORT_VECT_ADDR)        =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_ADDR_ABORT_HANDLER_ADDR)     = (INT32U)OS_CPU_ARM_ExceptAddrAbortHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_IRQ_VECT_ADDR)               =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_IRQ_HANDLER_ADDR)            = (INT32U)OS_CPU_ARM_ExceptIrqHndlr;

    (*(INT32U *)OS_CPU_ARM_EXCEPT_FIQ_VECT_ADDR)               =         OS_CPU_ARM_INSTR_JUMP_TO_HANDLER;
    (*(INT32U *)OS_CPU_ARM_EXCEPT_FIQ_HANDLER_ADDR)            = (INT32U)OS_CPU_ARM_ExceptFiqHndlr;
}


/*
*********************************************************************************************************
*                              GET NUMBER OF FREE ENTRIES IN EXCEPTION STACK
*
* Description : This function computes the number of free entries in the exception stack.
*
* Arguments   : None.
*
* Returns     : The number of free entries in the exception stack.
*********************************************************************************************************
*/

INT32U  OS_CPU_ExceptStkChk (void)
{
    OS_STK  *pchk;
    INT32U   nfree;
    INT32U   size;


    nfree = 0;
    size  = OS_CPU_EXCEPT_STK_SIZE;
#if OS_STK_GROWTH == 1u
    pchk = &OS_CPU_ExceptStk[0];
    while ((*pchk++ == (OS_STK)0) && (size > 0u)) {   /* Compute the number of zero entries on the stk */
        nfree++;
        size--;
    }
#else
    pchk = &OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE - 1u];
    while ((*pchk-- == (OS_STK)0) && (size > 0u)) {   /* Compute the number of zero entries on the stk */
        nfree++;
        size--;
    }
#endif
    return (nfree);
}
