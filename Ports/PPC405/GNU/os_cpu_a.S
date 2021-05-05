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
*                                             PowerPC405
*                                          GNU  C/C++ Compiler
*
* Filename : os_cpu_a.S
* Version  : V2.93.01
*********************************************************************************************************
*/

#include "xreg405.h"

	.file "os_cpu_a.S"

	.section ".vectors","ax"

#ifdef __DCC__
#define // ;
#define MACRO0(name)name:	.macro
#define MACRO1(name, param1)name:	.macro	param1
#define MACRO2(name, param1, param2)name:	.macro	param1, param2
#define CONCAT(left, right) left##&&##right
#define CONCAT3(left, parammiddle, right) left##&&##parammiddle##right
#define PARAM(param) param
#else
#include <ppc-asm.h>
#define r1 1
#define r2 2
#define MACRO0(name) .macro	name
#define MACRO1(name, param1) .macro	name	param1
#define MACRO2(name, param1, param2) .macro	name	param1 param2
#define CONCAT(left,right) left##right
#define PARAM(param) param
#endif

/*
*********************************************************************************************************
*                                          PUBLIC FUNCTIONS
*********************************************************************************************************
*/

    .global  OSStartHighRdy
    .global  OSIntCtxSw
    .global  OS_CPU_SR_Save
    .global  OS_CPU_SR_Restore

/*
*********************************************************************************************************
*                                         EXTERNAL FUNCTIONS
*********************************************************************************************************
*/

    .extern  OSIntEnter
    .extern  OSIntExit
    .extern  BSP_IntHandler
    .extern  OSTaskSwHook

/*
*********************************************************************************************************
*                                         EXTERNAL VARIABLES
*********************************************************************************************************
*/

    .extern  OSRunning
    .extern  OSIntNesting
    .extern  OSTCBCur
    .extern  OSTCBHighRdy
    .extern  OSPrioCur
    .extern  OSPrioHighRdy

/*
*********************************************************************************************************
*                               CONSTANTS USED TO ACCESS TASK CONTEXT STACK
*********************************************************************************************************
*/

    .set    STK_OFFSET_BC,     0
    .set    STK_OFFSET_NEW_LR, STK_OFFSET_BC     + 4
    .set    STK_OFFSET_MSR,    STK_OFFSET_NEW_LR + 4
    .set    STK_OFFSET_PC,     STK_OFFSET_MSR    + 4
    .set    STK_OFFSET_LR,     STK_OFFSET_PC     + 4
    .set    STK_OFFSET_CTR,    STK_OFFSET_LR     + 4
    .set    STK_OFFSET_XER,    STK_OFFSET_CTR    + 4
    .set    STK_OFFSET_CR,     STK_OFFSET_XER    + 4
    .set    STK_OFFSET_USPRG0, STK_OFFSET_CR     + 4
    .set    STK_OFFSET_R0,     STK_OFFSET_USPRG0 + 4
    .set    STK_OFFSET_R2,     STK_OFFSET_R0     + 4
    .set    STK_OFFSET_R3R31,  STK_OFFSET_R2     + 4
    .set    STK_CTX_SIZE,      STK_OFFSET_R3R31  + ( ( 31 - 3 ) + 1 ) * 4

