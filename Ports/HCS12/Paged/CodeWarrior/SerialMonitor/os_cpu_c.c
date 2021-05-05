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
*                                         MC9S12 Specific Code
*                                       Freescale Serial Monitor
*                                          Banked Memory Model
*                                            Codewarrior 4.x
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
* Note(s)  : (1) This port will not function in expanded mode. This is due to the fact that the
*                OS_TASK_SW() macro utilizes JSR instead of CALL.  The CALL instruction may not
*                be used in place of JSR since it pushes the PPAGE register on the stack which
*                will corrupts the stack frame. JSR only pushes the PC on to the stack which
*                is compatible the MC9S12 ISR exception stack handling procedures (see RTI).
*
*            (2) This port utilizes JSR instead of SWI when performing a task level context switch
*                due to the fact that the Freescale MC9S12 Serial Monitor utilizes the SWI trap
*                for break points.
*********************************************************************************************************
*/

#include  "uCOS_II.H"

/*
*********************************************************************************************************
*                                         PROTOTYPES
*********************************************************************************************************
*/

void  OSTaskAbort(void);

/*
*********************************************************************************************************
*                                           LOCALS
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
* Note(s)    : 1) XIRQ interrupts are disabled when your task starts executing. You can change this by
*                 clearing BIT6 in the CCR.
*              2) The STOP instruction is disabled when your task starts executing.  You can change this
*                 by clearing BIT7 in the CCR.
*              3) The other interrupts (i.e. maskable interrupts) are enabled when your task starts
*                 executing.  You can change this by setting BIT4 in the CCR.
*              4) You can change pass the above options in the 'opt' argument.  You MUST only use the
*                 upper 8 bits of 'opt' because the lower bits are reserved by uC/OS-II.  If you make
*                 changes to the code below, you will need to ensure that it doesn't affect the behaviour
*                 of OSTaskIdle() and OSTaskStat().
*              5) Registers are initialized to make them easy to differentiate with a debugger.
*              6) All your tasks are assumed to be in NON-PAGED memory.  However, the tasks can call
*                 functions in PAGED memory and if a context switch occurs, the proper page will be
*                 saved/restored by uC/OS-II.  Page #0 is stored onto the task's stack as a 'dummy'
*                 value.
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt)
{
    INT8U  *bstk;


    (void)&opt;                                     /* 'opt' is not used, prevent warning                     */

    bstk    = (INT8U *)ptos;                        /* Load stack pointer                                     */


    *--bstk = (INT8U)(((INT32U)OSTaskAbort) >>  8); /* Task return address low  (simulated calling function)  */
    *--bstk = (INT8U)(((INT32U)OSTaskAbort) >> 16); /* Task return address high (simulated calling function)  */
    *--bstk = (INT8U)(((INT32U)OSTaskAbort));       /* Task return address page (simulated calling function)  */


                                                    /* Format: PCH:PCL:PPAGE                                  */
    *--bstk = (INT8U)(((INT32U)task) >> 8);         /* Task start address PCL                                 */
    *--bstk = (INT8U)(((INT32U)task) >> 16);        /* Task start address PCH.                                */


    *--bstk = (INT8U)0x22;                          /* Y Register low                                         */
    *--bstk = (INT8U)0x22;                          /* Y Register high                                        */


    *--bstk = (INT8U)0x11;                          /* X Register low                                         */
    *--bstk = (INT8U)0x11;                          /* X Register high                                        */


    *--bstk = (INT8U)(((INT16U)p_arg) >> 8);        /* A register / D register high task argument             */
    *--bstk = (INT8U) ((INT16U)p_arg);              /* B register / D register low  task argument             */


    *--bstk = (0xC0);                               /* CCR Register (Disable STOP instruction and XIRQ)       */


    *--bstk = (INT8U)task;                          /* Save the PPAGE register                                */

    return ((OS_STK *)bstk);                        /* Return pointer to new top-of-stack                     */
}



/*
*********************************************************************************************************
*                                           TASK ABORT HANDLER
*
* Description: This function serves as the caller function for all tasks.  If a task accidently returns,
*              the caller will regain control of the CPU and delete the task preventing a system crash..
*
* Arguments  : none
*
* Note(s)    : In uC/OS-II, a task should NEVER exit and should instead call OSTaskDel(OS_PRIO_SELF)
*              when no longer desired.
*********************************************************************************************************
*/

void  OSTaskAbort (void)
{
    OSTaskDel(OS_PRIO_SELF);
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
#if OS_CPU_HOOKS_EN > 0
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
