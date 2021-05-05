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
*                                              PowerPC440
*                                          GNU  C/C++ Compiler
*
* Filename  : os_cpu_a.S
* Version   : V2.93.01
*********************************************************************************************************
*/

	.file "os_cpu_a.S"

	.section ".vectors","ax"

#define MACRO0(name) .macro	name
#define MACRO1(name, param1) .macro	name	param1
#define MACRO2(name, param1, param2) .macro	name	param1 param2
#define CONCAT(left,right) left##right
#define PARAM(param) param

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
    .extern  BSP_CriticalIntHandler
    .extern  BSP_NonCriticalIntHandler
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

    .set    STK_OFFSET_BC,       0
    .set    STK_OFFSET_NEW_LR,   STK_OFFSET_BC     + 4
    .set    STK_OFFSET_MSR,      STK_OFFSET_NEW_LR + 4
    .set    STK_OFFSET_PC,       STK_OFFSET_MSR    + 4
    .set    STK_OFFSET_LR,       STK_OFFSET_PC     + 4
    .set    STK_OFFSET_CTR,      STK_OFFSET_LR     + 4
    .set    STK_OFFSET_XER,      STK_OFFSET_CTR    + 4
    .set    STK_OFFSET_CR,       STK_OFFSET_XER    + 4
    .set    STK_OFFSET_USPRG0,   STK_OFFSET_CR     + 4
    .set    STK_OFFSET_R0,       STK_OFFSET_USPRG0 + 4
    .set    STK_OFFSET_R2R31,    STK_OFFSET_R0     + 4
    .set    STK_CTX_SIZE,        STK_OFFSET_R2R31  + ( ( 31 - 2 ) + 1 ) * 4
    .set    SIXTEEN_BYTES,       16
    .set    IVOR0_VAL ,          0
    .set    IVOR1_VAL,           IVOR0_VAL   + SIXTEEN_BYTES
    .set    IVOR2_VAL,           IVOR1_VAL   + SIXTEEN_BYTES
    .set    IVOR3_VAL,           IVOR2_VAL   + SIXTEEN_BYTES
    .set    IVOR4_VAL,           IVOR3_VAL   + SIXTEEN_BYTES
    .set    IVOR5_VAL,           IVOR4_VAL   + SIXTEEN_BYTES
    .set    IVOR6_VAL,           IVOR5_VAL   + SIXTEEN_BYTES
    .set    IVOR7_VAL,           IVOR6_VAL   + SIXTEEN_BYTES
    .set    IVOR8_VAL,           IVOR7_VAL   + SIXTEEN_BYTES
    .set    IVOR9_VAL,           IVOR8_VAL   + SIXTEEN_BYTES
    .set    IVOR10_VAL,          IVOR9_VAL   + SIXTEEN_BYTES
    .set    IVOR11_VAL,          IVOR10_VAL  + SIXTEEN_BYTES
    .set    IVOR12_VAL,          IVOR11_VAL  + SIXTEEN_BYTES
    .set    IVOR13_VAL,          IVOR12_VAL  + SIXTEEN_BYTES
    .set    IVOR14_VAL,          IVOR13_VAL  + SIXTEEN_BYTES
    .set    IVOR15_VAL,          IVOR14_VAL  + SIXTEEN_BYTES

