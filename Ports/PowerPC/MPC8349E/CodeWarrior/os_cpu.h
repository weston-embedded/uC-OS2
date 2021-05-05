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
*                                   Freescale MPC8349E Specific code
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

#include  <cpu.h>

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

typedef  CPU_BOOLEAN    BOOLEAN;
typedef  CPU_INT08U     INT8U;                                          /* Unsigned  8 bit quantity                                 */
typedef  CPU_INT08S     INT8S;                                          /* Signed    8 bit quantity                                 */
typedef  CPU_INT16U     INT16U;                                         /* Unsigned 16 bit quantity                                 */
typedef  CPU_INT16S     INT16S;                                         /* Signed   16 bit quantity                                 */
typedef  CPU_INT32U     INT32U;                                         /* Unsigned 32 bit quantity                                 */
typedef  CPU_INT32S     INT32S;                                         /* Signed   32 bit quantity                                 */
typedef  CPU_FP32       FP32;                                           /* Single precision floating point                          */
typedef  CPU_FP64       FP64;                                           /* Double precision floating point                          */

typedef  CPU_STK        OS_STK;                                         /* Define size of CPU stack entry                           */
typedef  CPU_SR         OS_CPU_SR;                                      /* Define size of CPU status register                       */

/*
*********************************************************************************************************
*                                           DEFINES
*********************************************************************************************************
*/

#define  OS_STK_RSVD_SIZE     8                                         /* EBI Buffer above the stack                 CEDRIC : A VOIR              */

#define  OS_STK_GROWTH        1                                         /* Stack grows from HIGH to LOW memory on PPC               */


/*
*********************************************************************************************************
*                                           FLOATING POINT
*
* Note: also enable or disable "OS_SAVE_CONTEXT_WITH_FPRS .equ 1" in os_cpu_a.inc
*********************************************************************************************************
*/

#define FPSCR_INIT          0x4L                                        /* Init Value of the FPSCR Register (NI-Bit is set)         */

#define  OS_SAVE_CONTEXT_WITH_FPRS

/*
*********************************************************************************************************
*                                           STACK FRAME
*********************************************************************************************************
*/
typedef struct stk_tag {
  INT32U    R01;
  INT32U    BLK;
  INT32U    R00;
  INT32U    R02;
  INT32U    R03;
  INT32U    R04;
  INT32U    R05;
  INT32U    R06;
  INT32U    R07;
  INT32U    R08;
  INT32U    R09;
  INT32U    R10;
  INT32U    R11;
  INT32U    R12;
  INT32U    R13;
  INT32U    R14;
  INT32U    R15;
  INT32U    R16;
  INT32U    R17;
  INT32U    R18;
  INT32U    R19;
  INT32U    R20;
  INT32U    R21;
  INT32U    R22;
  INT32U    R23;
  INT32U    R24;
  INT32U    R25;
  INT32U    R26;
  INT32U    R27;
  INT32U    R28;
  INT32U    R29;
  INT32U    R30;
  INT32U    R31;
#ifdef OS_SAVE_CONTEXT_WITH_FPRS
  INT32U    _space;                      /* non packed structure */
  long long F00;
  long long F01;
  long long F02;
  long long F03;
  long long F04;
  long long F05;
  long long F06;
  long long F07;
  long long F08;
  long long F09;
  long long F10;
  long long F11;
  long long F12;
  long long F13;
  long long F14;
  long long F15;
  long long F16;
  long long F17;
  long long F18;
  long long F19;
  long long F20;
  long long F21;
  long long F22;
  long long F23;
  long long F24;
  long long F25;
  long long F26;
  long long F27;
  long long F28;
  long long F29;
  long long F30;
  long long F31;
  INT32U    FPSCR;
#endif
  INT32U    CSRR0_;
  INT32U    CSRR1_;
  INT32U    CR;
  INT32U    SRR0_;
  INT32U    SRR1_;
  INT32U    CTR_;
  INT32U    XER_;
  INT32U    LR_;
} STK;


/*
*********************************************************************************************************
*                                              OS Task Swicth
*********************************************************************************************************
*/

#if defined __MWERKS__
#define  OS_TASK_SW()       asm (" sc ");
#else
#error Unknown Compiler Assembler Syntax
#endif


/*
*********************************************************************************************************
*                                           Critical Method MACROS
*********************************************************************************************************
*/

#define  OS_CRITICAL_METHOD     CPU_CFG_CRITICAL_METHOD

#define  OS_ENTER_CRITICAL()   {CPU_CRITICAL_ENTER();}
#define  OS_EXIT_CRITICAL()    {CPU_CRITICAL_EXIT();}


/*
*********************************************************************************************************
*                                         Function Prototypes
*********************************************************************************************************
*/

void  OSCtxSw(void);
void  OSIntCtxSw(void);
void  OSStartHighRdy(void);

void  OSTickISR(void);
void  OSExtIntISR(void);


#endif  /* __OS_CPU_H__ */