/*
*********************************************************************************************************
*                                       OS_CPU_ISR_CRITICAL
*
* Description: This macro is placed at offset 0x0100 in the interrupt-handler table, so it is executed
*              whenever the external critical interrupt signal is asserted.  This code saves the
*              current task's context before calling BSP_IntHandler(), which provides further,
*              application-specific processing of the interrupt.
*
*              The interrupted task's context is saved onto its stack as follows:
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00     Back chain                    (LOW Memory)
*                                        + 0x04     New Link Register
*                                        + 0x08     Machine-State Register
*                                        + 0x0C     Return Address
*                                        + 0x10     Link Register
*                                        + 0x14     Count Register
*                                        + 0x18     Fixed-Point Exception Register
*                                        + 0x1C     Condition Register
*                                        + 0x20     User SPR General-Purpose Register 0
*                                        + 0x24     GPR0
*                                        + 0x28     GPR2
*                                        + 0x2C     GPR3
*                                        + 0x30     GPR4
*                                                |
*                                                |
*                                               \./
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OS_CPU_ISR_CRITICAL)
                                                        /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu    XREG_GPR1,-STK_CTX_SIZE(XREG_GPR1)          /* Adjust the stack pointer                    */

    stw     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Save General-Purpose Registers 0 and 2      */
    stw     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    stmw    XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Save General-Purpose Registers 3-31         */

    mflr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Save the Link Register                      */

    mfctr   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Save the Count Register                     */

    mfxer   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Save the Fixed-Point Exception Register     */

    mfcr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Save the Condition Register                 */

    mfspr   XREG_GPR0, 0x100
    stw     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)           /* Save User SPR General-Purpose Register 0    */

    mfsrr3  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Save the Machine-State Register             */

    mfsrr2  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Save the return address                     */

    lis     XREG_GPR4, OSIntNesting@h                   /* See if OSIntNesting == 0                    */
    ori     XREG_GPR4, XREG_GPR4, OSIntNesting@l
    lwz     XREG_GPR5, 0(XREG_GPR4)
    addic.  XREG_GPR5, XREG_GPR5, 0
    bc      0x04,      0x02, OS_CPU_ISR_CRITICAL_1

    lis     XREG_GPR6, OSTCBCur@h                       /* Save the current task's stack pointer       */
    ori     XREG_GPR6, XREG_GPR6, OSTCBCur@l
    lwz     XREG_GPR7, 0(XREG_GPR6)
    stw     XREG_GPR1, 0(XREG_GPR7)

OS_CPU_ISR_CRITICAL_1:

    addi    XREG_GPR5, XREG_GPR5, 1                     /* Increment OSIntNesting                      */
    stb     XREG_GPR5, 0(XREG_GPR4)

    lis     XREG_GPR0, BSP_IntHandler@h                 /* Call the interrupt controller's handler     */
    ori     XREG_GPR0, XREG_GPR0, BSP_IntHandler@l
    mtlr    XREG_GPR0
    blrl

    lis     XREG_GPR0, OSIntExit@h                      /* Call OSIntExit()                            */
    ori     XREG_GPR0, XREG_GPR0, OSIntExit@l
    mtlr    XREG_GPR0
    blrl

                                                        /* *** RESTORE INTERRUPTED TASK'S CONTEXT **** */

    lwz     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Restore the Machine-State Register to SRR3  */
    mtsrr3  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Restore the Machine-State Register to SRR2  */
    mtsrr2  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100,     XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Restore the Condition Register              */
    mtcr    XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Restore the Fixed-Point Exception Register  */
    mtxer   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Restore the Count Register                  */
    mtctr   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Restore the Link Register                   */
    mtlr    XREG_GPR0

    lmw     XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Restore General-Purpose Registers 3-31      */

    lwz     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Restore General-Purpose Registers 0 and 2   */
    lwz     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    addi    XREG_GPR1, XREG_GPR1,STK_CTX_SIZE           /* Adjust the stack pointer                    */

    rfci                                                /* Return to the interrupted task              */

    .endm

/*
*********************************************************************************************************
*                                      OS_CPU_ISR_NON_CRITICAL
*
* Description: This macro is placed at offset 0x0500 in the interrupt-handler table, so it is executed
*              whenever the external non-critical interrupt signal is asserted.  This code saves the
*              current task's context before calling BSP_IntHandler(), which provides further,
*              application-specific processing of the interrupt.
*
*              The interrupted task's context is saved onto its stack as follows:
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00     Back chain                    (LOW Memory)
*                                        + 0x04     New Link Register
*                                        + 0x08     Machine-State Register
*                                        + 0x0C     Return Address
*                                        + 0x10     Link Register
*                                        + 0x14     Count Register
*                                        + 0x18     Fixed-Point Exception Register
*                                        + 0x1C     Condition Register
*                                        + 0x20     User SPR General-Purpose Register 0
*                                        + 0x24     GPR0
*                                        + 0x28     GPR2
*                                        + 0x2C     GPR3
*                                        + 0x30     GPR4
*                                                |
*                                                |
*                                               \./
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OS_CPU_ISR_NON_CRITICAL)
                                                        /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu    XREG_GPR1,-STK_CTX_SIZE(XREG_GPR1)          /* Adjust the stack pointer                    */

    stw     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Save General-Purpose Registers 0 and 2      */
    stw     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    stmw    XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Save General-Purpose Registers 3-31         */

    mflr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Save the Link Register                      */

    mfctr   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Save the Count Register                     */

    mfxer   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Save the Fixed-Point Exception Register     */

    mfcr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Save the Condition Register                 */

    mfspr   XREG_GPR0, 0x100
    stw     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)           /* Save User SPR General-Purpose Register 0    */

    mfsrr1  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Save the Machine-State Register             */

    mfsrr0  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Save the return address                     */

    lis     XREG_GPR4, OSIntNesting@h                   /* See if OSIntNesting == 0                    */
    ori     XREG_GPR4, XREG_GPR4, OSIntNesting@l
    lwz     XREG_GPR5, 0(XREG_GPR4)
    addic.  XREG_GPR5, XREG_GPR5, 0
    bc      0x04,      0x02, OS_CPU_ISR_NON_CRITICAL_1

    lis     XREG_GPR6, OSTCBCur@h                       /* Save the current task's stack pointer       */
    ori     XREG_GPR6, XREG_GPR6, OSTCBCur@l
    lwz     XREG_GPR7, 0(XREG_GPR6)
    stw     XREG_GPR1, 0(XREG_GPR7)

OS_CPU_ISR_NON_CRITICAL_1:

    addi    XREG_GPR5, XREG_GPR5, 1                     /* Increment OSIntNesting                      */
    stb     XREG_GPR5, 0(XREG_GPR4)

    lis     XREG_GPR0, BSP_IntHandler@h                 /* Call the interrupt controller's handler     */
    ori     XREG_GPR0, XREG_GPR0, BSP_IntHandler@l
    mtlr    XREG_GPR0
    blrl

    lis     XREG_GPR0, OSIntExit@h                      /* Call OSIntExit()                            */
    ori     XREG_GPR0, XREG_GPR0, OSIntExit@l
    mtlr    XREG_GPR0
    blrl

                                                        /* *** RESTORE INTERRUPTED TASK'S CONTEXT **** */

    lwz     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Restore the return address to SRR0          */
    mtsrr0  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100,     XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Restore the Condition Register              */
    mtcr    XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Restore the Fixed-Point Exception Register  */
    mtxer   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Restore the Count Register                  */
    mtctr   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Restore the Link Register                   */
    mtlr    XREG_GPR0

    lmw     XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Restore General-Purpose Registers 3-31      */

    lwz     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Restore General-Purpose Registers 0 and 2   */
    lwz     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    addi    XREG_GPR1, XREG_GPR1,STK_CTX_SIZE           /* Adjust the stack pointer                    */

    rfi                                                 /* Return to the interrupted task              */

    .endm

