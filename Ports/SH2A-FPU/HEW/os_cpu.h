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
*                                    Renesas SH-2A-FPU Specific code
*                           Renesas SH SERIES C/C++ Compiler (V.9.00.03.006)
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef _OS_CPU_H
#define _OS_CPU_H

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

#include <machine.h>

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char       BOOLEAN;
typedef unsigned char       INT8U;              /* Unsigned  8 bit quantity                          */
typedef signed   char       INT8S;              /* Signed    8 bit quantity                          */
typedef unsigned short      INT16U;             /* Unsigned 16 bit quantity                          */
typedef signed   short      INT16S;             /* Signed   16 bit quantity                          */
typedef unsigned long       INT32U;             /* Unsigned 32 bit quantity                          */
typedef signed   long       INT32S;             /* Signed   32 bit quantity                          */
typedef unsigned long long  INT64U;             /* Unsigned 64 bit quantity                          */
typedef signed   long long  INT64S;             /* Signed   64 bit quantity                          */
typedef float               FP32;               /* Single precision floating point                   */
typedef double              FP64;               /* Double precision floating point                   */

typedef unsigned long       OS_STK;             /* Each stack entry is 32-bit wide                   */
typedef unsigned long       OS_CPU_SR;          /* The status register (SR) is 32-bits wide          */

/*
*********************************************************************************************************
*                                           Renesas SH-2A-FPU
*
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
                                                /* Disable interrupts */
#define  OS_ENTER_CRITICAL()    set_imask( 15 ) /* When using a ROM Monitor e.g. HMON change this level so not to mask
                                                   out the debugger interrupts */

                                                /* Enable  interrupts */
#define  OS_EXIT_CRITICAL()     set_imask( 0 )
#endif

#if      OS_CRITICAL_METHOD == 2
/* Not currently supported */

/* The accepted way of implementing method 2 is to store the status register (SR - containing the */
/* current interrupt mask level) on to the stack before changing the mask level to a value */
/* which masks all interrupts.  To do this without a function call (which would use the stack */
/* when called) requires the use of in line assembler.  Unfortunately, there does not appear to */
/* be away for the Renesas compiler's inline assembler code to tell the compiler which */
/* registers are being clobbered so that the compiler can take appropriate action. */

/* When the Renesas compiler requires a temporary scratch area within a function it can make room */
/* on the stack by adjusting the stack pointer.  Any access to the scratch area then uses an offset */
/* from the stack pointer.  So, if inline assembly code adjusts the stack pointer, as it would when */
/* storing the current SR value, any reference to the scratch area is broken until the value is popped */
/* from the stack and the stack pointer contents return to the value expected by the compiler. */
/* In tests this has been seen to break critical functions including 'OSTCBInit' and */
/* 'OSTaskCreate'. */

/* The only solution I have come up with to this problem is to compile the code to assembler and */
/* manually alter the affected code which is not really acceptable and so far untested. */

/* Therefore it is suggested that method 3, below, is used */
#endif

#if      OS_CRITICAL_METHOD == 3
                                                /* Disable interrupts */
#define  OS_ENTER_CRITICAL()    cpu_sr = get_cr(); set_imask( 15 )
                                                /* When using a ROM Monitor e.g. HMON change this level so not to mask
                                                   out the debugger interrupts */

                                                /* Enable  interrupts */
#define  OS_EXIT_CRITICAL()     set_cr( cpu_sr )
#endif

/*
*********************************************************************************************************
*                                           Renesas SH-2A-FPU Miscellaneous
*********************************************************************************************************
*/

#define  OS_STK_GROWTH      1                   /* Stack grows from HIGH to LOW memory on SH-2A-FPU  */

#define  uCOS               33                  /* Interrupt vector # used for context switch    */

#define  OS_TASK_SW()       trapa( uCOS );  /* TRAPA instruction                           */

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            PROTOTYPES
*********************************************************************************************************
*/

void    OSStartHighRdy(void);
void    OSIntCtxSw    (void);
void    OSCtxSw       (void);

INT32U  OS_Get_GBR    (void);

#endif
