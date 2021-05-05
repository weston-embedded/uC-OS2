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
*                                           MPC57xx VLE Port
*                                             GNU Toolchain
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef _OS_CPU_H
#define _OS_CPU_H

#ifdef   OS_CPU_GLOBALS
#define  OS_CPU_EXT
#else
#define  OS_CPU_EXT  extern
#endif


/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*
* Note(s) : (1) C++ compilers MUST 'extern'ally declare ALL C function prototypes & variable/object
*               declarations for correct C language linkage.
*********************************************************************************************************
*/

#ifdef __cplusplus
extern  "C" {                                    /* See Note #1.                                       */
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#ifndef  OS_CPU_ISR_STK_SIZE
#define  OS_CPU_ISR_STK_SIZE      256u           /* Default ISR stack size is 256 OS_STK entries       */
#endif


/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef  unsigned  char    BOOLEAN;              /*  8-bit boolean or logical                          */
typedef  unsigned  char    INT8U;                /*  8-bit unsigned integer                            */
typedef    signed  char    INT8S;                /*  8-bit   signed integer                            */
typedef  unsigned  short   INT16U;               /* 16-bit unsigned integer                            */
typedef    signed  short   INT16S;               /* 16-bit   signed integer                            */
typedef  unsigned  long    INT32U;               /* 32-bit unsigned integer                            */
typedef    signed  long    INT32S;               /* 32-bit   signed integer                            */
typedef            float   FP32;                 /* 32-bit floating point                              */
typedef            double  FP64;                 /* 64-bit floating point                              */

typedef  unsigned  long    OS_STK;               /* Each stack entry is 32-bit wide                    */
typedef  unsigned  long    OS_CPU_SR;            /* Define size of CPU status register (PSR = 32 bits) */


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
#define  OS_ENTER_CRITICAL()  do { cpu_sr = OS_CPU_SR_Save();} while (0)
#define  OS_EXIT_CRITICAL()   do { OS_CPU_SR_Restore(cpu_sr);} while (0)
#endif


/*
*********************************************************************************************************
*                                               MACROS
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

#define  OS_TASK_SW()                              asm __volatile__ (" se_sc ");
#define  OS_STK_GROWTH                             1u         /* Stack grows from HIGH to LOW memory   */


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if OS_CRITICAL_METHOD == 3u                      /* See OS_CPU_A.S                                    */
OS_CPU_SR  OS_CPU_SR_Save   (void);
void       OS_CPU_SR_Restore(OS_CPU_SR  cpu_sr);
#endif

void       OSCtxSw          (void);
void       OSIntCtxSw       (void);
void       OSStartHighRdy   (void);
void       OS_CPU_IntHandler(void);


/*
*********************************************************************************************************
*                                   EXTERNAL C LANGUAGE LINKAGE END
*********************************************************************************************************
*/

#ifdef __cplusplus
}                                                 /* End of 'extern'al C lang linkage.                 */
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif
