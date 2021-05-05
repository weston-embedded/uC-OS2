#********************************************************************************************************
#                                              uC/OS-II
#                                        The Real-Time Kernel
#
#                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
#
#                                 SPDX-License-Identifier: APACHE-2.0
#
#               This software is subject to an open source license and is distributed by
#                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
#                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
#
#********************************************************************************************************

#********************************************************************************************************
#
#                                          Renesas V850ES Port
#
# Filename  : os_cpu_a.inc
# Version   : V2.93.01
#********************************************************************************************************
# For       : Renesas V850ES
# Toolchain : CubeSuite V1.20
#             CA850 compiler
#********************************************************************************************************

#********************************************************************************************************
#                                           PUBLIC FUNCTIONS
#********************************************************************************************************

    .extern     _OSTCBCur
    .extern     _OSIntNesting
    .extern     _OSIntExit


#********************************************************************************************************
#                                                EQUATES
#********************************************************************************************************

                                                                -- SYSTEM REGISTER
    .set    EIPC , 0
    .set    EIPSW, 1
    .set    ECR  , 4
    .set    PSW  , 5
    .set    CTPC , 16
    .set    CTPSW, 17

                                                                -- PROGRAM REGISTER STACK OFFSET
    .set    STK_OFFSET_R1   , 0
    .set    STK_OFFSET_R2   , STK_OFFSET_R1    + 4
    .set    STK_OFFSET_R6   , STK_OFFSET_R2    + 4
    .set    STK_OFFSET_R7   , STK_OFFSET_R6    + 4
    .set    STK_OFFSET_R8   , STK_OFFSET_R7    + 4
    .set    STK_OFFSET_R9   , STK_OFFSET_R8    + 4
    .set    STK_OFFSET_R10  , STK_OFFSET_R9    + 4
    .set    STK_OFFSET_R11  , STK_OFFSET_R10   + 4
    .set    STK_OFFSET_R12  , STK_OFFSET_R11   + 4
    .set    STK_OFFSET_R13  , STK_OFFSET_R12   + 4
    .set    STK_OFFSET_R14  , STK_OFFSET_R13   + 4
    .set    STK_OFFSET_R15  , STK_OFFSET_R14   + 4
    .set    STK_OFFSET_R16  , STK_OFFSET_R15   + 4
    .set    STK_OFFSET_R17  , STK_OFFSET_R16   + 4
    .set    STK_OFFSET_R18  , STK_OFFSET_R17   + 4
    .set    STK_OFFSET_R19  , STK_OFFSET_R18   + 4
    .set    STK_OFFSET_R20  , STK_OFFSET_R19   + 4
    .set    STK_OFFSET_R21  , STK_OFFSET_R20   + 4
    .set    STK_OFFSET_R22  , STK_OFFSET_R21   + 4
    .set    STK_OFFSET_R23  , STK_OFFSET_R22   + 4
    .set    STK_OFFSET_R24  , STK_OFFSET_R23   + 4
    .set    STK_OFFSET_R25  , STK_OFFSET_R24   + 4
    .set    STK_OFFSET_R26  , STK_OFFSET_R25   + 4
    .set    STK_OFFSET_R27  , STK_OFFSET_R26   + 4
    .set    STK_OFFSET_R28  , STK_OFFSET_R27   + 4
    .set    STK_OFFSET_R29  , STK_OFFSET_R28   + 4
    .set    STK_OFFSET_R30  , STK_OFFSET_R29   + 4
    .set    STK_OFFSET_R31  , STK_OFFSET_R30   + 4
    .set    STK_OFFSET_EIPC , STK_OFFSET_R31   + 4
                                                                -- SYSTEM REGISTER STACK OFFSET
    .set    STK_OFFSET_EIPSW, STK_OFFSET_EIPC  + 4
    .set    STK_OFFSET_CTPC , STK_OFFSET_EIPSW + 4
    .set    STK_OFFSET_CTPSW, STK_OFFSET_CTPC  + 4
    .set    STK_CTX_SIZE    , STK_OFFSET_CTPSW + 4

#********************************************************************************************************
#                                      CODE GENERATION DIRECTIVES
#********************************************************************************************************

    .text
    .align  4


