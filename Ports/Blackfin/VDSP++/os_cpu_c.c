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
*                             uCOS-II port for Analog Device's Blackfin 533
*                                           Visual DSP++ 5.0
*
*       This port was made with a large contribution from the Analog Devices Inc development team
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include <ucos_ii.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  EVENT_VECTOR_TABLE_ADDR  0xFFE02000            /* Event vector table start address            */
#define  IPEND                    0xFFE02108            /* Interrupt Pending Register                  */
#define  IPEND_BIT_4_MASK         0xFFFFFFEF            /* IPEND Register bit 4 mask                   */
#define  IVG_NUM                          16            /* Interrupt vector number                     */
#define  pIPEND  ((volatile unsigned long *)IPEND)      /* Pointer to IPEND Register                   */

/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

static  FNCT_PTR  OS_CPU_IntHanlderTab[IVG_NUM] = {(void *)0};

/*
*********************************************************************************************************
*                                             LOCAL FUNCTIONS
*********************************************************************************************************
*/

void    OSTaskCreateHook       (OS_TCB *ptcb);
void    OSTaskDelHook          (OS_TCB *ptcb);
void    OSTaskSwHook           (void);
void    OSTCBInitHook          (OS_TCB *ptcb);
void    OSInitHookBegin        (void);
void    OSInitHookEnd          (void);
void    OSTaskStatHook         (void);
void    OSTimeTickHook         (void);
void    OSTaskIdleHook         (void);
void    OS_CPU_IntHandler      (void);
void    OS_CPU_RegisterHandler (INT8U ivg, FNCT_PTR fn, BOOLEAN nesting);
OS_STK  *OSTaskStkInit         (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt);

/*
*********************************************************************************************************
*                                            EXTERNAL FUNCTIONS
*********************************************************************************************************
*/

extern  void  OSCtxSw                    (void);        /* See OS_CPU_A.S                              */
extern  void  OS_CPU_NESTING_ISR         (void);        /* See OS_CPU_A.S                              */
extern  void  OS_CPU_NON_NESTING_ISR     (void);        /* See OS_CPU_A.S                              */
extern  void  OS_CPU_EnableIntEntry      (INT8U mask);  /* See OS_CPU_A.S                              */
extern  void  OS_CPU_DisableIntEntry     (INT8U mask);  /* See OS_CPU_A.S                              */
extern  void  OS_CPU_Invalid_Task_Return (void);        /* See OS_CPU_A.S                              */

/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*                                   void  OSTaskCreateHook(OS_TCB *ptcb)
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Returns    : None
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
    (void)ptcb;                                         /* Prevent compiler warning                    */
#endif

}
#endif

/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*                                        void  OSTaskDelHook(OS_TCB *ptcb)
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Returns    : None
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0
void  OSTaskDelHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                         /* Prevent compiler warning                    */
#endif
}
#endif

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*                                        void  OSTaskSwHook (void)
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : None
*
* Returns    : None
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
* Returns    : None
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSTCBInitHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                         /* Prevent compiler warning                    */
#endif
}
#endif


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                      void  OSInitHookBegin (void)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : None
*
* Returns    : None
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
*                                       OS INITIALIZATION HOOK (END)
*                                       void  OSInitHookEnd (void)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : None
*
* Returns    : None
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookEnd (void)
{

    INT32U *pEventVectorTable;

    pEventVectorTable = ((INT32U*)EVENT_VECTOR_TABLE_ADDR);    /* Pointer to Event Vector Table        */
    pEventVectorTable[IVG14] = (INT32U)&OSCtxSw;               /* Register the context switch          */
                                                               /* handler for IVG14                    */
    OS_CPU_EnableIntEntry(IVG14);                              /* Enable Interrupt for IVG14           */

}
#endif

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*                                        void  OSTaskStatHook (void)
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : None
*
* Returns    : None
*
* Note(s)    : None
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0
void  OSTaskStatHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskStatHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*                                       void  OSTimeTickHook (void)
*
* Description: This function is called every tick.
*
* Arguments  : None
*
* Returns    : None
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/