/*
*********************************************************************************************************
*                                       OS_CPU_ISR_CRITICAL
*
* Description: This macro is placed at offset 0x0400 in the interrupt-handler table, so it is executed
*              whenever critical interrupts are enabled and the external critical interrupt signal is
*              asserted.  The macro's code saves the current task's context before calling
*              BSP_CriticalIntHandler(), which provides further, application-specific processing of the
*              interrupt.
*
*              Critical interrupts are disabled while this routine executes.  However, critical interupts
*              are not disabled when non-critical interrupts occur.  Therefore, this routine checks the
*              return address saved in CSSR0 to determine if one of the non-critical ISRs residing in the
*              interrupt-handler table was interrupted.  If the return address indeed corresponds to one
*              of these ISRs, OSIntNesting is not manipulated and the scheduler is not invoked.  These
*              precautions are necessary when a non-critical ISR is interrupted because the interrupt
*              may have occurred before that ISR incremented OSIntNesting, which could result in the
*              ensuing critical ISR prematurely invoking the scheduler.  Whether or not a non-critical
*              ISR was interrupted, though, context is saved and the interrupt is serviced by
*              BSP_CriticalIntHandler().
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
*                                               \ /
*                                                V
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OS_CPU_ISR_CRITICAL)
                                                         /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu     1,-STK_CTX_SIZE(1)                          /* Adjust the stack pointer                    */

    stw      0, STK_OFFSET_R0(1)                         /* Save General-Purpose Register 0             */

    stmw     2, STK_OFFSET_R2R31(1)                      /* Save General-Purpose Registers 2-31         */

    mflr     0
    stw      0, STK_OFFSET_LR(1)                         /* Save the Link Register                      */

    mfctr    0
    stw      0, STK_OFFSET_CTR(1)                        /* Save the Count Register                     */

    mfxer    0
    stw      0, STK_OFFSET_XER(1)                        /* Save the Fixed-Point Exception Register     */

    mfcr     0
    stw      0, STK_OFFSET_CR(1)                         /* Save the Condition Register                 */

    mfspr    0, 0x100
    stw      0, STK_OFFSET_USPRG0(1)                     /* Save User SPR General-Purpose Register 0    */

    mfcsrr1  0
    stw      0, STK_OFFSET_MSR(1)                        /* Save the Machine-State Register             */

    mfcsrr0  0
    stw      0, STK_OFFSET_PC(1)                         /* Save the return address                     */

                                                         /* See if a non-critical ISR was interrupted   */
    mfevpr   4
    subf     5, 4, 0                                     /* Subtract the EVPR from the return address   */
    cmpli    1, 0, 5, DEBUG_ISR@l                        /* Compare the result to the last offset       */
    lis      0, BSP_CriticalIntHandler@h                 /* Load the address of BSP_CriticalIntHandler  */
    ori      0, 0, BSP_CriticalIntHandler@l
    mtlr     0
    bc       0x05, 0x04, CRITICAL_CHECK_NESTING          /* Branch if the return address is not in...   */
	                                                     /* ...the interrupt-handler table              */
    blrl
    b        CRITICAL_RESTORE_CTX


CRITICAL_CHECK_NESTING:

    lis      4, OSIntNesting@h                           /* See if OSIntNesting == 0                    */
    ori      4, 4, OSIntNesting@l
    lwz      5, 0(4)
    addic.   5, 5, 0
    bc       0x04, 0x02, CRITICAL_INC_NESTING

    lis      6, OSTCBCur@h                               /* Save the current task's stack pointer       */
    ori      6, 6, OSTCBCur@l
    lwz      7, 0(6)
    stw      1, 0(7)

CRITICAL_INC_NESTING:

    addi     5, 5, 1                                     /* Increment OSIntNesting                      */
    stb      5, 0(4)

    blrl                                                 /* Call the interrupt controller's handler     */

    lis      0, OSIntExit@h                              /* Call OSIntExit()                            */
    ori      0, 0, OSIntExit@l
    mtlr     0
    blrl

                                                         /* *** RESTORE INTERRUPTED TASK'S CONTEXT **** */
CRITICAL_RESTORE_CTX:
    lwz      0, STK_OFFSET_MSR(1)                        /* Restore the Machine-State Register to CSRR1 */
    mtcsrr1  0

    lwz      0, STK_OFFSET_PC(1)                         /* Restore the Machine-State Register to CSRR0 */
    mtcsrr0  0

    lwz      0, STK_OFFSET_USPRG0(1)                     /* Restore User SPR General-Purpose Register 0 */
    mtspr    0x100, 0

    lwz      0, STK_OFFSET_CR(1)                         /* Restore the Condition Register              */
    mtcr     0

    lwz      0, STK_OFFSET_XER(1)                        /* Restore the Fixed-Point Exception Register  */
    mtxer    0

    lwz      0, STK_OFFSET_CTR(1)                        /* Restore the Count Register                  */
    mtctr    0

    lwz      0, STK_OFFSET_LR(1)                         /* Restore the Link Register                   */
    mtlr     0

    lmw      2, STK_OFFSET_R2R31(1)                      /* Restore General-Purpose Registers 2-31      */

    lwz      0, STK_OFFSET_R0(1)                         /* Restore General-Purpose Register 0          */

    addi     1, 1,STK_CTX_SIZE                           /* Adjust the stack pointer                    */

    rfci                                                 /* Return to the interrupted task              */

    .endm