#********************************************************************************************************
#                                             OS_CTX_SAVE
#
# Description : This MACRO saves the CPU registers (i.e. CPU context) onto the current task's stack using
#               the same order as they were saved in OSTaskStkInit().
#
# Note(s)     : 1) The assembler-reserved register (r1) is used as a temporary register when instruction
#                  expansion is performed using the assembler. If r1 is specified as a source or
#                  destination register, the assembler outputs a warning message; which can be suppressed
#                  with the following syntax:
#                  $NOWARNING
#                   r1 used as source/destination register
#                  $WARNING
#********************************************************************************************************

                                                                -- SAVE PROCESSOR REGISTER
.macro  OS_CTX_SAVE   REG_SP
    addi  -STK_CTX_SIZE, REG_SP, REG_SP                             -- Adjust the Stack Pointer

    st.w   r1, STK_OFFSET_R1[REG_SP]
    st.w   r2, STK_OFFSET_R2[REG_SP]
    st.w   r6, STK_OFFSET_R6[REG_SP]
    st.w   r7, STK_OFFSET_R7[REG_SP]
    st.w   r8, STK_OFFSET_R8[REG_SP]
    st.w   r9, STK_OFFSET_R9[REG_SP]
    st.w  r10, STK_OFFSET_R10[REG_SP]
    st.w  r11, STK_OFFSET_R11[REG_SP]
    st.w  r12, STK_OFFSET_R12[REG_SP]
    st.w  r13, STK_OFFSET_R13[REG_SP]
    st.w  r14, STK_OFFSET_R14[REG_SP]
    st.w  r15, STK_OFFSET_R15[REG_SP]
    st.w  r16, STK_OFFSET_R16[REG_SP]
    st.w  r17, STK_OFFSET_R17[REG_SP]
    st.w  r18, STK_OFFSET_R18[REG_SP]
    st.w  r19, STK_OFFSET_R19[REG_SP]
    st.w  r20, STK_OFFSET_R20[REG_SP]
    st.w  r21, STK_OFFSET_R21[REG_SP]
    st.w  r22, STK_OFFSET_R22[REG_SP]
    st.w  r23, STK_OFFSET_R23[REG_SP]
    st.w  r24, STK_OFFSET_R24[REG_SP]
    st.w  r25, STK_OFFSET_R25[REG_SP]
    st.w  r26, STK_OFFSET_R26[REG_SP]
    st.w  r27, STK_OFFSET_R27[REG_SP]
    st.w  r28, STK_OFFSET_R28[REG_SP]
    st.w  r29, STK_OFFSET_R29[REG_SP]
    st.w  r30, STK_OFFSET_R30[REG_SP]
    st.w  r31, STK_OFFSET_R31[REG_SP]

    stsr  EIPC, r2
    st.w  r2, STK_OFFSET_EIPC[REG_SP]                               -- Restore task's EIPC

    stsr  EIPSW, r2
    st.w  r2, STK_OFFSET_EIPSW[REG_SP]                              -- Restore task's EIPSW

    stsr  CTPC, r2
    st.w  r2, STK_OFFSET_CTPC[REG_SP]                               -- Restore task's CTPC

    stsr  CTPSW, r2
    st.w  r2, STK_OFFSET_CTPSW[REG_SP]                              -- Restore task's CTPSW
.endm


#********************************************************************************************************
#                                           OS_CTX_RESTORE
#
# Description : This MACRO restores the CPU registers (i.e. context) from the new task's stack in the
#               reverse order of OS_CTX_SAVE (see above)
#
# Note(s)     : 1) The assembler-reserved register (r1) is used as a temporary register when instruction
#                  expansion is performed using the assembler. If r1 is specified as a source or
#                  destination register, the assembler outputs a warning message; which can be suppressed
#                  with the following syntax:
#                  $NOWARNING
#                   r1 used as source/destination register
#                  $WARNING
#********************************************************************************************************

                                                                -- RESTORE PROCESSOR REGISTER
