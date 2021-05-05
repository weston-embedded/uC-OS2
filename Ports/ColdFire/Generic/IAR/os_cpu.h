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
*                                        ColdFire Specific code
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef OS_CPU_H
#define OS_CPU_H

#include  <cpu.h>

#ifdef    OS_CPU_GLOBALS
#define   OS_CPU_EXT
#else
#define   OS_CPU_EXT  extern
#endif


/*
**********************************************************************************************************
*                                           DATA TYPES
**********************************************************************************************************
*/

typedef  CPU_BOOLEAN    BOOLEAN;
typedef  CPU_INT08U     INT8U;                                          /* Unsigned  8 bit quantity                                 */
typedef  CPU_INT08S     INT8S;                                          /* Signed    8 bit quantity                                 */
typedef  CPU_INT16U     INT16U;                                         /* Unsigned 16 bit quantity                                 */
typedef  CPU_INT16S     INT16S;                                         /* Signed   16 bit quantity                                 */
typedef  CPU_INT32U     INT32U;                                         /* Unsigned 32 bit quantity                                 */
typedef  CPU_INT32S     INT32S;                                         /* Signed   32 bit quantity                                 */
typedef  CPU_FP32       FP32;                                           /* Single precision floating point                          */
typedef  CPU_FP64       FP64;                                           /* Double precision floating point                          */

typedef  CPU_STK        OS_STK;                                         /* Define size of CPU stack entry                           */
typedef  CPU_SR         OS_CPU_SR;                                      /* Define size of CPU status register                       */


/*
*********************************************************************************************************
*                                           Critical Method MACROS
*********************************************************************************************************
*/

#define  OS_CRITICAL_METHOD     CPU_CFG_CRITICAL_METHOD

#define  OS_ENTER_CRITICAL()   {CPU_CRITICAL_ENTER();}
#define  OS_EXIT_CRITICAL()    {CPU_CRITICAL_EXIT();}


/*
**********************************************************************************************************
*                                          Miscellaneous
**********************************************************************************************************
*/

#define  OS_TASK_SW()          __asm("TRAP #14")                        /* Use Trap #14 to perform a Task Level Context Switch */

#define  OS_STK_GROWTH             1                                    /* Define stack growth: 1 = Down, 0 = Up               */


/*
*********************************************************************************************************
*                                          ColdFire Specifics
*********************************************************************************************************
*/

#define  OS_INITIAL_SR        0x2000                                    /* Supervisor mode, interrupts enabled                 */

#define  OS_TRAP_NBR              14                                    /* OSCtxSw() invoked through TRAP #14                  */


/*
**********************************************************************************************************
*                                         Function Prototypes
**********************************************************************************************************
*/

void  OSStartHighRdy(void);
void  OSIntCtxSw    (void);
void  OSCtxSw       (void);


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of CPU cfg module inclusion.                     */
