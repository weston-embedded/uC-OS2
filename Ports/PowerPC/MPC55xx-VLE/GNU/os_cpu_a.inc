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
#                                          Freescale  MPC55xx
#
# Filename : os_cpu_a.inc
# Version  : V2.93.01
#********************************************************************************************************

#*********************************************************************************************************
#*                                              DEFINES
#*********************************************************************************************************

    .equ OS_SAVE_CONTEXT_WITH_FPRS,     1           # set to 1 for floating point support; also set in os_cpu.h

    .equ USPRG0,                      256
    .equ TCR,                         340
    .equ TSR,                         336
    .equ SPEFSCR,                     512
    .equ HID0,                       1008
    .equ L1CSR0,                     1010



#*********************************************************************************************************
#*                                         STACK FRAME DEFINITION
#*********************************************************************************************************

    .equ XR1     ,0
    .equ XBLK    ,XR1+4

 .if OS_SAVE_CONTEXT_WITH_FPRS
    .equ XR2     ,XBLK+4
    .equ XR3     ,XR2+8
    .equ XR4     ,XR3+8
    .equ XR5     ,XR4+8
    .equ XR6     ,XR5+8
    .equ XR7     ,XR6+8
    .equ XR8     ,XR7+8
    .equ XR9     ,XR8+8
    .equ XR10    ,XR9+8
    .equ XR11    ,XR10+8
    .equ XR12    ,XR11+8
    .equ XR13    ,XR12+8
    .equ XR14    ,XR13+8
    .equ XR15    ,XR14+8
    .equ XR16    ,XR15+8
    .equ XR17    ,XR16+8
    .equ XR18    ,XR17+8
    .equ XR19    ,XR18+8
    .equ XR20    ,XR19+8
    .equ XR21    ,XR20+8
    .equ XR22    ,XR21+8
    .equ XR23    ,XR22+8
    .equ XR24    ,XR23+8
    .equ XR25    ,XR24+8
    .equ XR26    ,XR25+8
    .equ XR27    ,XR26+8
    .equ XR28    ,XR27+8
    .equ XR29    ,XR28+8
    .equ XR30    ,XR29+8
    .equ XR31    ,XR30+8
    .equ XR0     ,XR31+8
 .else
    .equ XR2     ,XBLK+4
    .equ XR3     ,XR2+4
    .equ XR4     ,XR3+4
    .equ XR5     ,XR4+4
    .equ XR6     ,XR5+4
    .equ XR7     ,XR6+4
    .equ XR8     ,XR7+4
    .equ XR9     ,XR8+4
    .equ XR10    ,XR9+4
    .equ XR11    ,XR10+4
    .equ XR12    ,XR11+4
    .equ XR13    ,XR12+4
    .equ XR14    ,XR13+4
    .equ XR15    ,XR14+4
    .equ XR16    ,XR15+4
    .equ XR17    ,XR16+4
    .equ XR18    ,XR17+4
    .equ XR19    ,XR18+4
    .equ XR20    ,XR19+4
    .equ XR21    ,XR20+4
    .equ XR22    ,XR21+4
    .equ XR23    ,XR22+4
    .equ XR24    ,XR23+4
    .equ XR25    ,XR24+4
    .equ XR26    ,XR25+4
    .equ XR27    ,XR26+4
    .equ XR28    ,XR27+4
    .equ XR29    ,XR28+4
    .equ XR30    ,XR29+4
    .equ XR31    ,XR30+4
    .equ XR0     ,XR31+4
 .endif


    .equ XSRR0   ,XR0+4
    .equ XSRR1   ,XSRR0+4
    .equ XUSPRG  ,XSRR1+4
    .equ XCTR    ,XUSPRG+4
    .equ XXER    ,XCTR+4
    .equ XCR     ,XXER+4
    .equ XLR     ,XCR+4
    .equ XSPEFSCR,XLR+4
    .equ XPAD2   ,XSPEFSCR+4
    .equ XPAD3   ,XPAD2+4
    .equ XMSR    ,XPAD3+4

    .equ STACK_FRAME_SIZE   ,XMSR+4



