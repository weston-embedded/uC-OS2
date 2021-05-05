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
*                                             ARMv7-R Port
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
* For      : ARMv7-R Cortex-R
* Mode     : ARM or Thumb
**********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*
* Note(s) : 1) ARM_MODE_ARM       is the CPSR bit mask for ARM Mode
*           2) ARM_MODE_THUMB     is the CPSR bit mask for THUMB Mode
*           3) ARM_SVC_MODE_THUMB is the CPSR bit mask for SVC MODE + THUMB Mode
*           4) ARM_SVC_MODE_ARM   is the CPSR bit mask for SVC MODE + ARM Mode
*********************************************************************************************************
*/

#if (OS_CPU_ARM_ENDIAN_TYPE == OS_CPU_ARM_ENDIAN_LITTLE)
#define  ARM_MODE_ARM           0x00000000u
#define  ARM_MODE_THUMB         0x00000020u
#else                                                           /* Set bit 9 in big-endian mode.                        */
#define  ARM_MODE_ARM           0x00000200u
#define  ARM_MODE_THUMB         0x00000220u
#endif

#define  ARM_SVC_MODE_THUMB    (0x00000013u + ARM_MODE_THUMB)
#define  ARM_SVC_MODE_ARM      (0x00000013u + ARM_MODE_ARM)


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
*********************************************************************************************************
*/
#if OS_CPU_HOOKS_EN > 0u
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
                                                                /* Align the ISR stack to 8-bytes                       */
    OS_CPU_ExceptStkBase = (OS_STK *)&OS_CPU_ExceptStk[OS_CPU_EXCEPT_STK_SIZE];
    OS_CPU_ExceptStkBase = (OS_STK *)((OS_STK)(OS_CPU_ExceptStkBase) & 0xFFFFFFF8);

    OS_CPU_ARM_DRegCnt = OS_CPU_ARM_DRegCntGet();

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
    (void)ptcb;                                             /* Prevent compiler warning                           */
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
    (void)ptcb;                                             /* Prevent compiler warning                 */
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
* Note(s)   : (1) Interrupts are enabled when task starts executing.
*
*             (2) All tasks run in SVC mode.
*
*             (3) There are three differents stack frames depending on whether or not the Floating-Point (FP) co-processor
*                 is enabled or not.
*
*                 (a) The stack frame shown in the diagram is used when the FP coprocessor is present and
*                     OS_OPT_TASK_SAVE_FP is enabled. In this case the FP exception register, the FP registers and the
*                     FP control/status register are saved in the stack frame.
*
*                 (b) If the FP co-processor is present but the OS_OPT_TASK_SAVE_FP is not set, only the FP
*                     exception register is saved in the stack.
*
*                     (1) The FP exception register is saved twice in the stack frame to keep the 8-byte aligment.
*                         (See note #4.)
*
*                    +-----------+
*                    |   FPEXC   |
*                    +-----------+
*                    |    S0     |
*                    +-----------+
*                          .
*                          .
*                          .
*                    +-----------+
*                    |    S29    |
*                    +-----------+
*                    |    S30    |
*                    +-----------+       +-----------+
*                    |    S31    |       |   FPEXC   |
*                    +-----------+       +-----------+
*                    |   FPSCR   |       |   FPEXC   |
*                    +-----------+       +-----------+      +-----------+
*                    |   CPSR    |       |   CPSR    |      |   CPSR    |
*                    +-----------+       +-----------+      +-----------+
*                    |    R0     |       |     R0    |      |    R0     |
*                    +-----------+       +-----------+      +-----------+
*                          .                  .                  .
*                          .                  .                  .
*                          .                  .                  .
*                    +-----------+       +-----------+      +-----------+
*                    |    R10    |       |    R10    |      |    R10    |
*                    +-----------+       +-----------+      +-----------+
*                    |    R11    |       |    R11    |      |    R11    |
*                    +-----------+       +-----------+      +-----------+
*                    |    R12    |       |    R12    |      |    R12    |
*                    +-----------+       +-----------+      +-----------+
*                    |  R14 (LR) |       |  R14 (LR) |      |  R14 (LR) |
*                    +-----------+       +-----------+      +-----------+
*                    | PC = Task |       | PC = Task |      | PC = Task |
*                    +-----------+       +-----------+      +-----------+
*
*                        (a)                 (b)                (c)
*
*             (4) The SP must be 8-byte aligned in conforming to the Procedure Call Standard for the ARM architecture
*
*                    (a) Section 2.1 of the  ABI for the ARM Architecture Advisory Note. SP must be 8-byte aligned
*                        on entry to AAPCS-Conforming functions states :
*
*                        The Procedure Call Standard for the ARM Architecture [AAPCS] requires primitive
*                        data types to be naturally aligned according to their sizes (for size = 1, 2, 4, 8 bytes).
*                        Doing otherwise creates more problems than it solves.
*
*                        In return for preserving the natural alignment of data, conforming code is permitted
*                        to rely on that alignment. To support aligning data allocated on the stack, the stack
*                        pointer (SP) is required to be 8-byte aligned on entry to a conforming function. In
*                        practice this requirement is met if:
*
*                           (1) At each call site, the current size of the calling function's stack frame is a multiple of 8 bytes.
*                               This places an obligation on compilers and assembly language programmers.
*
*                           (2) SP is a multiple of 8 when control first enters a program.
*                               This places an obligation on authors of low level OS, RTOS, and runtime library
*                               code to align SP at all points at which control first enters
*                               a body of (AAPCS-conforming) code.
*
*                       In turn, this requires the value of SP to be aligned to 0 modulo 8:
*
*                           (3) By exception handlers, before calling AAPCS-conforming code.
*
*                           (4) By OS/RTOS/run-time system code, before giving control to an application.
*
*                 (b) Section 2.3.1 corrective steps from the the SP must be 8-byte aligned on entry
*                     to AAPCS-conforming functions advisory note also states.
*
*                     " This requirement extends to operating systems and run-time code for all architecture versions
*                       prior to ARMV7 and to the A, R and M architecture profiles thereafter. Special considerations
*                       associated with ARMV7M are discussed in section 2.3.3"
*
*                     (1) Even if the SP 8-byte aligment is not a requirement for the ARMv7M profile, the stack is aligned
*                         to 8-byte boundaries to support legacy execution enviroments.
*
*                 (c) Section 5.2.1.2 from the Procedure Call Standard for the ARM
*                     architecture states :  "The stack must also conform to the following
*                     constraint at a public interface:
*
*                     (1) SP mod 8 = 0. The stack must be double-word aligned"
*
*                 (d) From the ARM Technical Support Knowledge Base. 8 Byte stack aligment.
*
*                     "8 byte stack alignment is a requirement of the ARM Architecture Procedure
*                      Call Standard [AAPCS]. This specifies that functions must maintain an 8 byte
*                      aligned stack address (e.g. 0x00, 0x08, 0x10, 0x18, 0x20) on all external
*                      interfaces. In practice this requirement is met if:
*
*                      (1) At each external interface, the current stack pointer
*                          is a multiple of 8 bytes.
*
*                      (2) Your OS maintains 8 byte stack alignment on its external interfaces
*                          e.g. on task switches"
**********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void  *p_arg),
                        void               *p_arg,
                        OS_STK             *ptos,
                        INT16U              opt)
{
    OS_STK  *p_stk;
    INT32U   task_addr;
    INT32U   fpu_reg_cnt;
    INT8U    i;


    (void)opt;

    p_stk     =  ptos + 1u;                                     /* Load stack pointer                                   */
    p_stk     = (OS_STK *)((OS_STK)(p_stk) & 0xFFFFFFF8u);      /* Align stack pointer.                                 */
    task_addr = (INT32U)task & ~1u;                             /* Mask off lower bit in case task is thumb mode        */

    *--p_stk  = (OS_STK)task_addr;                              /* Entry Point                                          */
    *--p_stk  = (OS_STK)OS_TaskReturn;                          /* R14 (LR)                                             */
    *--p_stk  = (OS_STK)0x12121212u;                            /* R12                                                  */
    *--p_stk  = (OS_STK)0x11111111u;                            /* R11                                                  */
    *--p_stk  = (OS_STK)0x10101010u;                            /* R10                                                  */
    *--p_stk  = (OS_STK)0x09090909u;                            /* R9                                                   */
    *--p_stk  = (OS_STK)0x08080808u;                            /* R8                                                   */
    *--p_stk  = (OS_STK)0x07070707u;                            /* R7                                                   */
    *--p_stk  = (OS_STK)0x06060606u;                            /* R6                                                   */
    *--p_stk  = (OS_STK)0x05050505u;                            /* R5                                                   */
    *--p_stk  = (OS_STK)0x04040404u;                            /* R4                                                   */
    *--p_stk  = (OS_STK)0x03030303u;                            /* R3                                                   */
    *--p_stk  = (OS_STK)0x02020202u;                            /* R2                                                   */
    *--p_stk  = (OS_STK)0x01010101u;                            /* R1                                                   */
    *--p_stk  = (OS_STK)p_arg;                                  /* R0 : argument                                        */


    if (((INT32U)task & 0x01u) == 0x01u) {                      /* See if task runs in Thumb or ARM mode                */
       *--p_stk = (OS_STK)(ARM_SVC_MODE_THUMB);                 /* Set Thumb mode.                                      */
    } else {
       *--p_stk = (OS_STK)(ARM_SVC_MODE_ARM);
    }

    fpu_reg_cnt = OS_CPU_ARM_DRegCntGet();

    if(fpu_reg_cnt != 0u) {
        *--p_stk = (OS_STK)0;                                   /* Initialize Floating point status & control register  */
                                                                /* Initialize general-purpose Floating point registers  */
         for (i = 0u; i < fpu_reg_cnt * 2u; i++) {
             *--p_stk = (OS_STK)0;
         }

        *--p_stk = (OS_STK)(0x40000000);                        /* Initialize Floating-Point Exception Register (Enable)*/
    }

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
*              3) If debug variables are enabled, the current process id is saved into the context ID register
*                 found in the system control coprocessor. The Embedded Trace Macrocell (ETM) and the debug logic
*                 use this register. The ETM can broadcast its value to indicate the process that is running currently.
*
*                     (a) The proccess id is formed by concatenating the current task priority with the lower 24 bits
*                         from the current task TCB.
*
*                            31              24                     0
*                             +---------------+---------------------+
*                             | OSPrioHighRdy | OSTCBHighRdy[23..0] |
*                             +---------------+---------------------+
*********************************************************************************************************
*/

#if (OS_CPU_HOOKS_EN > 0u) && (OS_TASK_SW_HOOK_EN > 0u)
void  OSTaskSwHook (void)

{
#if OS_CFG_DBG_EN > 0u
    INT32U  ctx_id;
#endif

#if OS_APP_HOOKS_EN > 0u
    App_TaskSwHook();
#endif

    OS_TRACE_TASK_SWITCHED_IN(OSTCBHighRdy);

#if OS_CFG_DBG_EN > 0u
    ctx_id = ((INT32U)(OSPrioHighRdy    << 24u)             )
           | ((INT32U)(OSTCBHighRdy           ) & 0x00FFFFFF);
    OS_CPU_ARM_CtxID_Set(ctx_id);
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
    pchk  = &OS_CPU_ExceptStk[0];
    while ((*pchk++ == (OS_STK)0) && (size > 0u)) {   /* Compute the number of zero entries on the stk */
        nfree++;
        size--;
    }

    return (nfree);
}
