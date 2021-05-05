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
*                                    Freescale MPC56xx Specific code
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef __OS_CPU_H__                                                    /* test for multiple inclusion                              */
#define __OS_CPU_H__

/*
*********************************************************************************************************
*                                             INCLUDES
*********************************************************************************************************
*/

#ifdef   OS_CPU_GLOBALS
#define  OS_CPU_EXT
#else
#define  OS_CPU_EXT  extern
#endif


/*
*********************************************************************************************************
*                                           DATA TYPES
*********************************************************************************************************
*/

typedef  unsigned  char    BOOLEAN;
typedef  unsigned  char    INT8U;                               /* Unsigned  8 bit quantity                             */
typedef    signed  char    INT8S;                               /* Signed    8 bit quantity                             */
typedef  unsigned  short   INT16U;                              /* Unsigned 16 bit quantity                             */
typedef    signed  short   INT16S;                              /* Signed   16 bit quantity                             */
typedef  unsigned  long    INT32U;                              /* Unsigned 32 bit quantity                             */
typedef    signed  long    INT32S;                              /* Signed   32 bit quantity                             */
typedef            float   FP32;                                /* Single precision floating point                      */
typedef            double  FP64;                                /* Double precision floating point                      */

typedef  unsigned  long    OS_STK;                              /* Define size of CPU stack entry                       */
typedef  unsigned  long    OS_CPU_SR;                           /* Define size of CPU status register                   */


/*
*********************************************************************************************************
*                                           DEFINES
*********************************************************************************************************
*/

#define  OS_STK_RSVD_SIZE                   10                  /* EABI Buffer above the stack                          */

#define  OS_STK_GROWTH                      1                   /* Stack grows from HIGH to LOW memory on PPC           */


/*
*********************************************************************************************************
*                                           FLOATING POINT
*
* Note: also enable or disable "OS_SAVE_CONTEXT_WITH_FPRS .equ 1" in os_cpu_a.h
*********************************************************************************************************
*/

#define  OS_SAVE_CONTEXT_WITH_FPRS          1                   /* Enable Floating Point Capability                     */


/*
*********************************************************************************************************
*                                              OS Task Switch
*********************************************************************************************************
*/

#if   defined __GNUC__
#define  OS_TASK_SW()       asm __volatile__ (" sc ");
#else
#define  OS_TASK_SW()       asm (" sc ");
#endif


/*
*********************************************************************************************************
*                                               MPC56xx
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

#define  OS_CRITICAL_METHOD    3u

#define  OS_ENTER_CRITICAL()   do { cpu_sr = OS_CPU_SR_Save(); } while (0)
#define  OS_EXIT_CRITICAL()    do { OS_CPU_SR_Restore(cpu_sr); } while (0)


/*
*********************************************************************************************************
*                                         Function Prototypes
*********************************************************************************************************
*/

OS_CPU_SR  OS_CPU_SR_Rd     (void);

OS_CPU_SR  OS_CPU_SR_Save   (void);
void       OS_CPU_SR_Restore(OS_CPU_SR  cpu_sr);

void       OSCtxSw          (void);
void       OSIntCtxSw       (void);
void       OSStartHighRdy   (void);

void       OSTickISR        (void);
void       OSExtIntISR      (void);


#endif  /* __OS_CPU_H__ */
