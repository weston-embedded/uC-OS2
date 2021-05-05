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
*                                             TI C28x Port
*
* Filename  : os_cpu.h
* Version   : V2.93.01
*********************************************************************************************************
* For       : TI C28x
* Mode      : C28 Object mode
* Toolchain : TI C/C++ Compiler
*********************************************************************************************************
*/

#ifndef  OS_CPU_H
#define  OS_CPU_H

#ifdef   OS_CPU_GLOBALS
#define  OS_CPU_EXT
#else
#define  OS_CPU_EXT  extern
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                              OS TICK INTERRUPT PRIORITY CONFIGURATION
*
* Note(s) : (1) For systems that don't need any high, real-time priority interrupts; the tick interrupt
*               should be configured as the highest priority interrupt but won't adversely affect system
*               operations.
*
*           (2) For systems that need one or more high, real-time interrupts; these should be configured
*               higher than the tick interrupt which MAY delay execution of the tick interrupt.
*
*               (a) If the higher priority interrupts do NOT continually consume CPU cycles but only
*                   occasionally delay tick interrupts, then the real-time interrupts can successfully
*                   handle their intermittent/periodic events with the system not losing tick interrupts
*                   but only increasing the jitter.
*
*               (b) If the higher priority interrupts consume enough CPU cycles to continually delay the
*                   tick interrupt, then the CPU/system is most likely over-burdened & can't be expected
*                   to handle all its interrupts/tasks. The system time reference gets compromised as a
*                   result of losing tick interrupts.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef  unsigned  char    BOOLEAN;
typedef  unsigned  char    INT8U;                               /* Unsigned  8 bit quantity                             */
typedef  signed    char    INT8S;                               /* Signed    8 bit quantity                             */
typedef  unsigned  short   INT16U;                              /* Unsigned 16 bit quantity                             */
typedef  signed    short   INT16S;                              /* Signed   16 bit quantity                             */
typedef  unsigned  long    INT32U;                              /* Unsigned 32 bit quantity                             */
typedef  signed    long    INT32S;                              /* Signed   32 bit quantity                             */
typedef            float   FP32;                                /* Single precision floating point                      */
typedef  long      double  FP64;                                /* Double precision floating point                      */

typedef  unsigned  short   OS_STK;                              /* Each stack entry is 16-bit wide                      */
typedef  unsigned  short   OS_CPU_SR;                           /* Define size of CPU status register (INTM = 1 bit)    */


/*
*********************************************************************************************************
*                                                 C28x
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
#define  OS_ENTER_CRITICAL()  {cpu_sr = OS_CPU_SR_Save();}
#define  OS_EXIT_CRITICAL()   {OS_CPU_SR_Restore(cpu_sr);}
#endif


/*
*********************************************************************************************************
*                                           C28x Miscellaneous
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        0u                                /* Stack grows from LOW to HIGH memory on C28x          */


/*
*********************************************************************************************************
*                                              OS_TASK_SW
*
* Note(s): OS_TASK_SW()  invokes the task level context switch.
*
*          (1) On some processors, this corresponds to a call to OSCtxSw() which is an assembly language
*              function that performs the context switch.
*
*          (2) On some processors, you need to simulate an interrupt using a 'software interrupt' or a
*              TRAP instruction.  Some compilers allow you to add in-line assembly language as shown.
*********************************************************************************************************
*/

#define  OS_TASK_SW()         asm(" TRAP #16")


/*
*********************************************************************************************************
*                                              OSIntCtxSw
*
* Note(s): OSIntCtxSw()  invokes the interrupt level context switch.
*
*          (1) On some processors, this corresponds to a call to OSIntCtxSw() which is an assembly language
*              function that performs the context switch.
*
*          (2) On some processors, you need to simulate an interrupt using a 'software interrupt' or a
*              TRAP instruction.  Some compilers allow you to add in-line assembly language as shown.
*********************************************************************************************************
*/

#define  OSIntCtxSw()         asm(" TRAP #16")


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
                                                                /* Wrapper functions to get ST0 and ST1 registers.      */
INT16U     OS_CPU_GetST0         (void);
INT16U     OS_CPU_GetST1         (void);
                                                                /* OS-II Port Implementation. See OS_CPU_A.ASM          */
#if (OS_CRITICAL_METHOD == 3u)
OS_CPU_SR  OS_CPU_SR_Save        (void);
void       OS_CPU_SR_Restore     (OS_CPU_SR  cpu_sr);
#endif

void       OSStartHighRdy        (void);

void       OS_CPU_INT_Handler    (void);

void       OS_CPU_RTOSINT_Handler(void);

#endif
