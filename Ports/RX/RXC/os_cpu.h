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
*                                      Renesas RX Specific Code
*
* Filename  : os_cpu.h
* Version   : V2.93.01
*********************************************************************************************************
* For       : Renesas RXC compiler
* Toolchain : HEW      with RXC
*             E2STUDIO with RXC
*********************************************************************************************************
*/

#ifndef _OS_CPU_H
#define _OS_CPU_H

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

#include  <cpu.h>

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef  unsigned  char    BOOLEAN;                             /*  8-bit boolean or logical                            */
typedef  unsigned  char    INT8U;                               /*  8-bit unsigned integer                              */
typedef    signed  char    INT8S;                               /*  8-bit   signed integer                              */
typedef  unsigned  short   INT16U;                              /* 16-bit unsigned integer                              */
typedef    signed  short   INT16S;                              /* 16-bit   signed integer                              */
typedef  unsigned  long    INT32U;                              /* 32-bit unsigned integer                              */
typedef    signed  long    INT32S;                              /* 32-bit   signed integer                              */
typedef            float   FP32;                                /* 32-bit floating point                                */
typedef            double  FP64;                                /* 64-bit floating point                                */

typedef  unsigned  long    OS_STK;                              /* Each stack entry is 32-bit wide                      */
typedef  unsigned  long    OS_CPU_SR;                           /* Define size of CPU status register (PSR = 32 bits)   */


/*
*********************************************************************************************************
*                                               MACROS
*********************************************************************************************************
*/

#define  OS_STK_GROWTH             1u                           /* Stack growth (0 == Up. 1 == Down)                    */
#define  OS_TASK_SW()       OSCtxSw()                           /* Context switch through pended call                   */


/*
*********************************************************************************************************
*                                      Critical Section Management
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*             NOT IMPLEMENTED
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.
*             NOT IMPLEMENTED
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/

#define  OS_CRITICAL_METHOD   3u

#if OS_CRITICAL_METHOD == 3u
#ifdef   CPU_CFG_KA_IPL_BOUNDARY
#define  OS_ENTER_CRITICAL()   do { cpu_sr = get_ipl(); \
                                    set_ipl(CPU_CFG_KA_IPL_BOUNDARY); } while (0)
#else
#define  OS_ENTER_CRITICAL()   do { cpu_sr = get_ipl(); \
                                    set_ipl(12);     } while (0)
#endif
                                                                /* Restore CPU status word.                             */
#define  OS_EXIT_CRITICAL()    do { set_ipl(cpu_sr); } while (0)
#endif


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void        OSCtxSw               (void);
void        OSIntCtxSw            (void);
void        OSStartHighRdy        (void);

void        OSCtxSwISR            (void);

CPU_INT32U  OS_KA_IPL_BoundaryGet (void);

#endif
