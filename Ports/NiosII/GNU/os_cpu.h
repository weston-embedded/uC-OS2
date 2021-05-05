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
*                                          Altera NiosII Port
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
* For       : Altera NiosII
* Toolchain : GNU - Altera NiosII
*********************************************************************************************************
*/

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2003-5 Altera Corporation, San Jose, California, USA.         *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
*                                                                             *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/

#ifndef __OS_CPU_H__
#define __OS_CPU_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "sys/alt_irq.h"

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif


/*
*********************************************************************************************************
*                                             DATA TYPES
*                                        (Compiler Specific)
*********************************************************************************************************
*/

                                                                /* This is the definition for Nios32.                   */
typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                                   /* Unsigned  8 bit quantity                             */
typedef signed   char  INT8S;                                   /* Signed    8 bit quantity                             */
typedef unsigned short INT16U;                                  /* Unsigned 16 bit quantity                             */
typedef signed   short INT16S;                                  /* Signed   16 bit quantity                             */
typedef unsigned long  INT32U;                                  /* Unsigned 32 bit quantity                             */
typedef signed   long  INT32S;                                  /* Signed   32 bit quantity                             */
typedef float          FP32;                                    /* Single precision floating point                      */
typedef double         FP64;                                    /* Double precision floating point                      */
typedef unsigned int   OS_STK;                                  /* Each stack entry is 32-bits                          */

/*
*********************************************************************************************************
*                                               MACROS
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        1                                 /* Stack grows from HIGH to LOW memory                  */
#define  OS_TASK_SW           OSCtxSw

/*
*********************************************************************************************************
*                              Disable and Enable Interrupts - 2 methods
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical
*             section, interrupts will be enabled even if they were disabled before
*             entering the critical section.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In
*             other words, if interrupts were disabled before entering the critical
*             section, they will be disabled when leaving the critical section.
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
#error OS_CRITICAL_METHOD == 1 not supported, please use method 3 instead.
#endif

#if      OS_CRITICAL_METHOD == 2
#error OS_CRITICAL_METHOD == 2 not supported, please use method 3 instead.
#endif

#if      OS_CRITICAL_METHOD == 3
#define  OS_CPU_SR alt_irq_context
#define  OS_ENTER_CRITICAL() \
         cpu_sr = alt_irq_disable_all ()
#define  OS_EXIT_CRITICAL() \
         alt_irq_enable_all (cpu_sr);
#endif

/*
*********************************************************************************************************
*                                             PROTOTYPES
*********************************************************************************************************
*/

void  OSStartHighRdy(void);
void  OSCtxSw       (void);
void  OSIntCtxSw    (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OS_CPU_H__ */
