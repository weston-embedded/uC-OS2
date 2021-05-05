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
*                                           Renesas R32C Port
*
* Filename  : os_cpu.h
* Version   : V2.93.01
*********************************************************************************************************
* For       : Renesas R32C
* Toolchain : IAR IDE
*********************************************************************************************************
*/

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
*********************************************************************************************************
*                                             DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                                   /* Unsigned  8 bit quantity            */
typedef signed   char  INT8S;                                   /* Signed    8 bit quantity            */
typedef unsigned short INT16U;                                  /* Unsigned 16 bit quantity            */
typedef signed   short INT16S;                                  /* Signed   16 bit quantity            */
typedef unsigned int   INT32U;                                  /* Unsigned 32 bit quantity            */
typedef signed   int   INT32S;                                  /* Signed   32 bit quantity            */
typedef float          FP32;                                    /* Single precision floating point     */
typedef double         FP64;                                    /* Double precision floating point     */

typedef unsigned int   OS_STK;                                  /* Each stack entry is 32-bit wide     */
typedef unsigned int   OS_CPU_SR;                               /* Type of CPU status register         */

/*
*********************************************************************************************************
*                                        RENESAS M16C FAMILY
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

#define  OS_CRITICAL_METHOD    3

#if      OS_CRITICAL_METHOD == 1
#define  OS_ENTER_CRITICAL()  asm("FCLR I")                     /* Disable interrupts                  */
#define  OS_EXIT_CRITICAL()   asm("FSET I")                     /* Enable  interrupts                  */
#endif


#if      OS_CRITICAL_METHOD == 2
#define  OS_ENTER_CRITICAL()  asm("PUSHC FLG"); asm("FCLR I")   /* Disable interrupts                  */
#define  OS_EXIT_CRITICAL()   asm("POPC FLG")                   /* Enable  interrupts                  */
#endif

#if      OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()  {cpu_sr = OS_CPU_SR_Save();}      /* Disable interrupts                  */
#define  OS_EXIT_CRITICAL()   {OS_CPU_SR_Restore(cpu_sr);} 	    /* Enable  interrupts                  */
#endif

/*
*********************************************************************************************************
*                                  RENESAS M16C FAMILY MISCELLANEOUS
*********************************************************************************************************
*/

#define  OS_STK_GROWTH        1                                 /* Stack grows from HIGH to LOW memory */
#define  OS_TASK_SW()         asm("INT #0")                     /* Mapped to the software interrupt 0  */

/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

#if OS_CRITICAL_METHOD == 3u                      /* See OS_CPU_A.S53                                   */
OS_CPU_SR  OS_CPU_SR_Save        (void);
void       OS_CPU_SR_Restore     (OS_CPU_SR   cpu_sr);
#endif

void     OSCtxSw        (void);
void     OSIntCtxSw     (void);
void     OSStartHighRdy (void);
void     OSTickISR      (void);