/*
*********************************************************************************************************
*                                              OSCtxSw
*
* Description: Performs a Context switch from a task.  This function is ALWAYS called with interrupts
*              DISABLED.
*
*              OSCtxSw() implements the following pseudo-code:
*
*                  Save CPU registers;
*                  OSTCBCur->OSTCBStkPtr = SP;
*                  OSTaskSwHook();
*                  OSPrioCur = OSPrioHighRdy;
*                  OSTCBCur  = OSTCBHighRdy;
*                  SP        = OSTCBHighRdy->OSTCBStkPtr;
*                  Restore CPU registers
*                  Execute the return-from-interrupt instruction;
*
*
*              The stack frame of the task to suspend will look as follows when OSCtxSw() is done:
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00     Back chain                    (LOW Memory)
*                                        + 0x04     New Link Register
*                                        + 0x08     Machine-State Register
*                                        + 0x0C     Return Address
*                                        + 0x10     Link Register
*                                        + 0x14     Count Register
*                                        + 0x18     Fixed-Point Exception Register
*                                        + 0x1C     Condition Register
*                                        + 0x20     User SPR General-Purpose Register 0
*                                        + 0x24     GPR0
*                                        + 0x28     GPR2
*                                        + 0x2C     GPR3
*                                        + 0x30     GPR4
*                                                |
*                                                |
*                                               \./
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*              The stack frame of the task to resume looks as follows:
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00     Back chain                    (LOW Memory)
*                                        + 0x04     New Link Register
*                                        + 0x08     Machine-State Register
*                                        + 0x0C     Return Address
*                                        + 0x10     Link Register
*                                        + 0x14     Count Register
*                                        + 0x18     Fixed-Point Exception Register
*                                        + 0x1C     Condition Register
*                                        + 0x20     User SPR General-Purpose Register 0
*                                        + 0x24     GPR0
*                                        + 0x28     GPR2
*                                        + 0x2C     GPR3
*                                        + 0x30     GPR4
*                                                |
*                                                |
*                                               \./
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OSCtxSw)
                                                        /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu    XREG_GPR1,-STK_CTX_SIZE(XREG_GPR1)          /* Adjust the stack pointer                    */

    stw     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Save General-Purpose Registers 0 and 2      */
    stw     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    stmw    XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Save General-Purpose Registers 3-31         */

    mflr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Save the Link Register                      */

    mfctr   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Save the Count Register                     */

    mfxer   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Save the Fixed-Point Exception Register     */

    mfcr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Save the Condition Register                 */

    mfspr   XREG_GPR0, 0x100
    stw     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)           /* Save User SPR General-Purpose Register 0    */

    mfsrr1  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Save the Machine-State Register             */

    mfsrr0  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Save the return address                     */

    lis     XREG_GPR4, OSTCBCur@h                       /* Save the current task's stack pointer       */
    ori     XREG_GPR4, XREG_GPR4, OSTCBCur@l
    lwz     XREG_GPR5, 0(XREG_GPR4)
    stw     XREG_GPR1, 0(XREG_GPR5)

    lis     XREG_GPR0, OSTaskSwHook@h                   /* Call OSTaskSwHook()                         */
    ori     XREG_GPR0, XREG_GPR0, OSTaskSwHook@l
    mtlr    XREG_GPR0
    blrl

    lis     XREG_GPR4, OSPrioHighRdy@h                  /* Update the current priority                 */
    ori     XREG_GPR4, XREG_GPR4, OSPrioHighRdy@l
    lbz     XREG_GPR5, 0(XREG_GPR4)
    lis     XREG_GPR6, OSPrioCur@h
    ori     XREG_GPR6, XREG_GPR6, OSPrioCur@l
    stb     XREG_GPR5, 0(XREG_GPR6)

    lis     XREG_GPR4, OSTCBHighRdy@h                   /* Update the current TCB                      */
    ori     XREG_GPR4, XREG_GPR4, OSTCBHighRdy@l
    lwz     XREG_GPR5, 0(XREG_GPR4)
    lis     XREG_GPR6, OSTCBCur@h
    ori     XREG_GPR6, XREG_GPR6, OSTCBCur@l
    stw     XREG_GPR5, 0(XREG_GPR6)

    lwz     XREG_GPR1, 0(XREG_GPR5)                     /* Load the new task's stack pointer           */

                                                        /* ******** RESTORE NEW TASK'S CONTEXT ******* */

    lwz     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Restore the return address to SRR0          */
    mtsrr0  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100,     XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Restore the Condition Register              */
    mtcr    XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Restore the Fixed-Point Exception Register  */
    mtxer   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Restore the Count Register                  */
    mtctr   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Restore the Link Register                   */
    mtlr    XREG_GPR0

    lmw     XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Restore General-Purpose Registers 3-31      */

    lwz     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Restore General-Purpose Registers 0 and 2   */
    lwz     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    addi    XREG_GPR1, XREG_GPR1,STK_CTX_SIZE           /* Adjust the stack pointer                    */

    rfi                                                 /* Return to the interrupted task              */

    .endm