/*
*********************************************************************************************************
*                                      OS_CPU_ISR_NON_CRITICAL
*
* Description: This macro is placed at offset 0x0300 in the interrupt-handler table, so it is executed
*              whenever the external non-critical interrupt signal is asserted.  This code saves the
*              current task's context before calling BSP_NonCriticalIntHandler(), which provides further,
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
*                                               \ /
*                                                V
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OS_CPU_ISR_NON_CRITICAL)


                                                        /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu    1, -STK_CTX_SIZE(1)                         /* Adjust the stack pointer                    */

    stw     0, STK_OFFSET_R0(1)                         /* Save General-Purpose Register 0             */

    stmw    2, STK_OFFSET_R2R31(1)                      /* Save General-Purpose Registers 2-31         */

    mflr    0
    stw     0, STK_OFFSET_LR(1)                         /* Save the Link Register                      */

    mfctr   0
    stw     0, STK_OFFSET_CTR(1)                        /* Save the Count Register                     */

    mfxer   0
    stw     0, STK_OFFSET_XER(1)                        /* Save the Fixed-Point Exception Register     */

    mfcr    0
    stw     0, STK_OFFSET_CR(1)                         /* Save the Condition Register                 */

    mfspr   0, 0x100
    stw     0, STK_OFFSET_USPRG0(1)                     /* Save User SPR General-Purpose Register 0    */

    mfsrr1  0
    stw     0, STK_OFFSET_MSR(1)                        /* Save the Machine-State Register             */

    mfsrr0  0
    stw     0, STK_OFFSET_PC(1)                         /* Save the return address                     */

    lis     4, OSIntNesting@h                           /* See if OSIntNesting == 0                    */
    ori     4, 4, OSIntNesting@l
    lwz     5, 0(4)
    addic.  5, 5, 0
    bc      0x04, 0x02, NON_CRITICAL_INC_NESTING

    lis     6, OSTCBCur@h                               /* Save the current task's stack pointer       */
    ori     6, 6, OSTCBCur@l
    lwz     7, 0(6)
    stw     1, 0(7)

