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
*                                         Synopsys ARC EM6 Port
*
* Filename  : os_cpu.h
* Version   : V2.93.01
*********************************************************************************************************
* For       : Synopsys ARC EM6
* Mode      : Little-Endian, 32 registers, FPU, Code Density, Loop Counter, Stack Check
* Toolchain : MetaWare C/C++ Clang-based Compiler
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
*                                         CONFIGURATION PRAGMAS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
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

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (STATUS = 32 bits) */


/*
*********************************************************************************************************
*                                           Synopsys ARC EM6
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
*                                     Synopsys ARC EM6 Miscellaneous
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        1u                  /* Stack grows from HIGH to LOW memory on ARM        */

#define  OS_TASK_SW()         OSCtxSw()


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if OS_CRITICAL_METHOD == 3u                      /* See OS_CPU_A.S                                    */
OS_CPU_SR  OS_CPU_SR_Save    (void);
void       OS_CPU_SR_Restore (OS_CPU_SR cpu_sr);
#endif

void  OSCtxSw                    (void);
void  OSIntCtxSw                 (void);

void  OSStartHighRdy             (void);

void  OS_CPU_EM6_ExceptionHandler(void);
void  OS_CPU_EM6_InterruptHandler(void);
void  OS_CPU_EM6_TrapHandler     (void);

                                                  /* Must be implemented in the BSP for the OS.        */
void  OS_CPU_ExceptionHandler    (INT8U  except_id);
void  OS_CPU_InterruptHandler    (INT8U  int_id);


#endif
