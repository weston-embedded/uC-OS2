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
*                                         M683xx Specific code
*                                             COSMIC C V4.1
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef  OS_CPU_H
#define  OS_CPU_H

/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned short OS_STK;                   /* Each stack entry is 16-bit wide                    */
typedef unsigned short OS_CPU_SR;                /* Define size of CPU status register (SR = 16 bits)  */

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
*                                           Motorola 683xx
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
*             Note: COSMIC allows an alternate method which doesn't incur the penalty of a function call
*                   and return.  This is done through the use of the _asm() directive.  Note that D7 is
*                   used to pass arguments.
*
* Note(s)  : 1) The COSMIC compiler doesn't currently handle OS_CRITICAL_METHOD #2 because it doesn't
*               correctly keep track of the stack pointer manipulation through 'asm()' calls.  You should
*               thus NOT be using it.
*            2) Method #3 is the preferred method to use to disable insterrupts.
*********************************************************************************************************
*/
#define    OS_CRITICAL_METHOD    3

#if        OS_CRITICAL_METHOD == 1
#define    OS_ENTER_CRITICAL()  _asm("  ORI   #$0700,SR\n")
#define    OS_EXIT_CRITICAL()   _asm("  AND   #$0F800,SR\n")
#endif


#if        OS_CRITICAL_METHOD == 2
#define    OS_ENTER_CRITICAL()  _asm("  MOVE  SR,-(A7)\n  ORI #$0700,SR\n")
#define    OS_EXIT_CRITICAL()   _asm("  MOVE  (A7)+,SR\n")
#endif


#if        OS_CRITICAL_METHOD == 3
#define    OS_ENTER_CRITICAL()  (cpu_sr = OSCPUSaveSR())      /* Disable interrupts                    */
#define    OS_EXIT_CRITICAL()   (OSCPURestoreSR(cpu_sr))      /* Enable  interrupts                    */
#endif


#if 0
#if        OS_CRITICAL_METHOD == 3
#define    OS_ENTER_CRITICAL()  cpu_sr = _asm("CLR.L D7\n MOVE.W SR,D7\n ORI #$0700,SR\n")
#define    OS_EXIT_CRITICAL()   _asm("MOVE.W D7,SR\n", cpu_sr)
#endif
#endif

#define    OS_TASK_SW()         _asm("  TRAP  #15\n")


#define    OS_STK_GROWTH        1                             /* Define stack growth: 1 = Down, 0 = Up */


#define    CPU_INT_DIS()        _asm("  ORI   #$0700,SR\n")   /* Disable interrupts                    */
#define    CPU_INT_EN()         _asm("  AND   #$0F800,SR\n")  /* Enable  interrupts                    */


#define    OS_INITIAL_SR        0x2000                        /* Supervisor mode, interrupts enabled   */

#define    OS_TRAP_NBR              15                        /* OSCtxSw() invoked through TRAP #15    */

/*
*********************************************************************************************************
*                                             PROTOTYPES
*********************************************************************************************************
*/

void       OSIntExit68K(void);

#if      OS_CRITICAL_METHOD == 3
OS_CPU_SR  OSCPUSaveSR(void);                    /* Return the value of the CCR register and then ...  */
                                                 /* ... disable interrupts via SEI instruction.        */
void       OSCPURestoreSR(OS_CPU_SR os_cpu_sr);  /* Set CCR register to 'os_cpu_sr'                    */
#endif

#endif