NON_CRITICAL_INC_NESTING:

    addi    5, 5, 1                                     /* Increment OSIntNesting                      */
    stb     5, 0(4)

    lis     0, BSP_NonCriticalIntHandler@h              /* Call the interrupt controller's handler     */
    ori     0, 0, BSP_NonCriticalIntHandler@l
    mtlr    0
    blrl

    lis     0, OSIntExit@h                              /* Call OSIntExit()                            */
    ori     0, 0, OSIntExit@l
    mtlr    0
    blrl

                                                        /* *** RESTORE INTERRUPTED TASK'S CONTEXT **** */

    lwz     0, STK_OFFSET_MSR(1)                        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  0

    lwz     0, STK_OFFSET_PC(1)                         /* Restore the return address to SRR0          */
    mtsrr0  0

    lwz     0, STK_OFFSET_USPRG0(1)                     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100, 0

    lwz     0, STK_OFFSET_CR(1)                         /* Restore the Condition Register              */
    mtcr    0

    lwz     0, STK_OFFSET_XER(1)                        /* Restore the Fixed-Point Exception Register  */
    mtxer   0

    lwz     0, STK_OFFSET_CTR(1)                        /* Restore the Count Register                  */
    mtctr   0

    lwz     0, STK_OFFSET_LR(1)                         /* Restore the Link Register                   */
    mtlr    0

    lmw     2, STK_OFFSET_R2R31(1)                      /* Restore General-Purpose Registers 2-31      */

    lwz     0, STK_OFFSET_R0(1)                         /* Restore General-Purpose Register 0          */

    addi    1, 1, STK_CTX_SIZE                          /* Adjust the stack pointer                    */

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
*                                               \ /
*                                                V
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
*                                               \ /
*                                                V
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OSCtxSw)

                                                    /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu    1,-STK_CTX_SIZE(1)                          /* Adjust the stack pointer                    */

    stw     0, STK_OFFSET_R0(1)                         /* Save General-Purpose Register 0             */

    stmw    2, STK_OFFSET_R2R31(1)                      /* Save General-Purpose Registers 2-31         */

    mflr    0
    stw     0, STK_OFFSET_LR(1)                         /* Save the Link Register                      */

    mfctr   0
    stw     0, STK_OFFSET_CTR(1)                        /* Save the Count Register                     */

    mfxer   0
    stw     0, STK_OFFSET_XER(1)                        /* Save the Fixed-Point Exception Register     */

    mfcr    0
    stw     0, STK_OFFSET_CR(1)                         /* Save the Condition Register                 */

    mfspr   0, 0x100
    stw     0, STK_OFFSET_USPRG0(1)                     /* Save User SPR General-Purpose Register 0    */

    mfsrr1  0
    stw     0, STK_OFFSET_MSR(1)                        /* Save the Machine-State Register             */

    mfsrr0  0
    stw     0, STK_OFFSET_PC(1)                         /* Save the return address                     */

    lis     4, OSTCBCur@h                               /* Save the current task's stack pointer       */
    ori     4, 4, OSTCBCur@l
    lwz     5, 0(4)
    stw     1, 0(5)

    lis     0, OSTaskSwHook@h                           /* Call OSTaskSwHook()                         */
    ori     0, 0, OSTaskSwHook@l
    mtlr    0
    blrl

    lis     4, OSPrioHighRdy@h                          /* Update the current priority                 */
    ori     4, 4, OSPrioHighRdy@l
    lbz     5, 0(4)
    lis     6, OSPrioCur@h
    ori     6, 6, OSPrioCur@l
    stb     5, 0(6)

    lis     4, OSTCBHighRdy@h                           /* Update the current TCB                      */
    ori     4, 4, OSTCBHighRdy@l
    lwz     5, 0(4)
    lis     6, OSTCBCur@h
    ori     6, 6, OSTCBCur@l
    stw     5, 0(6)

    lwz     1, 0(5)                                     /* Load the new task's stack pointer           */

                                                        /* ******** RESTORE NEW TASK'S CONTEXT ******* */

    lwz     0, STK_OFFSET_MSR(1)                        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  0

    lwz     0, STK_OFFSET_PC(1)                         /* Restore the return address to SRR0          */
    mtsrr0  0

    lwz     0, STK_OFFSET_USPRG0(1)                     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100, 0

    lwz     0, STK_OFFSET_CR(1)                         /* Restore the Condition Register              */
    mtcr    0

    lwz     0, STK_OFFSET_XER(1)                        /* Restore the Fixed-Point Exception Register  */
    mtxer   0

    lwz     0, STK_OFFSET_CTR(1)                        /* Restore the Count Register                  */
    mtctr   0

    lwz     0, STK_OFFSET_LR(1)                         /* Restore the Link Register                   */
    mtlr    0

    lmw     2, STK_OFFSET_R2R31(1)                      /* Restore General-Purpose Registers 2-31      */

    lwz     0, STK_OFFSET_R0(1)                         /* Restore General-Purpose Register 0          */

    addi    1, 1, STK_CTX_SIZE                          /* Adjust the stack pointer                    */

    rfi                                                 /* Return to the interrupted task              */

    .endm

