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
*                             uCOS-II port for Analog Device's Blackfin 533
*                                           Visual DSP++ 5.0
*
*       This port was made with a large contribution from the Analog Devices Inc development team
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#if !defined(OS_CPU_H)
#define OS_CPU_H


/*
*********************************************************************************************************
*                                            GLOBAL DEFINES
*********************************************************************************************************
*/
#if ( defined(_LANGUAGE_ASM) || defined(_LANGUAGE_C))
                                                /* NESTED & NOT_NESTED Defines for interrupts routines */
#define  NESTED               1
#define  NOT_NESTED           0

#endif

#if defined(_LANGUAGE_C)

#ifdef   OS_CPU_GLOBALS
#define  OS_CPU_EXT
#else
#define  OS_CPU_EXT  extern
#endif

#define  OS_STK_GROWTH        1                 /* Stack grows from HIGH to LOW memory on Blackfin     */
#define  OS_CRITICAL_METHOD   3

#define  IVG0                 0                 /* 16 Interrupts vector number ( 0 to 15)              */
#define  IVG1                 1
#define  IVG2                 2
#define  IVG3                 3
#define  IVG4                 4
#define  IVG5                 5
#define  IVG6                 6
#define  IVG7                 7
#define  IVG8                 8
#define  IVG9                 9
#define  IVG10               10
#define  IVG11               11
#define  IVG12               12
#define  IVG13               13
#define  IVG14               14
#define  IVG15               15


/*
*********************************************************************************************************
*                                            GLOBAL MACROS
*********************************************************************************************************
*/


#define  OS_TASK_SW()  asm("raise 14;");        /* Raise Interrupt 14 (trap)                           */


/*
*********************************************************************************************************
*                                    CRITICAL SECTIONS MANAGEMENT
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
*********************************************************************************************************
*/

#define  OS_ENTER_CRITICAL()  (cpu_sr = OS_CPU_SR_Save())   /* Disable interrupts                      */
#define  OS_EXIT_CRITICAL()   (OS_CPU_SR_Restore(cpu_sr))   /* Enable  interrupts                      */


/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/


typedef unsigned int    BOOLEAN;    /* Unsigned  8 bit quantity                                        */
typedef unsigned char   INT8U;      /* Unsigned  8 bit quantity                                        */
typedef signed   char   INT8S;      /* Signed    8 bit quantity                                        */
typedef unsigned short  INT16U;     /* Unsigned 16 bit quantity                                        */
typedef signed   short  INT16S;     /* Signed   16 bit quantity                                        */
typedef unsigned int    INT32U;     /* Unsigned 32 bit quantity                                        */
typedef signed   int    INT32S;     /* Signed   32 bit quantity                                        */
typedef float           FP32;       /* Single precision floating point                                 */
typedef double          FP64;       /* Double precision floating point                                 */

typedef unsigned int    OS_STK;     /* Each stack entry is 32-bit wide                                 */
typedef unsigned int    OS_CPU_SR;  /* Define size of CPU status register                              */

typedef void (*FNCT_PTR)(void);     /* Pointer to a function which returns void and has no  argument   */

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

extern     void OSStartHighRdy(void);                                        /* See OS_CPU_A.S         */
extern     void OSIntCtxSw(void);                                            /* See OS_CPU_A.S         */

OS_CPU_SR  OS_CPU_SR_Save(void);                                             /* See OS_CPU_A.S         */
void       OS_CPU_SR_Restore(OS_CPU_SR sr);                                  /* See OS_CPU_A.S         */
void       OS_CPU_RegisterHandler(INT8U ivg, FNCT_PTR fn, BOOLEAN nesting);  /* See OS_CPU_C.C         */

#endif   /* _LANGUAGE_C */

#endif /* OS_CPU_H */