#*********************************************************************************************************
#*                                         PROLOGUE DEFINITION
#*********************************************************************************************************
prologue:       .macro
        e_add2i. r1, -STACK_FRAME_SIZE
        e_stwu  r1,    0(r1)
        e_stw   r0,  XR0(r1)

    .if OS_SAVE_CONTEXT_WITH_FPRS
        mfmsr   r0                                  # system call (sc) clears the SPE bit ...
        e_or2is r0,  0x0200                         # ... therefore set them first
        e_or2i  r0,  0x0000
        mtmsr   r0
        isync

        evstdd  r2,  XR2(r1)                         # store GPR as 64bit Register
        evstdd  r3,  XR3(r1)
        evstdd  r4,  XR4(r1)
        evstdd  r5,  XR5(r1)
        evstdd  r6,  XR6(r1)
        evstdd  r7,  XR7(r1)
        evstdd  r8,  XR8(r1)
        evstdd  r9,  XR9(r1)
        evstdd  r10, XR10(r1)
        evstdd  r11, XR11(r1)
        evstdd  r12, XR12(r1)
        evstdd  r13, XR13(r1)
        evstdd  r14, XR14(r1)
        evstdd  r15, XR15(r1)
        evstdd  r16, XR16(r1)
        evstdd  r17, XR17(r1)
        evstdd  r18, XR18(r1)
        evstdd  r19, XR19(r1)
        evstdd  r20, XR20(r1)
        evstdd  r21, XR21(r1)
        evstdd  r22, XR22(r1)
        evstdd  r23, XR23(r1)
        evstdd  r24, XR24(r1)
        evstdd  r25, XR25(r1)
        evstdd  r26, XR26(r1)
        evstdd  r27, XR27(r1)
        evstdd  r28, XR28(r1)
        evstdd  r29, XR29(r1)
        evstdd  r30, XR30(r1)
        evstdd  r31, XR31(r1)
    .else
        e_stw   r2,  XR2(r1)                        # store GPR as 32bit Register
        e_stw   r3,  XR3(r1)
        e_stw   r4,  XR4(r1)
        e_stw   r5,  XR5(r1)
        e_stw   r6,  XR6(r1)
        e_stw   r7,  XR7(r1)
        e_stw   r8,  XR8(r1)
        e_stw   r9,  XR9(r1)
        e_stw   r10, XR10(r1)
        e_stw   r11, XR11(r1)
        e_stw   r12, XR12(r1)
        e_stw   r13, XR13(r1)
        e_stw   r14, XR14(r1)
        e_stw   r15, XR15(r1)
        e_stw   r16, XR16(r1)
        e_stw   r17, XR17(r1)
        e_stw   r18, XR18(r1)
        e_stw   r19, XR19(r1)
        e_stw   r20, XR20(r1)
        e_stw   r21, XR21(r1)
        e_stw   r22, XR22(r1)
        e_stw   r23, XR23(r1)
        e_stw   r24, XR24(r1)
        e_stw   r25, XR25(r1)
        e_stw   r26, XR26(r1)
        e_stw   r27, XR27(r1)
        e_stw   r28, XR28(r1)
        e_stw   r29, XR29(r1)
        e_stw   r30, XR30(r1)
        e_stw   r31, XR31(r1)
    .endif

        mfmsr   r0
        e_stw   r0,  XMSR(r1)

        mfspr   r0,  SPEFSCR
        e_stw   r0,  XSPEFSCR(r1)

        mfspr   r0,  SRR0
        e_stw   r0,  XSRR0(r1)

        mfspr   r0,  SRR1
        e_stw   r0,  XSRR1(r1)

        mfspr   r0,  USPRG0
        e_stw   r0,  XUSPRG(r1)

        mflr    r0
        e_stw   r0,  XLR(r1)

        mfctr   r0
        e_stw   r0,  XCTR(r1)

        mfxer   r0
        e_stw   r0,  XXER(r1)

        mfcr    r0
        e_stw   r0,  XCR(r1)

        mfmsr   r0
    .endm