/*
*********************************************************************************************************
*                                            OSTickISR
*
* Description: This macro is used to process uC/OS-II tick interrupts, which are produced by the
*              programmable interval timer.  This code saves the current task's context before calling
*              OSTimeTick(), uC/OS-II's CPU-independent routine for processing these interrupts.
*
*              The interrupted task's context is saved onto its stack as follows:
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00     Back chain                    (LOW Memory)
*                                        + 0x04     New Link Register
*                                        + 0x08     Machine-State Register
*                                        + 0x0C     Return Address
*                                        + 0x10     Link Register
*                                        + 0x14     Count Register
*                                        + 0x18     Fixed-Point Exception Register
*                                        + 0x1C     Condition Register
*                                        + 0x20     User SPR General-Purpose Register
*                                        + 0x24     GPR0
*                                        + 0x28     GPR2
*                                        + 0x2C     GPR3
*                                        + 0x30     GPR4
*                                                |
*                                                |
*                                               \./
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OSTickISR)
                                                        /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu    XREG_GPR1,-STK_CTX_SIZE(XREG_GPR1)          /* Adjust the stack pointer                    */

    stw     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Save General-Purpose Registers 0 and 2      */
    stw     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    stmw    XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Save General-Purpose Registers 3-31         */

    mflr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Save the Link Register                      */

    mfctr   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Save the Count Register                     */

    mfxer   XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Save the Fixed-Point Exception Register     */

    mfcr    XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Save the Condition Register                 */

    mfspr   XREG_GPR0, 0x100
    stw     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)           /* Save User SPR General-Purpose Register 0    */

    mfsrr1  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Save the Machine-State Register             */

    mfsrr0  XREG_GPR0
    stw     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Save the return address                     */

    lis     XREG_GPR4, OSIntNesting@h                   /* See if OSIntNesting == 0                    */
    ori     XREG_GPR4, XREG_GPR4, OSIntNesting@l
    lbz     XREG_GPR5, 0(XREG_GPR4)
    addic.  XREG_GPR5, XREG_GPR5, 0
    bc      0x04,      0x02, OSTickISR_1

    lis     XREG_GPR6, OSTCBCur@h                       /* Save the current task's stack pointer       */
    ori     XREG_GPR6, XREG_GPR6, OSTCBCur@l
    lwz     XREG_GPR7, 0(XREG_GPR6)
    stw     XREG_GPR1, 0(XREG_GPR7)

OSTickISR_1:

    addi    XREG_GPR5, XREG_GPR5, 1                     /* Increment OSIntNesting                      */
    stb     XREG_GPR5, 0(XREG_GPR4)

    lis     XREG_GPR0, 0x0800                           /* Clear the PIT interrupt                     */
    mttsr   XREG_GPR0

    lis     XREG_GPR0, OSTimeTick@h                     /* Call OSTimeTick()                           */
    ori     XREG_GPR0, XREG_GPR0, OSTimeTick@l
    mtlr    XREG_GPR0
    blrl

    lis     XREG_GPR0, OSIntExit@h                      /* Call OSIntExit()                            */
    ori     XREG_GPR0, XREG_GPR0, OSIntExit@l
    mtlr    XREG_GPR0
    blrl

                                                        /* **** RESTORE INTERRUPTED TASK'S CONTEXT *** */

    lwz     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Restore the return address to SRR0          */
    mtsrr0  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100,     XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Restore the Condition Register              */
    mtcr    XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Restore the Fixed-Point Exception Register  */
    mtxer   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Restore the Count Register                  */
    mtctr   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Restore the Link Register                   */
    mtlr    XREG_GPR0

    lmw     XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Restore General-Purpose Registers 3-31      */

    lwz     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Restore General-Purpose Registers 0 and 2   */
    lwz     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    addi    XREG_GPR1, XREG_GPR1,STK_CTX_SIZE           /* Adjust the stack pointer                    */

    rfi                                                 /* Return to the interrupted task              */

    .endm

/*
*********************************************************************************************************
*                                      OS_NULL_NON_CRITICAL
*
* Description : This exception handler simply returns to the interrupted code.
*********************************************************************************************************
*/

    MACRO0(OS_NULL_NON_CRITICAL)

    rfi

    .endm

/*
*********************************************************************************************************
*                                        OS_NULL_CRITICAL
*
* Description : This exception handler simply returns to the interrupted code.
*********************************************************************************************************
*/

    MACRO0(OS_NULL_CRITICAL)

    rfci

    .endm

