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
*                                       ATmega256  Specific code
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
**********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
**********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                            */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                            */
typedef unsigned int   INT16U;                   /* Unsigned 16 bit quantity                            */
typedef signed   int   INT16S;                   /* Signed   16 bit quantity                            */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                            */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                            */
typedef float          FP32;                     /* Single precision floating point                     */

typedef unsigned char  OS_STK;                   /* Each stack entry is 8-bit wide                      */
typedef unsigned char  OS_CPU_SR;                /* Define size of CPU status register (PSW = 8 bits)   */

/*
*********************************************************************************************************
*                                             Atmel AVR
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts. In other words, if
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section. The IAR compiler does not support inline assembly so I'm
*             using the _OPC() intrinsic function. Here are the instructions:
*
*             OS_ENTER_CRITICAL:
*                 ST      -Y,R16
*                 IN      R16,SREG
*                 CLI
*                 PUSH    R16
*                 LD      R16,Y+
*
*             OS_EXIT_CRITICAL:
*                 ST      -Y,R16
*                 POP     R16
*                 OUT     SREG,R16
*                 LD      R16,Y+
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
#define  OS_ENTER_CRITICAL()    _CLI()                    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()     _SEI()                    /* Enable  interrupts                        */
#endif

#if      OS_CRITICAL_METHOD == 2
#define  OS_ENTER_CRITICAL()    _OPC(0x930A);_OPC(0xB70F);_CLI();_OPC(0x930F);_OPC(0x9109)
#define  OS_EXIT_CRITICAL()     _OPC(0x930A);_OPC(0x910F),_OPC(0xBF0F);_OPC(0x9109)
#endif

#if      OS_CRITICAL_METHOD == 3
#define  OS_ENTER_CRITICAL()  (cpu_sr = OS_CPU_SR_Save())    /* Disable interrupts                      */
#define  OS_EXIT_CRITICAL()   (OS_CPU_SR_Restore(cpu_sr))    /* Enable  interrupts                      */
#endif

/*
**********************************************************************************************************
*                                          AVR Miscellaneous
**********************************************************************************************************
*/

#define  OS_STK_GROWTH       1                      /* Stack grows from HIGH to LOW memory on AVR       */

#define  OS_TASK_SW()       OSCtxSw()

/*
**********************************************************************************************************
*                                          GLOBAL VARIABLES
**********************************************************************************************************
*/

OS_CPU_EXT  INT16U  OSTaskStkSize;
OS_CPU_EXT  INT16U  OSTaskStkSizeHard;

/*
**********************************************************************************************************
*                                         Function Prototypes
**********************************************************************************************************
*/

#if OS_CRITICAL_METHOD == 3
OS_CPU_SR  OS_CPU_SR_Save(void);
void       OS_CPU_SR_Restore(OS_CPU_SR  cpu_sr);
#endif

void       OSStartHighRdy(void);
void       OSCtxSw(void);
void       OSIntCtxSw(void);