#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TimeTickHook();
#endif
}
#endif



/*
*********************************************************************************************************
*                                               IDLE HOOK
*                                       void  OSTaskIdleHook (void)
*
* Description: This function is called by uC/OS-ii IDLE Task.
*
* Arguments  : None
*
* Returns    : None
*
* Note(s)    : 1) This hook function is called when the IDLE task executes - place code here that
*                 needs to be done when the processor is 'idle' i.e. no tasks are being executed.
*
*              2) Interrupts should be disabled during this call.
*********************************************************************************************************
*/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
void  OSTaskIdleHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskIdleHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                           ISR HANDLER REGISTRATION
*                          void OS_CPU_RegisterHandler(INT8U ivg, FNCT_PTR fn, BOOLEAN nesting)
*
* Description : Registers Interrupts handler routine within the event table OS_CPU_IntrHanlderTab.
*               Chooses OS_CPU_NESTING_ISR or OS_CPU_NON_NESTING_ISR as ISR Handler depending on the value
*               of nesting argument.
*               Enables Interrupt for a given IVG (first argument)
*
* Arguments   : INT8U ivg       : interrupt vector groupe number (IVG0 to IVG15)
*               FNCT_PTR fn     : function pointer to the handler routine for a given IVG
*               BOOLEAN nesting : to choose Nested ISR or not
*
* Returns     : None
*
* Note(s)     : 1) IVG14 is reserved for task-level context switching
*               2) IVG6  is used for Core Timer to drive the OS ticks
*********************************************************************************************************
*/

void  OS_CPU_RegisterHandler (INT8U ivg, FNCT_PTR fn, BOOLEAN nesting)
{
    INT32U *pEventVectorTable;


    if (ivg > IVG15) {                                            /* The Blackfin 533 supports only 16 vectors  */
        return;
    }

    if (ivg == IVG14) {                                           /* IVG14 is reserved for task-level context   */
        return;                                                   /* switching                                  */
    }

    if (ivg == IVG4) {                                            /* Reserved vector                            */
        return;
    }

    pEventVectorTable = (INT32U*)EVENT_VECTOR_TABLE_ADDR;         /* pEventVectorTable points to the start      */
                                                                  /* address of Event vector table              */
    if (nesting == NESTED) {
        pEventVectorTable[ivg] = (INT32U)&OS_CPU_NESTING_ISR;     /* Select Nested ISR if nesting is required   */
                                                                  /* for the given ivg                          */
    } else {
        pEventVectorTable[ivg] = (INT32U)&OS_CPU_NON_NESTING_ISR; /* Select Non Nested ISR if nesting is not    */
                                                                  /* required for the given ivg                 */
    }

    OS_CPU_IntHanlderTab[ivg] = fn;                               /* Register Handler routine for the given ivg */
    OS_CPU_EnableIntEntry(ivg);

}

/*
*********************************************************************************************************
*                                           ISR HANDLER GLOBAL ROUTINE
*                                          void OS_CPU_IntHandler(void)
*
* Description : This routine is called by  OS_CPU_NON_NESTING_ISR or OS_CPU_NESTING_ISR when an
*               interrupt occurs. It determines the high priority pending interrupt request
*               depending on the value of IPEND register, and then selects the Handler routine for that
*               IRQ using OS_CPU_IntrHanlderTab table.
*
* Arguments   : None
*
* Returns     : None
*
* Note(s)     : None
*********************************************************************************************************
*/