/*
*********************************************************************************************************
*                                 PowerPC405 Interrupt-Handler Table
*
* This is the interrupt-handler table used by the PowerPC to appropriately invoke handlers for various
* types of interrupts.  The interrupts are the results of exceptions, each of which corresponds to an
* offset into this table.  When the interrupt associated with a given expection occurs, the PowerPC adds
* the offset for that exception to this table's base address, which is stored in the Exception-Vector
* Prefix Register (EVPR) by OSStartHighRdy().  The code at the resulting address should represent the
* handler for the exception.
*
* The table entries for the critical input exception (offset 0x0100) and the external input exception
* (offset 0x0500) span 256 (0x100) bytes each, which is enough room to accomodate uC/OS-II's handlers
* for those interrupts, OS_CPU_ISR_CRITICAL() and OS_CPU_ISR_NON_CRITICAL(), respectively.  The entry
* for the programmable-interval timer exception, however, only covers 16 bytes, so OSTickISR(),
* uC/OS-II's handler for timer exceptions, must be placed at another location.  Since the entries at
* offsets 0x1300 and 0x1400 don't correspond to any exceptions, OSTickISR() is placed there, and a
* branch to that location is inserted at offset 0x1000, the actual offset corresponding to the
* programmable-interval timer exception.
*
* All table entries that are not currently used by uC/OS-II contain null routines that simply return
* from the interrupt without performing any actions.  These routines can be replaced if an application
* requires support for additional types of exceptions.
*********************************************************************************************************
*/

_vectorbase:

    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x0100                /* Offset 0x0100 -> Critical Input Exception              */
    OS_CPU_ISR_CRITICAL


    .org _vectorbase + 0x0200                /* Offset 0x0200 -> Machine Check Exception               */
    OS_NULL_CRITICAL


    .org _vectorbase + 0x0300                /* Offset 0x0300 -> Data Storage Exception                */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x0400                /* Offset 0x0400 -> Instruction Storage Exception         */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x0500                /* Offset 0x0500 -> External Input Exception              */
    OS_CPU_ISR_NON_CRITICAL


    .org _vectorbase + 0x0600                /* Offset 0x0600 -> Alignment Exception                   */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x0700                /* Offset 0x0700 -> Program Exception                     */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x0800                /* Offset 0x0800 -> FPU Unavailable Exception             */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x0C00                /* Offset 0x0C00 -> System Call Exception                 */
    OSCtxSw


    .org _vectorbase + 0x0F20                /* Offset 0x0F20 -> APU Unavailable Exception             */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x1000                /* Offset 0x1000 -> Programmable-Interval Timer Exception */
    b PITISR


    .org _vectorbase + 0x1010                /* Offset 0x1010 -> Fixed-Interval Timer Exception        */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x1020                /* Offset 0x1020 -> Watchdog Timer Exception              */
    OS_NULL_CRITICAL


    .org _vectorbase + 0x1100                /* Offset 0x1100 -> Data TLB Miss Exception               */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x1200                /* Offset 0x1200 -> Instruction TLB Miss Exception        */
    OS_NULL_NON_CRITICAL


    .org _vectorbase + 0x1300                /* Offset 0x1300 -> Excess code from other entries        */
PITISR:
    OSTickISR

    .org _vectorbase + 0x2000                /* Offset 0x2000 -> Debug Exceptions                      */
    OS_NULL_CRITICAL



    .section ".text"

/*
*********************************************************************************************************
*                                            OSIntCtxSw()
*
* Description: Performs the Context Switch from an ISR.
*
*              OSIntCtxSw() implements the following pseudo-code:
*
*                  OSTaskSwHook();
*                  OSPrioCur = OSPrioHighRdy;
*                  OSTCBCur  = OSTCBHighRdy;
*                  SP        = OSTCBHighRdy->OSTCBStkPtr;
*                  if (Context saved by Critical ISR) {
*                      Restore MSR and PC to SRR3 and SRR2, respectively;
*                      Restore all other registers;
*                      Execute the return-from-critical-interrupt instruction;
*                  } else {
*                      Restore MSR and PC to SRR1 and SRR0, respectively;
*                      Restore all other registers;
*                      Execute the return-from-interrupt instruction;
*                  }
*
*              Upon entry, the registers of the task being suspended have already been saved onto that
*              task's stack and the SP for the task has been saved in its OS_TCB by the ISR.
*
*              The stack frame of the task to resume is assumed to look as follows:
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00     Back chain                    (LOW Memory)
*                                        + 0x04     New Link Register
*                                        + 0x08     Machine-State Register
*                                        + 0x0C     Return Address
*                                        + 0x10     Link Register
*                                        + 0x14     Count Register
*                                        + 0x18     Fixed-Point Exception Register
*                                        + 0x1C     Condition Register
*                                        + 0x20     Interrupt type
*                                        + 0x24     GPR0
*                                        + 0x28     GPR2
*                                        + 0x2C     GPR3
*                                        + 0x30     GPR4
*                                                |
*                                                |
*                                               \./
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
* Note(s)    : 1) If the task frame was saved by a critical ISR, the stack entry at offset 0x20 will be
*                 set to 1.
*                 If the task frame was saved by a non-critical ISR, the stack entry at offset 0x20 will
*                 be set to 0.
*********************************************************************************************************
*/