/*
*********************************************************************************************************
*                                            OSTickISR
*
* Description: This macro is used to process uC/OS-II tick interrupts, which are produced by the
*              CPU's Decrementer.  This code saves the current task's context before calling
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
*                                               \ /
*                                                V
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
*********************************************************************************************************
*/

    MACRO0(OSTickISR)

                                                        /* ******* SAVE CURRENT TASK'S CONTEXT ******* */
    stwu    1, -STK_CTX_SIZE(1)                         /* Adjust the stack pointer                    */

    stw     0, STK_OFFSET_R0(1)                         /* Save General-Purpose Register 0             */

    stmw    2, STK_OFFSET_R2R31(1)                      /* Save General-Purpose Registers 2-31         */

    mflr    0
    stw     0, STK_OFFSET_LR(1)                         /* Save the Link Register                      */

    mfctr   0
    stw     0, STK_OFFSET_CTR(1)                        /* Save the Count Register                     */

    mfxer   0
    stw     0, STK_OFFSET_XER(1)                        /* Save the Fixed-Point Exception Register     */

    mfcr    0
    stw     0, STK_OFFSET_CR(1)                         /* Save the Condition Register                 */

    mfspr   0, 0x100
    stw     0, STK_OFFSET_USPRG0(1)                     /* Save User SPR General-Purpose Register 0    */

    mfsrr1  0
    stw     0, STK_OFFSET_MSR(1)                        /* Save the Machine-State Register             */

    mfsrr0  0
    stw     0, STK_OFFSET_PC(1)                         /* Save the return address                     */

    lis     4, OSIntNesting@h                           /* See if OSIntNesting == 0                    */
    ori     4, 4, OSIntNesting@l
    lbz     5, 0(4)
    addic.  5, 5, 0
    bc      0x04, 0x02, TICK_INC_NESTING

    lis     6, OSTCBCur@h                               /* Save the current task's stack pointer       */
    ori     6, 6, OSTCBCur@l
    lwz     7, 0(6)
    stw     1, 0(7)

TICK_INC_NESTING:

    addi    5, 5, 1                                     /* Increment OSIntNesting                      */
    stb     5, 0(4)

    lis     0, 0x0800                                   /* Clear the Decrementer interrupt             */
    mttsr   0

    lis     0, OSTimeTick@h                             /* Call OSTimeTick()                           */
    ori     0, 0, OSTimeTick@l
    mtlr    0
    blrl

    lis     0, OSIntExit@h                              /* Call OSIntExit()                            */
    ori     0, 0, OSIntExit@l
    mtlr    0
    blrl

                                                        /* **** RESTORE INTERRUPTED TASK'S CONTEXT *** */

    lwz     0, STK_OFFSET_MSR(1)                        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  0

    lwz     0, STK_OFFSET_PC(1)                         /* Restore the return address to SRR0          */
    mtsrr0  0

    lwz     0, STK_OFFSET_USPRG0(1)                     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100, 0

    lwz     0, STK_OFFSET_CR(1)                         /* Restore the Condition Register              */
    mtcr    0

    lwz     0, STK_OFFSET_XER(1)                        /* Restore the Fixed-Point Exception Register  */
    mtxer   0

    lwz     0, STK_OFFSET_CTR(1)                        /* Restore the Count Register                  */
    mtctr   0

    lwz     0, STK_OFFSET_LR(1)                         /* Restore the Link Register                   */
    mtlr    0

    lmw     2, STK_OFFSET_R2R31(1)                      /* Restore General-Purpose Registers 2-31      */

    lwz     0, STK_OFFSET_R0(1)                         /* Restore General-Purpose Register 0          */

    addi    1, 1, STK_CTX_SIZE                          /* Adjust the stack pointer                    */

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
*                                 PowerPC440 Interrupt-Handler Table
*
* This is the interrupt-handler table used by the PowerPC 440 to appropriately invoke handlers for various
* types of interrupts.  The entry points listed below are set accordingly to the IOVRs registers which are
* initialized in OSStartHighRdy routine.
*
*
* All table entries that are not currently used by uC/OS-II contain null routines that simply return
* from the interrupt without performing any actions.  These routines can be replaced if an application
* requires support for additional types of exceptions.
*********************************************************************************************************
*/
.globl	_vectorbase
.align  4                              /* 16 bytes alignment to ensure the IVORs are aligned correctly */

_vectorbase:

.org _vectorbase + IVOR0_VAL                            /* "Vector 0x00", Critical interrupt.          */
	IVOR0:  b     CRITICAL

