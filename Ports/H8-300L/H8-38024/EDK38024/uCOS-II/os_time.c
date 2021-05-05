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
*                                            TIME MANAGEMENT
*
* Filename : os_time.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef  OS_MASTER_FILE
#include <ucos_ii.h>
#endif

/*
*********************************************************************************************************
*                                DELAY TASK 'n' TICKS   (n from 0 to 65535)
*
* Description: This function is called to delay execution of the currently running task until the
*              specified number of system ticks expires.  This, of course, directly equates to delaying
*              the current task for some time to expire.  No delay will result If the specified delay is
*              0.  If the specified delay is greater than 0 then, a context switch will result.
*
* Arguments  : ticks     is the time delay that the task will be suspended in number of clock 'ticks'.
*                        Note that by specifying 0, the task will not be delayed.
*
* Returns    : none
*********************************************************************************************************
*/

void  OSTimeDly (INT16U ticks)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    INT8U      y;


    if (ticks > 0) {                             /* 0 means no delay!                                  */
        OS_ENTER_CRITICAL();
        y            =  OSTCBCur->OSTCBY;        /* Delay current task                                 */
        OSRdyTbl[y] &= ~OSTCBCur->OSTCBBitX;
        if (OSRdyTbl[y] == 0) {
            OSRdyGrp &= ~OSTCBCur->OSTCBBitY;
        }
        OSTCBCur->OSTCBDly = ticks;              /* Load ticks in TCB                                  */
        OS_EXIT_CRITICAL();
        OS_Sched();                              /* Find next task to run!                             */
    }
}
