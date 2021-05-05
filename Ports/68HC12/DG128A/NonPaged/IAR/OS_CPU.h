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
*                                         68HC12 Specific code
*                                                 IAR
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef OS_CPU_H
#define OS_CPU_H

/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned int   INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   int   INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned char  OS_STK;                   /* Each stack entry is 8-bit wide                     */
typedef unsigned char  OS_CPU_SR;                /* Define size of CPU status register (PSW = 16 bits) */

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#ifndef  FALSE
#define  FALSE    0
#endif

#ifndef  TRUE
#define  TRUE     1
#endif

/*
*********************************************************************************************************
*                                           Motorola 68HC12
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
* NOTE(s)  :  1) The current version of the compiler does NOT allow method #2 to be used without changing
*                the processor independent portion of uC/OS-II.
*             2) The current version of the compiler does NOT allow method #3 either.  However, this can
*                be implemented in OS_CPU_A.S by defining the functions: OSCPUSaveSR() and
*                OSCPURestoreSR().
*********************************************************************************************************
*/
#define  OS_CRITICAL_METHOD    3

#if      OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()  (cpu_sr = OSCPUSaveSR())    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()   (OSCPURestoreSR(cpu_sr))    /* Enable  interrupts                        */
#endif


#define  OS_TASK_SW()          OSCtxSw()

#define  OS_STK_GROWTH        1                  /* Define stack growth: 1 = Down, 0 = Up              */

/*
*********************************************************************************************************
*                                             PROTOTYPES
*********************************************************************************************************
*/
#if      OS_CRITICAL_METHOD == 3
OS_CPU_SR  OSCPUSaveSR(void);                    /* Return the value of the CCR register and then ...  */
                                                 /* ... disable interrupts via SEI instruction.        */
void       OSCPURestoreSR(OS_CPU_SR os_cpu_sr);  /* Set CCR register to 'os_cpu_sr'                    */
#endif

#endif
