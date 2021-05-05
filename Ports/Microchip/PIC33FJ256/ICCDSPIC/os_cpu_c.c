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
*                                         dsPIC33 IAR Compiler
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#include  "uCOS_II.H"

/*
*********************************************************************************************************
*                                             LOCALS
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
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookBegin (void)
{
#if OS_TMR_EN > 0
    OSTmrCtr = 0;
#endif

    SPLIM = 0xFFFE;
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
#if OS_CPU_INT_DIS_MEAS_EN > 0
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
#if OS_CPU_HOOKS_EN > 0
void  OSTaskCreateHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                                         /* Prevent compiler warning                                 */
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
#if OS_APP_HOOKS_EN > 0
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                                         /* Prevent compiler warning                                 */
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
#if OS_APP_HOOKS_EN > 0
    App_TaskStatHook();
#endif
}
#endif


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
void  OSTaskSwHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskSwHook();
#endif
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
#if OS_APP_HOOKS_EN > 0
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                                         /* Prevent compiler warning                                 */
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
* Note(s)    : 1) You may pass a task creation parameters through the opt variable. You MUST only use the
*                 upper 8 bits of 'opt' because the lower bits are reserved by uC/OS-II.  If you make
*                 changes to the code below, you will need to ensure that it doesn't affect the behaviour
*                 of OSTaskIdle() and OSTaskStat().
*              2) Registers are initialized to make them easy to differentiate with a debugger.
*
*              3) Setup the stack frame of the task:
*
*                        ptos -  0  ->
*                        ptos -  2  ->  CORCON
*                        ptos -  4  ->  SR (initialized to 0)
*                        ptos -  6  ->  DOENDH
*                        ptos -  8  ->  DOENDL
*                        ptos - 10  ->  DOSTARTH
*                        ptos - 12  ->  DOSTARTL
*                        ptos - 14  ->  DCOUNT
*                        ptos - 16  ->  RCOUNT
*                        ptos - 18  ->  PSVPAG
*                        ptos - 20  ->  TBLPAG
*                        ptos - 22  ->  ACCBU
*                        ptos - 24  ->  ACCBH
*                        ptos - 26  ->  ACCBL
*                        ptos - 28  ->  ACCAU
*                        ptos - 30  ->  ACCAH
*                        ptos - 32  ->  ACCAL
*                        ptos - 34  ->  W14
*                        ptos - 36  ->  W13
*                        ptos - 38  ->  W12
*                        ptos - 40  ->  W11
*                        ptos - 42  ->  W10
*                        ptos - 44  ->  W9
*                        ptos - 46  ->  W8
*                        ptos - 48  ->  W7
*                        ptos - 50  ->  W6
*                        ptos - 52  ->  W5
*                        ptos - 54  ->  W4
*                        ptos - 56  ->  W3
*                        ptos - 58  ->  W2
*                        ptos - 60  ->  W1
*                        ptos - 62  ->  p_arg
*                        ptos - 64  ->   0 (15..8) | CORCON.7 | PC (22..16)      Simulate ISR
*                        ptos - 66  ->  PC (15..0)
*                        ptos - 68  ->  PC (22..16)                              Simulate function call
*                        ptos - 70  ->  PC (15..0)
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    INT16U  x;
    INT16U   pc_high;


	pc_high =   0;                                                      /* Upper byte of PC always 0. Pointers are 16 bit unsigned  */

   *ptos++  =  (OS_STK)task;                                            /* Simulate a call to the task by putting 32 bits of data   */
   *ptos++  =  (OS_STK)pc_high;                                         /* data on the stack.                                       */

                                                                        /* Simulate an interrupt                                    */
   *ptos++  =  (OS_STK)task;                                            /* Put the address of this task on the stack (PC)           */

    x       =  0;                                                       /* Set the SR to enable ALL interrupts                      */
    if (CORCON & 0x0008) {                                              /* Check the CPU's current interrupt level. Bit 3 in CORCON */
        x  |= 0x0080;                                                   /* If set, then save the priority level bit in x bit [7]    */
    }
   *ptos++  = (OS_STK)(x | (INT16U)pc_high);                            /* Push the SR Low, CORCON IPL3 and PC (22..16)             */

                                                                        /* Push all of the registers to stack                       */
   *ptos++  = (OS_STK)p_arg;                                            /* Register W0 holds data passed to the task when started   */
   *ptos++  = 0x1111;                                                   /* Initialize register W1                                   */
   *ptos++  = 0x2222;                                                   /* Initialize register W2                                   */
   *ptos++  = 0x3333;                                                   /* Initialize register W3                                   */
   *ptos++  = 0x4444;                                                   /* Initialize register W4                                   */
   *ptos++  = 0x5555;                                                   /* Initialize register W5                                   */
   *ptos++  = 0x6666;                                                   /* Initialize register W6                                   */
   *ptos++  = 0x7777;                                                   /* Initialize register W7                                   */
   *ptos++  = 0x8888;                                                   /* Initialize register W8                                   */
   *ptos++  = 0x9999;                                                   /* Initialize register W9                                   */
   *ptos++  = 0xAAAA;                                                   /* Initialize register W10                                  */
   *ptos++  = 0xBBBB;                                                   /* Initialize register W11                                  */
   *ptos++  = 0xCCCC;                                                   /* Initialize register W12                                  */
   *ptos++  = 0xDDDD;                                                   /* Initialize register W13                                  */
   *ptos++  = 0xEEEE;                                                   /* Initialize register W14                                  */

   *ptos++  = ACCAL;                                                    /* Push Accumulator A onto the stack                        */
   *ptos++  = ACCAH;                                                    /* Push Accumulator A onto the stack                        */
   *ptos++  = ACCAU;                                                    /* Push Accumulator A onto the stack                        */
   *ptos++  = ACCBL;                                                    /* Push Accumulator B onto the stack                        */
   *ptos++  = ACCBH;                                                    /* Push Accumulator B onto the stack                        */
   *ptos++  = ACCBU;                                                    /* Push Accumulator B onto the stack                        */
   *ptos++  = TBLPAG;                                                   /* Push the Data Table Page Address onto the stack          */
   *ptos++  = PSVPAG;                                                   /* Push the Program Space Visability Register on the stack  */
   *ptos++  = RCOUNT;                                                   /* Push the Repeat Loop Counter Register onto the stack     */
   *ptos++  = DCOUNT;                                                   /* Push the Do Loop     Counter Register onto the stack     */
   *ptos++  = DOSTARTL;                                                 /* Push the Do Loop Start Address Register onto the stack   */
   *ptos++  = DOSTARTH;                                                 /* Push the Do Loop Start Address Register onto the stack   */
   *ptos++  = DOENDL;                                                   /* Push the Do Loop End   Address Register onto the stack   */
   *ptos++  = DOENDH;                                                   /* Push the Do Loop End   Address Register onto the stack   */

   *ptos++  = 0;                                                        /* Force the SR to enable all interrupt, clear flags        */
   *ptos++  = CORCON;                                                   /* Push the Core Control Register on to the stack           */

    return (ptos);                                                      /* Return the stack pointer to the new tasks stack          */
}