OSIntCtxSw:

    lis     XREG_GPR0, OSTaskSwHook@h                   /* Call OSTaskSwHook()                         */
    ori     XREG_GPR0, XREG_GPR0, OSTaskSwHook@l
    mtlr    XREG_GPR0
    blrl

    lis     XREG_GPR4, OSPrioHighRdy@h                  /* Update the current priority                 */
    ori     XREG_GPR4, XREG_GPR4, OSPrioHighRdy@l
    lbz     XREG_GPR5, 0(XREG_GPR4)
    lis     XREG_GPR6, OSPrioCur@h
    ori     XREG_GPR6, XREG_GPR6, OSPrioCur@l
    stb     XREG_GPR5, 0(XREG_GPR6)

    lis     XREG_GPR4, OSTCBHighRdy@h                   /* Update the current TCB                      */
    ori     XREG_GPR4, XREG_GPR4, OSTCBHighRdy@l
    lwz     XREG_GPR5, 0(XREG_GPR4)
    lis     XREG_GPR6, OSTCBCur@h
    ori     XREG_GPR6, XREG_GPR6, OSTCBCur@l
    stw     XREG_GPR5, 0(XREG_GPR6)

    lwz     XREG_GPR1, 0(XREG_GPR5)                     /* Load the new task's stack pointer           */

                                                        /* ******** RESTORE NEW TASK'S CONTEXT ******* */

    lwz     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Restore the return address to SRR0          */
    mtsrr0  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100,     XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Restore the Condition Register              */
    mtcr    XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Restore the Fixed-Point Exception Register  */
    mtxer   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Restore the Count Register                  */
    mtctr   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Restore the Link Register                   */
    mtlr    XREG_GPR0

    lmw     XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Restore General-Purpose Registers 3-31      */

    lwz     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Restore General-Purpose Registers 0 and 2   */
    lwz     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    addi    XREG_GPR1, XREG_GPR1,STK_CTX_SIZE           /* Adjust the stack pointer                    */

    rfi                                                 /* Return to the interrupted task              */

/*
*********************************************************************************************************
*                                            OSStartHighRdy()
*
* Description: Starts the highest priority task that is available to run.  OSStartHighRdy() MUST:
*
*              a) Call OSTaskSwHook()
*              b) Set OSRunning to TRUE
*              c) Switch to the highest priority task.
*
*              The stack frame of the task to resume is assumed to look as follows:
*
*              OSTCBHighRdy->OSTCBStkPtr + 0x00     Back chain                    (LOW Memory)
*                                        + 0x04     New Link Register
*                                        + 0x08     Machine-State Register
*                                        + 0x0C     Return Address
*                                        + 0x10     Link Register
*                                        + 0x14     Count Register
*                                        + 0x18     Fixed-Point Exception Register
*                                        + 0x1C     Condition Register
*                                        + 0x20     Interrupt type (initialized to 0x00000000)
*                                        + 0x24     GPR0
*                                        + 0x28     GPR2
*                                        + 0x2C     GPR3
*                                        + 0x30     GPR4
*                                                |
*                                                |
*                                               \./
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
* Note(s)    : 1) The MSR entry in the stack is initialized so that interrupts will be enabled when the
*                 task begins to run.  In order to allow the interrupts to be properly serviced,
*                 OSStartHighRdy() initializes the Exception-Vector Prefix Register (EVPR) so that it
*                 contains the address of the interrupt-handler table defined above.
*********************************************************************************************************
*/

