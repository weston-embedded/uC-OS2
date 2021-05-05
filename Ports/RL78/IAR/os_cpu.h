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
* Filename  : os_cpu.h
* Version   : V2.93.01
*********************************************************************************************************
* For       : Renesas RL78
* Toolchain : IAR EWRL78 v1.2x and up
*********************************************************************************************************
*/

#ifndef _OS_CPU_H
#define _OS_CPU_H

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

#include  <intrinsics.h>

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned  char        BOOLEAN;
typedef unsigned  char        INT8U;                            /* Unsigned  8 bit quantity                             */
typedef   signed  char        INT8S;                            /* Signed    8 bit quantity                             */
typedef unsigned  short  int  INT16U;                           /* Unsigned 16 bit quantity                             */
typedef   signed  short  int  INT16S;                           /* Signed   16 bit quantity                             */
typedef unsigned  long        INT32U;                           /* Unsigned 32 bit quantity                             */
typedef   signed  long        INT32S;                           /* Signed   32 bit quantity                             */
typedef            float      FP32;                             /* Single precision floating point                      */
typedef            double     FP64;                             /* Double precision floating point                      */

typedef unsigned  short int  OS_STK;                            /* Each stack entry is 16-bit wide                      */
typedef unsigned  char       OS_CPU_SR;                         /* The status register (SR) is 8-bits wide              */

/*
*********************************************************************************************************
*                                               RL78
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*
*********************************************************************************************************
*/
#define  OS_CRITICAL_METHOD    3

#if      OS_CRITICAL_METHOD == 1

                                                                /* Disable interrupts                                   */
#define  OS_ENTER_CRITICAL()    __disable_interrupt()
                                                                /* Enable interrupts                                    */
#define  OS_EXIT_CRITICAL()     __enable_interrupt()

#endif

#if      OS_CRITICAL_METHOD == 2
                                                                /* Disable interrupts                                   */
#define  OS_ENTER_CRITICAL()    __asm("PUSH PSW");                  \
                                __asm("DI");

                                                                /* Enable interrupts                                    */
#define  OS_EXIT_CRITICAL()     __asm("POP PSW");                   \
                                __asm("EI");

#endif

#if      OS_CRITICAL_METHOD == 3
                                                                /* Disable interrupts                                   */
#define  OS_ENTER_CRITICAL()    cpu_sr = __get_interrupt_state();   \
                                __disable_interrupt()
                                                                /* Enable  interrupts                                   */
#define  OS_EXIT_CRITICAL()     __set_interrupt_state(cpu_sr)

#endif

/*
*********************************************************************************************************
*                                         RL78 MISCELLANEOUS
*********************************************************************************************************
*/

#define  OS_STK_GROWTH      1                                   /* Stack grows from HIGH to LOW memory on the NEC 78K0R */

#define  OS_TASK_SW()       __break()                           /* Issues a break instruction                           */

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               PROTOTYPES
*********************************************************************************************************
*/

#if OS_CRITICAL_METHOD == 3u                                    /* See OS_CPU_A.ASM                                    */
void  OSStartHighRdy    (void);
void  OSIntCtxSw        (void);
void  OSCtxSw           (void);

void  OS_CPU_TickInit   (INT32U  tick_per_sec);
#endif
#endif