void  OS_CPU_IntHandler (void)
{
    INT32U  status;
    INT32U  mask;
    INT8U   i;


    mask   = 1;
    status = *pIPEND & IPEND_BIT_4_MASK;                  /* Use IPEND_BIT_4_MASK to avoid testing     */
                                                          /* IPEND[4] :disable interrupts              */
    for (i =0; i < IVG_NUM; i++) {

        if ((1 << i) == (status & mask)) {

            if (OS_CPU_IntHanlderTab[i] != (void *)0) {   /* Be sure that Handler routine was          */
                                                          /* registered                                */
                 OS_CPU_IntHanlderTab[i]();               /* Branch to the Handler routine             */

            }
            break;

        }
        mask <<=1;
    }
}

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*             OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
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
* Note(s)    : This function does the following (refer to Porting chapter (Chapter 13 of uCOS-II book)
*               (1) Simulate a function call to the task with an argument
*               (2) Simulate ISR vector
*               (3) Setup stack frame to contain desired initial values of all registers
*               (4) Return top of stack pointer to the caller
*
*            Refer to VisualDSP++ C/C++ Compiler and Library Manual for Blackfin Processors
*            and ADSP-BF53x/BF56x Blackfin(R) Processor Programming Reference Manual.
*
*             The convention for the task frame (after context save is complete) is as follows:
*                      (stack represented from high to low memory as per convention)
*                                          (*** High memory ***) R0
*                                                                P1
*                                                                RETS       (function return address of thread)
*                                                                R1
*                                                                R2
*                                                                P0
*                                                                P2
*                                                                ASTAT
*                                                                RETI      (interrupt return address: $PC of thread)
*                                                                R7:3    (R7 is lower than R3)
*                                                                P5:3    (P5 is lower than P3)
*                                                                FP        (frame pointer)
*                                                                I3:0    (I3 is lower than I0)
*                                                                B3:0    (B3 is lower than B0)
*                                                                L3:0    (L3 is lower than L0)
*                                                                M3:0    (M3 is lower than M0)
*                                                                A0.x
*                                                                A0.w
*                                                                A1.x
*                                                                A1.w
*                                                                LC1:0    (LC1 is lower than LC0)
*                                                                LT1:0    (LT1 is lower than LT0)
*            OSTCBHighRdy--> OSTCBStkPtr --> (*** Low memory ***)LB1:0    (LB1 is lower than LB0)
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{
    OS_STK  *stk;
    INT8U   i;

    opt    = opt;                                 /* 'opt' is not used, prevent warning                    */
    stk    = ptos;                                /* Load stack pointer                                    */

                                                  /* Simulate a function call to the task with an argument */
    stk   -= 3;                                   /* 3 words assigned for incoming args (R0, R1, R2)       */

                                                  /* Now simulating vectoring to an ISR                    */
    *--stk = (OS_STK) pdata;                      /* R0 value - caller's incoming argument #1              */
    *--stk = (OS_STK) 0;                          /* P1 value - value irrelevant                           */

    *--stk = (OS_STK)OS_CPU_Invalid_Task_Return;  /* RETS value - NO task should return with RTS.          */
                                                  /* however OS_CPU_Invalid_Task_Return is a safety        */
                                                  /* catch-allfor tasks that return with an RTS            */

    *--stk = (OS_STK) pdata;                      /* R1 value - caller's incoming argument #2              */
                                                  /* (not relevant in current test example)                */
    *--stk = (OS_STK) pdata;                      /* R2 value - caller's incoming argument #3              */
                                                  /* (not relevant in current test example)                */
    *--stk = (OS_STK) 0;                          /* P0 value - value irrelevant                           */
    *--stk = (OS_STK) 0;                          /* P2 value - value irrelevant                           */
    *--stk = (OS_STK) 0;                          /* ASTAT value - caller's ASTAT value - value            */
                                                  /* irrelevant                                            */

    *--stk = (OS_STK) task;                       /* RETI value- pushing the start address of the task     */

    for (i = 35; i>0; i--) {                      /* remaining reg values - R7:3, P5:3,                    */
                                                  /* 4 words of A1:0(.W,.X), LT0, LT1,                     */
        *--stk = (OS_STK)0;                       /* LC0, LC1, LB0, LB1,I3:0, M3:0, L3:0, B3:0,            */
    }                                             /* All values irrelevant                                 */

    return ((OS_STK *)stk);                       /* Return top-of-stack                                   */
}
