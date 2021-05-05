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
*                                   Freescale DSP568xx Specific code
*                        Metrowerks CodeWarrior for Freescale Embedded DSP568xx
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef OS_CPU_H
#define OS_CPU_H

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
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

typedef unsigned int   OS_STK;                   /* Each stack entry is 16-bit wide                    */
typedef unsigned short OS_CPU_SR;                /* Define size of CPU status register (PSW = 16 bits) */

#define BYTE           INT8S                     /* Define data types for backward compatibility ...   */
#define UBYTE          INT8U                     /* ... to uC/OS V1.xx.  Not actually needed for ...   */
#define WORD           INT16S                    /* ... uC/OS-II.                                      */
#define UWORD          INT16U
#define LONG           INT32S
#define ULONG          INT32U

OS_CPU_EXT OS_CPU_SR cpu_sr;

/*
*********************************************************************************************************
*                              Freescale DSP568xx
*
* Method #1:  Not implemented.
*
* Method #2:  Not implemented.
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/

#if 0

        /* These routines emphasize speed at the expense of space */

        inline void OS_DISABLE_INTERRUPTS (register volatile OS_CPU_SR * cpu_sr)
        {
              asm(move.w  SR,x:(cpu_sr));
              asm(bfset   #$0300,SR);
              asm(nop);
              asm(nop);
              asm(nop);
              asm(nop);
              asm(nop);
        }

        inline void OS_ENABLE_INTERRUPTS (register OS_CPU_SR cpu_sr)
        {
              asm(moveu.w  cpu_sr,SR);
              asm(nop);
              asm(nop);
              asm(nop);
              asm(nop);
              asm(nop);
        }

#else

        /* These routines emphasize space at the expense of speed */

        asm void OS_DISABLE_INTERRUPTS (OS_CPU_SR * cpu_sr);

        asm void OS_ENABLE_INTERRUPTS (OS_CPU_SR cpu_sr);

#endif


#define  OS_CRITICAL_METHOD    3

#if      OS_CRITICAL_METHOD == 1
#error "OS_CRITICAL_METHOD #1 - Not Implemented"
#endif

#if      OS_CRITICAL_METHOD == 2
#error "OS_CRITICAL_METHOD #2 - Not Implemented"
#endif


#if      OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()                                     OS_DISABLE_INTERRUPTS (&cpu_sr)

#define  OS_EXIT_CRITICAL()                                      OS_ENABLE_INTERRUPTS  (cpu_sr)
#endif

/*
*********************************************************************************************************
*                           Freescale DSP568xx Miscellaneous
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        0                       /* Stack grows from LOW to HIGH memory on DPS568xx  */

#define  OS_TASK_SW()         asm(swi)

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

#endif
