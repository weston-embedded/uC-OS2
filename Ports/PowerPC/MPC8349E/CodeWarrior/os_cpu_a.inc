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
#                                          Freescale  MPC8349E
#                                         CodeWarrior Compiler
#
# Filename : os_cpu_a.inc
# Version  : V2.93.01
#********************************************************************************************************


#*********************************************************************************************************
#*                                              DEFINES
#*********************************************************************************************************

    .equ OS_SAVE_CONTEXT_WITH_FPRS,     1           # set to 1 for floating point support; also set in os_cpu.h


#*********************************************************************************************************
#*                                         STACK FRAME DEFINITION
#*********************************************************************************************************

 .ifdef OS_SAVE_CONTEXT_WITH_FPRS
   .equ  STK_FRM_SZ,    424           #/* 32 gpr + 8 spr + sp + 32 fp + fpscr -> (41 * 4 + 32 * 8 + 4 ) */
   .equ  XTR_REG_OFFS,  396           #/* (33 * 4) + (32 * 8) + 4 + 4 fill */
   .equ  R0_OFFS,       8
   .equ  R2_OFFS,       12
   .equ  XF0,           136
   .equ  XF1,           XF0+8
   .equ  XF2,           XF1+8
   .equ  XF3,           XF2+8
   .equ  XF4,           XF3+8
   .equ  XF5,           XF4+8
   .equ  XF6,           XF5+8
   .equ  XF7,           XF6+8
   .equ  XF8,           XF7+8
   .equ  XF9,           XF8+8
   .equ  XF10,          XF9+8
   .equ  XF11,          XF10+8
   .equ  XF12,          XF11+8
   .equ  XF13,          XF12+8
   .equ  XF14,          XF13+8
   .equ  XF15,          XF14+8
   .equ  XF16,          XF15+8
   .equ  XF17,          XF16+8
   .equ  XF18,         XF17+8
   .equ  XF19,          XF18+8
   .equ  XF20,          XF19+8
   .equ  XF21,          XF20+8
   .equ  XF22,          XF21+8
   .equ  XF23,          XF22+8
   .equ  XF24,          XF23+8
   .equ  XF25,          XF24+8
   .equ  XF26,         XF25+8
   .equ  XF27,          XF26+8
   .equ  XF28,          XF27+8
   .equ  XF29,          XF28+8
   .equ  XF30,          XF29+8
   .equ  XF31,          XF30+8
   .equ  XFPSCR,        XF31+8
 .else
   .equ  STK_FRM_SZ,    168           #/* 32 gpr + 8 spr + sp -> (41 * 4) */
   .equ  XTR_REG_OFFS,  132           #/* (33 * 4) */
   .equ  R0_OFFS,       8
   .equ  R2_OFFS,       12
 .endif