OSStartHighRdy:

    lis     XREG_GPR0, OSTaskSwHook@h                   /* Call OSTaskSwHook()                         */
    ori     XREG_GPR0, XREG_GPR0, OSTaskSwHook@l
    mtlr    XREG_GPR0
    blrl

    li      XREG_GPR0, 1                                /* Indicate that the OS is running             */
    lis     XREG_GPR4, OSRunning@h
    ori     XREG_GPR4, XREG_GPR4, OSRunning@l
    stb     XREG_GPR0, 0(XREG_GPR4)

    lis     XREG_GPR4, OSTCBHighRdy@h                   /* Load the first task's stack pointer         */
    ori     XREG_GPR4, XREG_GPR4, OSTCBHighRdy@l
    lwz     XREG_GPR5, 0(XREG_GPR4)
    lwz     XREG_GPR1, 0(XREG_GPR5)

    lis     XREG_GPR0, _vectorbase@h                    /* Initialize the EVPR                         */
    mtevpr  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_MSR(XREG_GPR1)        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_PC(XREG_GPR1)         /* Restore the return address to SRR0          */
    mtsrr0  XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_USPRG0(XREG_GPR1)     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100,     XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CR(XREG_GPR1)         /* Restore the Condition Register              */
    mtcr    XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_XER(XREG_GPR1)        /* Restore the Fixed-Point Exception Register  */
    mtxer   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_CTR(XREG_GPR1)        /* Restore the Count Register                  */
    mtctr   XREG_GPR0

    lwz     XREG_GPR0, STK_OFFSET_LR(XREG_GPR1)         /* Restore the Link Register                   */
    mtlr    XREG_GPR0

    lmw     XREG_GPR3, STK_OFFSET_R3R31(XREG_GPR1)      /* Restore General-Purpose Registers 3-31      */

    lwz     XREG_GPR0, STK_OFFSET_R0(XREG_GPR1)         /* Restore General-Purpose Registers 0 and 2   */
    lwz     XREG_GPR2, STK_OFFSET_R2(XREG_GPR1)

    addi    XREG_GPR1, XREG_GPR1,STK_CTX_SIZE           /* Adjust the stack pointer                    */

    rfi                                                 /* Start the task                              */

/*
*********************************************************************************************************
*                                         DISABLE INTERRUPTS
*                                  OS_CPU_SR  OS_CPU_SR_Save(void);
*
* Description : Sets the MSR, disabling interrupts, and returns the previous MSR contents.  This allows
*               the machine state to be restored at a subsequent time.
*
* Arguments   : None
*
* Returns     : Current MSR contents in GPR3
*
* Note(s)     : 1) The variable in the calling routine that will hold the return value MUST be declared
*                  volatile for proper operation.  There is no guarantee that the proper register will
*                  be scheduled for the subsequent 'OS_CPU_SR_Save()' function call if the variable is
*                  not declared volatile.
*********************************************************************************************************
*/

OS_CPU_SR_Save:

    addis  XREG_GPR4, 0,         0xFFFD
    ori    XREG_GPR4, XREG_GPR4, 0x7FFF
    mfmsr  XREG_GPR3
    and    XREG_GPR4, XREG_GPR4, XREG_GPR3              /* Clear bits 14 and 16, corresponding to...   */
    mtmsr  XREG_GPR4                                    /* ...critical and non-critical interrupts     */
    blr

/*
*********************************************************************************************************
*                                         ENABLE INTERRUPTS
*                                 void OS_CPU_SR_Restore(OS_CPU_SR sr);
*
* Description : Sets the MSR, possibly enabling interrupts, using the value passed in GPR3.
*
* Arguments   : Saved MSR contents in GPR3
*
* Returns     : None
*
* Note(s)     : 1) The argument from the calling routine MUST be declared volatile for proper operation.
*                  There is no guarantee that the proper register will be scheduled for the call to
*                  OS_CPU_SR_Restore() if the variable is not declared 'volatile'.
*********************************************************************************************************
*/

OS_CPU_SR_Restore:

    mtmsr  XREG_GPR3                                    /* Restore the saved MSR                       */
    blr
