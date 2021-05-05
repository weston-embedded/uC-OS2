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
*                                           Renesas RL78 Port
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
* For       : Renesas RL78
* Toolchain : E2Studios v2.x GNURL78 Compiler v1.x
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>

/*
 *********************************************************************************************************
 *                                            GLOBAL DATA
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                          LOCAL VARIABLES
 *********************************************************************************************************
 */

#if (OS_VERSION >= 281) && (OS_TMR_EN > 0)
static INT16U OSTmrCtr;
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
void OSInitHookBegin(void) {
#if (OS_VERSION >= 281) && (OS_TMR_EN > 0)
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
void OSInitHookEnd(void) {
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
void OSTaskCreateHook(OS_TCB *ptcb) {
#if OS_APP_HOOKS_EN > 0
	App_TaskCreateHook(ptcb);
#else
	(void)ptcb; 												/* Prevent compiler warning                             */
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
void OSTaskDelHook(OS_TCB *ptcb) {
#if OS_APP_HOOKS_EN > 0
	App_TaskDelHook(ptcb);
#else
	(void)ptcb; 												/* Prevent compiler warning                             */
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
void OSTaskIdleHook(void) {
#if OS_APP_HOOKS_EN > 0
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
void OSTaskReturnHook(OS_TCB *ptcb) {
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

#if OS_CPU_HOOKS_EN > 0
void OSTaskStatHook(void) {
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
 *
 *
 * Returns    : Always returns the location of the new top-of-stack' once the processor registers have
 *              been placed on the stack in the proper order.
 *
 *********************************************************************************************************
 */

OS_STK *OSTaskStkInit(void (*task)(void *pd), void *pdata, OS_STK *ptos,
		INT16U opt) {
	OS_STK *p_stk;

	(void) opt;

	p_stk = (INT16U *) ptos; 									/* Load stack pointer                                   */

	*(--p_stk) = (INT16U) ((INT32U) pdata >> 16);
	*(--p_stk) = (INT16U) pdata;
	*(--p_stk) = (INT16U) 0x8600; 								/* PC bits 16-19 in lower 8 bits, psw in upper 16 bits  */
	*(--p_stk) = (INT16U) task; 								/* PC bits 0-15                                         */
	*(--p_stk) = 0x1100; 										/* RP0   =   R1 + R0                                    */
	*(--p_stk) = 0x3322; 										/* RP1   =   R3 + R2                                    */
	*(--p_stk) = 0x5544; 										/* RP2   =   R5 + R4                                    */
	*(--p_stk) = 0x7766; 										/* RP3   =   R7 + R6                                    */
	*(--p_stk) = 0x0F00; 										/* ES:CS =   ES + CS register                           */

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

#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void OSTaskSwHook(void) {
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
void OSTCBInitHook(OS_TCB *ptcb) {
#if OS_APP_HOOKS_EN > 0
	App_TCBInitHook(ptcb);
#else
	(void)ptcb; 												/* Prevent compiler warning                             */
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
void OSTimeTickHook(void) {
#if OS_APP_HOOKS_EN > 0
	App_TimeTickHook();
#endif

#if (OS_VERSION >= 281) && (OS_TMR_EN > 0)
	OSTmrCtr++;
	if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
		OSTmrCtr = 0;
		OSTmrSignal();
	}
#endif

}
#endif