.org _vectorbase + IVOR1_VAL                            /* "Vector 0x10", Machine Check interrupt.     */
	IVOR1:  b     NULL_CRITICAL


.org _vectorbase + IVOR2_VAL                            /* "Vector 0x20", Data Storage interrupt.      */
	IVOR2:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR3_VAL                            /* "Vector 0x30", Instruct. Storage interrupt. */
	IVOR3:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR4_VAL                            /* "Vector 0x40", External interrupt.          */
	IVOR4:  b     NONCRITICAL


.org _vectorbase + IVOR5_VAL                            /* "Vector 0x50", Alignment interrupt.         */
	IVOR5:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR6_VAL                            /* "Vector 0x60", Program interrupt.           */
	IVOR6:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR7_VAL                            /* " Vector 0x70", FPU Unavailable interrupt.  */
	IVOR7:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR8_VAL                            /* "Vector 0x80", System Call interrupt.       */
	IVOR8:  b     SYSTEM_CALL_ISR


.org _vectorbase + IVOR9_VAL                            /* "Vector 0x90", APU Available interrupt.     */
	IVOR9:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR10_VAL                           /* "Vector 0xA0", Decrementer interrupt.       */
	IVOR10:  b     DEC_ISR


.org _vectorbase + IVOR11_VAL                           /* "Vector 0xB0", FIT interrupt.               */
	IVOR11:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR12_VAL                           /* "Vector 0xC0", Watchdog Timer interrupt.    */
	IVOR12:  b     NULL_CRITICAL


.org _vectorbase + IVOR13_VAL                           /* "Vector 0xD0", Data TLB Miss interrupt.     */
	IVOR13:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR14_VAL                           /* "Vector 0xE0", Instruct. TLB Miss interr.   */
	IVOR14:  b     NULL_NONCRITICAL


.org _vectorbase + IVOR15_VAL                           /* "Vector 0xF0", Debug interrupt.             */
	IVOR15:  b     NULL_CRITICAL


.org _vectorbase + 0x0300
NONCRITICAL:	OS_CPU_ISR_NON_CRITICAL

.org _vectorbase + 0x0400
CRITICAL:	OS_CPU_ISR_CRITICAL

.org _vectorbase + 0x0500
MACHINE:	OS_NULL_CRITICAL

.org _vectorbase + 0x0600
SYSTEM_CALL_ISR: OSCtxSw

.org _vectorbase + 0x0700
DEC_ISR: OSTickISR

.org _vectorbase + 0x0800
NULL_NONCRITICAL: OS_NULL_NON_CRITICAL

.org _vectorbase + 0x0900
NULL_CRITICAL: OS_NULL_CRITICAL

.org _vectorbase + 0x0a00
DEBUG_ISR: OS_NULL_CRITICAL


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
*                      Restore MSR and PC to CSRR1 and CSRR0, respectively;
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
*                                               \ /
*                                                V
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
* Note(s)    : 1) If the task frame was saved by a critical ISR, the stack entry at offset 0x20 will be
*                 set to 1.
*                 If the task frame was saved by a non-critical ISR, the stack entry at offset 0x20 will
*                 be set to 0.
*********************************************************************************************************
*/