#*********************************************************************************************************
#*                                         EPILOGUE DEFINITION
#*********************************************************************************************************
epilogue:       .macro
        wrteei  0

    .if OS_SAVE_CONTEXT_WITH_FPRS
        evldd   r2,  XR2(r1)                        # load GPR as 64bit Register
        evldd   r3,  XR3(r1)
        evldd   r4,  XR4(r1)
        evldd   r5,  XR5(r1)
        evldd   r6,  XR6(r1)
        evldd   r7,  XR7(r1)
        evldd   r8,  XR8(r1)
        evldd   r9,  XR9(r1)
        evldd   r10, XR10(r1)
        evldd   r11, XR11(r1)
        evldd   r12, XR12(r1)
        evldd   r13, XR13(r1)
        evldd   r14, XR14(r1)
        evldd   r15, XR15(r1)
        evldd   r16, XR16(r1)
        evldd   r17, XR17(r1)
        evldd   r18, XR18(r1)
        evldd   r19, XR19(r1)
        evldd   r20, XR20(r1)
        evldd   r21, XR21(r1)
        evldd   r22, XR22(r1)
        evldd   r23, XR23(r1)
        evldd   r24, XR24(r1)
        evldd   r25, XR25(r1)
        evldd   r26, XR26(r1)
        evldd   r27, XR27(r1)
        evldd   r28, XR28(r1)
        evldd   r29, XR29(r1)
        evldd   r30, XR30(r1)
        evldd   r31, XR31(r1)
    .else
        e_lwz   r2,  XR2(r1)                        # load GPR as 32bit Register
        e_lwz   r3,  XR3(r1)
        e_lwz   r4,  XR4(r1)
        e_lwz   r5,  XR5(r1)
        e_lwz   r6,  XR6(r1)
        e_lwz   r7,  XR7(r1)
        e_lwz   r8,  XR8(r1)
        e_lwz   r9,  XR9(r1)
        e_lwz   r10, XR10(r1)
        e_lwz   r11, XR11(r1)
        e_lwz   r12, XR12(r1)
        e_lwz   r13, XR13(r1)
        e_lwz   r14, XR14(r1)
        e_lwz   r15, XR15(r1)
        e_lwz   r16, XR16(r1)
        e_lwz   r17, XR17(r1)
        e_lwz   r18, XR18(r1)
        e_lwz   r19, XR19(r1)
        e_lwz   r20, XR20(r1)
        e_lwz   r21, XR21(r1)
        e_lwz   r22, XR22(r1)
        e_lwz   r23, XR23(r1)
        e_lwz   r24, XR24(r1)
        e_lwz   r25, XR25(r1)
        e_lwz   r26, XR26(r1)
        e_lwz   r27, XR27(r1)
        e_lwz   r28, XR28(r1)
        e_lwz   r29, XR29(r1)
        e_lwz   r30, XR30(r1)
        e_lwz   r31, XR31(r1)
    .endif

        e_lwz   r0,  XCR(r1)
        mtcrf   0xff,r0
        e_lwz   r0,  XXER(r1)
        mtxer   r0
        e_lwz   r0,  XCTR(r1)
        mtctr   r0
        e_lwz   r0,  XLR(r1)
        mtlr    r0

        e_lwz   r0,  XSRR1(r1)
        mtspr   SRR1,r0
        e_lwz   r0,  XSRR0(r1)
        mtspr   SRR0,r0

        e_lwz   r0,  XUSPRG(r1)
        mtspr   USPRG0,r0

        e_lwz   r0,  XSPEFSCR(r1)
        mtspr   SPEFSCR,r0

        e_lwz   r0,  XR0(r1)
        e_add2i. r1, STACK_FRAME_SIZE
    .endm


#*********************************************************************************************************
#*                                   CPU ASSEMBLY PORT INCLUDE FILE END
#*********************************************************************************************************
