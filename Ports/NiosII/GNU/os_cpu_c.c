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
*                                          Altera NiosII Port
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
* For       : Altera NiosII
* Toolchain : GNU - Altera NiosII
*********************************************************************************************************
*/

#include <reent.h>
#include <string.h>

#include <stddef.h>

#define  OS_CPU_GLOBALS
#include "includes.h"                                           /* Standard includes for uC/OS-II                       */

#include "system.h"

extern void OSStartTsk;                                         /* The entry point for all tasks.                       */

#if OS_TMR_EN > 0
static  INT16U  OSTmrCtr;
#endif

/*
*********************************************************************************************************
*                                      INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to
*              initialize the stack frame of the task being created.  This function is
*              highly processor specific.
*
* What it does: It builds up initial stack for a task.
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
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void    (*task)(void *p_arg),
                        void     *pdata,
                        OS_STK   *ptos,
                        INT16U    opt)
{
   INT32U   *frame_pointer;
   INT32U   *stk;

#if OS_THREAD_SAFE_NEWLIB
   struct _reent* local_impure_ptr;

   /*
    * create and initialise the impure pointer used for Newlib thread local storage.
    * This is only done if the C library is being used in a thread safe mode. Otherwise
    * a single reent structure is used for all threads, which saves memory.
    */

   local_impure_ptr = (struct _reent*)((((INT32U)(ptos)) & ~0x3) - sizeof(struct _reent));

   _REENT_INIT_PTR (local_impure_ptr);

   /*
    * create a stack frame at the top of the stack (leaving space for the
    * reentrant data structure).
    */

   frame_pointer = (INT32U*) local_impure_ptr;
#else
   frame_pointer = (INT32U*) (((INT32U)(ptos)) & ~0x3);
#endif /* OS_THREAD_SAFE_NEWLIB */
   stk = frame_pointer - 13;

   /* Now fill the stack frame. */

   stk[12] = (INT32U)task;            /* task address (ra) */
   stk[11] = (INT32U)pdata;           /* first register argument (r4) */

#if OS_THREAD_SAFE_NEWLIB
   stk[10] = (INT32U) local_impure_ptr; /* value of _impure_ptr for this thread */
#endif /* OS_THREAD_SAFE_NEWLIB */
   stk[0]  = ((INT32U)&OSStartTsk) + 4;/* exception return address (ea) */

   /* The next three lines don't generate any code, they just put symbols into
    * the debug information which will later be used to navigate the thread
    * data structures
    */
   __asm__ (".set OSTCBNext_OFFSET,%0" :: "i" (offsetof(OS_TCB, OSTCBNext)));
   __asm__ (".set OSTCBPrio_OFFSET,%0" :: "i" (offsetof(OS_TCB, OSTCBPrio)));
   __asm__ (".set OSTCBStkPtr_OFFSET,%0" :: "i" (offsetof(OS_TCB, OSTCBStkPtr)));

   return((OS_STK *)stk);
}

#if OS_CPU_HOOKS_EN
/*
*********************************************************************************************************
*                                         TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  OSTaskCreateHook (OS_TCB  *ptcb)
{
    ptcb = ptcb;                                                /* Prevent compiler warning                             */
}


/*
*********************************************************************************************************
*                                         TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  OSTaskDelHook (OS_TCB  *ptcb)
{
    ptcb = ptcb;                                                /* Prevent compiler warning                             */
}


/*
*********************************************************************************************************
*                                          TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform
*              other operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

void  OSTaskSwHook (void)
{
}


/*
*********************************************************************************************************
*                                         STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/

void  OSTaskStatHook (void)
{
}


/*
*********************************************************************************************************
*                                              TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

/*
 * Iniche stack has no header declaration for its timer 'hook'.
 * Do that here to avoid build warnings.
 */
#ifdef ALT_INICHE
void  cticks_hook (void);
#endif

void  OSTimeTickHook (void)
{
#if OS_TMR_EN > 0
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif

#ifdef ALT_INICHE
    /* Service the Interniche timer */
    cticks_hook();
#endif
}

void  OSInitHookBegin (void)
{
#if OS_TMR_EN > 0
    OSTmrCtr = 0;
#endif
}

void  OSInitHookEnd (void)
{
}

void  OSTaskIdleHook (void)
{
}

void  OSTaskReturnHook (OS_TCB  *ptcb)
{
}

void  OSTCBInitHook (OS_TCB  *ptcb)
{
}

#endif