OSIntCtxSw:

    lis     0, OSTaskSwHook@h                           /* Call OSTaskSwHook()                         */
    ori     0, 0, OSTaskSwHook@l
    mtlr    0
    blrl

    lis     4, OSPrioHighRdy@h                          /* Update the current priority                 */
    ori     4, 4, OSPrioHighRdy@l
    lbz     5, 0(4)
    lis     6, OSPrioCur@h
    ori     6, 6, OSPrioCur@l
    stb     5, 0(6)

    lis     4, OSTCBHighRdy@h                           /* Update the current TCB                      */
    ori     4, 4, OSTCBHighRdy@l
    lwz     5, 0(4)
    lis     6, OSTCBCur@h
    ori     6, 6, OSTCBCur@l
    stw     5, 0(6)

    lwz     1, 0(5)                                     /* Load the new task's stack pointer           */

                                                        /* ******** RESTORE NEW TASK'S CONTEXT ******* */

    lwz     0, STK_OFFSET_MSR(1)                        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  0

    lwz     0, STK_OFFSET_PC(1)                         /* Restore the return address to SRR0          */
    mtsrr0  0

    lwz     0, STK_OFFSET_USPRG0(1)                     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100, 0

    lwz     0, STK_OFFSET_CR(1)                         /* Restore the Condition Register              */
    mtcr    0

    lwz     0, STK_OFFSET_XER(1)                        /* Restore the Fixed-Point Exception Register  */
    mtxer   0

    lwz     0, STK_OFFSET_CTR(1)                        /* Restore the Count Register                  */
    mtctr   0

    lwz     0, STK_OFFSET_LR(1)                         /* Restore the Link Register                   */
    mtlr    0

    lmw     2, STK_OFFSET_R2R31(1)                      /* Restore General-Purpose Registers 2-31      */

    lwz     0, STK_OFFSET_R0(1)                         /* Restore General-Purpose Register 0          */

    addi    1, 1, STK_CTX_SIZE                          /* Adjust the stack pointer                    */

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
*                                               \ /
*                                                V
*                                        + 0x9C     GPR31                        (HIGH MEMORY)
*
* Note(s)    : 1) The MSR entry in the stack is initialized so that interrupts will be enabled when the
*                 task begins to run.  In order to allow the interrupts to be properly serviced,
*                 OSStartHighRdy() initializes IVPR and IOVRs registers.
*********************************************************************************************************
*/

OSStartHighRdy:

    lis     0, OSTaskSwHook@h                           /* Call OSTaskSwHook()                         */
    ori     0, 0, OSTaskSwHook@l
    mtlr    0
    blrl

    li      0, 1                                        /* Indicate that the OS is running             */
    lis     4, OSRunning@h
    ori     4, 4, OSRunning@l
    stb     0, 0(4)

    lis     4, OSTCBHighRdy@h                           /* Load the first task's stack pointer         */
    ori     4, 4, OSTCBHighRdy@l
    lwz     5, 0(4)
    lwz     1, 0(5)

    lis     4,  _vectorbase@h                           /* Initialize the IVPR register                */
    ori   4, 4,  _vectorbase@l
    mtivpr  4


                                                        /* Initialize the IVORs registers              */
    mtivor0 4

    addi     4, 4, IVOR1_VAL
    mtivor1 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor2 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor3 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor4 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor5 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor6 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor7 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor8 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor9 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor10 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor11 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor12 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor13 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor14 4

    addi     4, 4, SIXTEEN_BYTES
    mtivor15 4


    lwz     0, STK_OFFSET_MSR(1)                        /* Restore the Machine-State Register to SRR1  */
    mtsrr1  0

    lwz     0, STK_OFFSET_PC(1)                         /* Restore the return address to SRR0          */
    mtsrr0  0

    lwz     0, STK_OFFSET_USPRG0(1)                     /* Restore User SPR General-Purpose Register 0 */
    mtspr   0x100, 0

    lwz     0, STK_OFFSET_CR(1)                         /* Restore the Condition Register              */
    mtcr    0

    lwz     0, STK_OFFSET_XER(1)                        /* Restore the Fixed-Point Exception Register  */
    mtxer   0

    lwz     0, STK_OFFSET_CTR(1)                        /* Restore the Count Register                  */
    mtctr   0

    lwz     0, STK_OFFSET_LR(1)                         /* Restore the Link Register                   */
    mtlr    0

    lmw     2, STK_OFFSET_R2R31(1)                      /* Restore General-Purpose Registers 2-31      */

    lwz     0, STK_OFFSET_R0(1)                         /* Restore General-Purpose Register 0          */

    addi    1, 1, STK_CTX_SIZE                          /* Adjust the stack pointer                    */

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

    addis  4, 0, 0xFFFD
    ori    4, 4, 0x7FFF
    mfmsr  3
    and    4, 4, 3                                      /* Clear bits 14 and 16, corresponding to...   */
    mtmsr  4                                            /* ...critical and non-critical interrupts     */
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

    mtmsr  3                                            /* Restore the saved MSR                       */
    blr