.macro  OS_CTX_RESTORE  REG_SP
    ld.w  STK_OFFSET_R1[REG_SP] , r1
    ld.w  STK_OFFSET_R6[REG_SP] , r6
    ld.w  STK_OFFSET_R7[REG_SP] , r7
    ld.w  STK_OFFSET_R8[REG_SP] , r8
    ld.w  STK_OFFSET_R9[REG_SP] , r9
    ld.w  STK_OFFSET_R10[REG_SP], r10
    ld.w  STK_OFFSET_R11[REG_SP], r11
    ld.w  STK_OFFSET_R12[REG_SP], r12
    ld.w  STK_OFFSET_R13[REG_SP], r13
    ld.w  STK_OFFSET_R14[REG_SP], r14
    ld.w  STK_OFFSET_R15[REG_SP], r15
    ld.w  STK_OFFSET_R16[REG_SP], r16
    ld.w  STK_OFFSET_R17[REG_SP], r17
    ld.w  STK_OFFSET_R18[REG_SP], r18
    ld.w  STK_OFFSET_R19[REG_SP], r19
    ld.w  STK_OFFSET_R20[REG_SP], r20
    ld.w  STK_OFFSET_R21[REG_SP], r21
    ld.w  STK_OFFSET_R22[REG_SP], r22
    ld.w  STK_OFFSET_R23[REG_SP], r23
    ld.w  STK_OFFSET_R24[REG_SP], r24
    ld.w  STK_OFFSET_R25[REG_SP], r25
    ld.w  STK_OFFSET_R26[REG_SP], r26
    ld.w  STK_OFFSET_R27[REG_SP], r27
    ld.w  STK_OFFSET_R28[REG_SP], r28
    ld.w  STK_OFFSET_R29[REG_SP], r29
    ld.w  STK_OFFSET_R30[REG_SP], r30
    ld.w  STK_OFFSET_R31[REG_SP], r31

    ld.w  STK_OFFSET_EIPSW[REG_SP], r2                              -- Restore task's EIPSW
    ldsr  r2, EIPSW

    ld.w  STK_OFFSET_EIPC[REG_SP], r2                               -- Restore task's EIPC
    ldsr  r2, EIPC

    ld.w  STK_OFFSET_CTPC[REG_SP], r2                               -- Restore task's CTPC
    ldsr  r2, CTPC

    ld.w  STK_OFFSET_CTPSW[REG_SP], r2                              -- Restore task's CTPSW
    ldsr  r2, CTPSW

    ld.w  STK_OFFSET_R2[REG_SP] , r2

    addi STK_CTX_SIZE, REG_SP, REG_SP                                   -- Adjust the Stack Pointer
.endm


#********************************************************************************************************
#                                            OS_ISR_ENTER
#
# Description : Interrupt service routine prologue for kernel-aware handler.
#               This macro implements the following code in assembly language:
#
#                  OS_ISR_ENTER
#                      OS_CTX_SAVE                   ; Call the macro: OS_CTX_SAVE
#                      OSIntNestingCtr++;
#                      if (OSIntNestingCtr == 1) {
#                          OSTCBCurPtr->StkPtr = SP;
#                      }
#
#               This MACRO is to be used by your assembly language based ISRs as follows:
#
#                  MyISR
#                      OS_ISR_ENTER
#                      ISR Body here
#                      OS_ISR_EXIT
#********************************************************************************************************

.macro  OS_ISR_ENTER
    OS_CTX_SAVE  sp                                    -- Save processor registers on the stack

    ld.b #_OSIntNesting[r0], r2                        -- OSIntNestingCtr++;
    add 0x1, r2
    st.b r2, #_OSIntNesting[r0]

    cmp  0x1, r2
    bne  _b                                            -- if (OSIntNestingCtr == 1) {

    mov   #_OSTCBCur, r11                              --     OSTCBCurPtr->OSTCBStkPtr = SP;
    ld.w  0[r11]    , r11
    st.w  sp        , 0[r11]                           -- }

_b:
.endm


#********************************************************************************************************
#                                             OS_ISR_EXIT
#
# Description : Interrupt service routine epilog for kernel-aware handler.
#               This macro implements the following code in assembly language:
#
#                  OS_ISR_EXIT:
#                      OSIntExit();                  ; Call the C function: OSIntExit();
#                      OS_CTX_RESTORE                ; Call the macro: OS_CTX_RESTORE
#                      Return from interrupt         ; CPU instruction to return from interrupt/exception
#********************************************************************************************************

.macro  OS_ISR_EXIT
    jarl _OSIntExit, lp                                -- Call 'OSIntExit()'

    OS_CTX_RESTORE  sp                                 -- Restore processor registers from stack
    reti                                               -- CPU instruction to return from Interrupt/exception

.endm


#********************************************************************************************************
#                                     ASSEMBLY LANGUAGE MACROS FILE END
#********************************************************************************************************