#*********************************************************************************************************
#*                                         PROLOGUE DEFINITION
#*    THIS Stack frame definitions have to be congruent with the
#*   Stack Frame usage in OS_CPU_C.c
#*********************************************************************************************************
prologue:       .macro

        stwu    r1,-STK_FRM_SZ(r1)      # create stack frame
        stw     r0,R0_OFFS(r1)          # save r0
        stmw    r2,R2_OFFS(r1)          # save regs r2 through r31

    .ifdef OS_SAVE_CONTEXT_WITH_FPRS
        mfmsr   r0
        ori     r0,r0,0x2000
        mtmsr   r0
        isync                                # wait for FPU availability
        stfd    f0,XF0(r1)
        stfd    f1,XF1(r1)
        stfd    f2,XF2(r1)
        stfd    f3,XF3(r1)
        stfd    f4,XF4(r1)
        stfd    f5,XF5(r1)
        stfd    f6,XF6(r1)
        stfd    f7,XF7(r1)
        stfd    f8,XF8(r1)
        stfd    f9,XF9(r1)
        stfd    f10,XF10(r1)
        stfd    f11,XF11(r1)
        stfd    f12,XF12(r1)
        stfd    f13,XF13(r1)
        stfd    f14,XF14(r1)
        stfd    f15,XF15(r1)
        stfd    f16,XF16(r1)
        stfd    f17,XF17(r1)
        stfd    f18,XF18(r1)
        stfd    f19,XF19(r1)
        stfd    f20,XF20(r1)
        stfd    f21,XF21(r1)
        stfd    f22,XF22(r1)
        stfd    f23,XF23(r1)
        stfd    f24,XF24(r1)
        stfd    f25,XF25(r1)
        stfd    f26,XF26(r1)
        stfd    f27,XF27(r1)
        stfd    f28,XF28(r1)
        stfd    f29,XF29(r1)
        stfd    f30,XF30(r1)
        stfd    f31,XF31(r1)
        mffs    f0
        stfd    f0,XFPSCR(r1)
    .endif

        mflr    r31                     # fetch LR
        mfxer   r30                     # fetch XER
        mfctr   r29                     # fetch CTR
        mfsrr1  r28                     # fetch MSR (from normal interrupt)
        mfsrr0  r27                     # fetch PC  (from normal interrupt)
        mfcr    r26                     # fetch CR
        mfspr   r25, 59				    # fetch CSRR1 (for critical interrupt)
        mfspr   r24, 58				    # fecth CSRR0
        stmw    r24,XTR_REG_OFFS(r1)    # save regs r24 - r31
    .endm



#*********************************************************************************************************
#*                                         EPILOGUE DEFINITION
#*********************************************************************************************************
epilogue:       .macro
        lmw          r24,XTR_REG_OFFS(r1)
        mtspr        58,r24			       # restore CSRR1 (for critical interrupt)
        mtspr        59,r25				   # restore CSRR0 (for critical interrupt)
        mtcr         r26                    # restore CR
        mtsrr0       r27                    # restore PC
        mtsrr1       r28                    # restore MSR
        mtctr        r29                    # restore CTR
        mtxer        r30                    # restore XER
        mtlr         r31                    # restore LR

    .ifdef OS_SAVE_CONTEXT_WITH_FPRS
        mfmsr        r0
        ori          r0,r0,0x2000
        mtmsr        r0
        lfd          f0,XFPSCR(r1)
        mtfsf        0xFF,f0
        lfd          f0,XF0(r1)
        lfd          f1,XF1(r1)
        lfd          f2,XF2(r1)
        lfd          f3,XF3(r1)
        lfd          f4,XF4(r1)
        lfd          f5,XF5(r1)
        lfd          f6,XF6(r1)
        lfd          f7,XF7(r1)
        lfd          f8,XF8(r1)
        lfd          f9,XF9(r1)
        lfd          f10,XF10(r1)
        lfd          f11,XF11(r1)
        lfd          f12,XF12(r1)
        lfd          f13,XF13(r1)
        lfd          f14,XF14(r1)
        lfd          f15,XF15(r1)
        lfd          f16,XF16(r1)
        lfd          f17,XF17(r1)
        lfd          f18,XF18(r1)
        lfd          f19,XF19(r1)
        lfd          f20,XF20(r1)
        lfd          f21,XF21(r1)
        lfd          f22,XF22(r1)
        lfd          f23,XF23(r1)
        lfd          f24,XF24(r1)
        lfd          f25,XF25(r1)
        lfd          f26,XF26(r1)
        lfd          f27,XF27(r1)
        lfd          f28,XF28(r1)
        lfd          f29,XF29(r1)
        lfd          f30,XF30(r1)
        lfd          f31,XF31(r1)
    .endif

        lmw          r2,R2_OFFS(r1)         # restore regs r2 through r31
        lwz          r0,R0_OFFS(r1)         # restore r0
        lwz          r1,0(r1)               # restore stack pointer
    .endm


#*********************************************************************************************************
#*                                   CPU ASSEMBLY PORT INCLUDE FILE END
#*********************************************************************************************************
