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
# Filename : os_cpu_a.s
# Version  : V2.93.01
#********************************************************************************************************

#*********************************************************************************************************
#*                                             ASM HEADER
#*********************************************************************************************************

    .include "os_cpu_a.inc"

    .extern BSP_TmrTickISR
    .extern BSP_ProcessorISR

    .extern OSTaskSwHook
    .extern OSIntEnter
    .extern OSIntExit

    .extern OSTCBCur                                # pointer to current tasks TCB
    .extern OSTCBHighRdy                            # pointer to highest priority ready task
    .extern OSPrioHighRdy
    .extern OSPrioCur
    .extern OSRunning
    .extern OSIntNesting


#*********************************************************************************************************
#*                                              DEFINES
#*********************************************************************************************************



#*********************************************************************************************************
#*                                          PUBLIC DECLARATIONS
#*********************************************************************************************************

    .global OSCtxSw
    .global OSStartHighRdy
    .global OSIntCtxSw

    .global OSExtIntISR
	.global Exception_Epilogue

#*********************************************************************************************************
#*                                          Section
#*********************************************************************************************************

   .text

#
#*********************************************************************************************************
#*                                      ISR HANDLER FOR EXCEPTIONS OF THE E300C1 CORE
#*
#* Description : This function is executed to restore registers when an exception occurs.
#*               The e300c1 defined the exceptions in a predefined memory space. That is each
#*               exceptions have 256 bytes to hold the exception's code. Each exceptions starts
#*               at an offset multiple of 256 bytes (e.g 0x100, 0x200, etc.).
#*               So the registers restoration is done here because if the epilogue macro is included
#*               directly in the memory space of the exceptions, the code size could exceed the 256
#*               bytes size.
#*               Except for Reset (0x100), External interrupt (0x500) and System Call (0xC00).
#*********************************************************************************************************
Exception_Epilogue:
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
    rfi                                     # return from interrupt



#*********************************************************************************************************
#*                                      ISR HANDLER FOR EXTERNAL INTERRUPT
#*
#* Description : This function is executed when an External interrupt (0x500) exception occurs.
#*********************************************************************************************************
    .align  8
OSExtIntISR:
    prologue                                        # Save context

    lis     r11,OSIntNesting@ha                     # load OSIntNesting
    lbz     r3, OSIntNesting@l(r11)
    addi    r0, r3, 1
    stb     r0, OSIntNesting@l(r11)                 # OSIntNesting++

    cmpwi   r0, 1                                   # if (OSIntNesting != 1) ...
    bne     OSExtIntISR_NotSaveSP                   # do not save the stackpointer

    lis     r11,OSTCBCur@ha                         # Get pointer to current TCB
    lwz     r11,OSTCBCur@l(r11)
    stw     r1, 0(r11)                              # Save stack pointer in current TCB

OSExtIntISR_NotSaveSP:
    bl      BSP_ProcessorISR                        # call processor interrupt handler CEDRIC AJOUT
    bl      OSIntExit                               # call to decrement OSIntNesting

    epilogue                                        # Restore context
    rfi                                             # return from interrupt



#*********************************************************************************************************
#*                                 START HIGHEST PRIORITY TASK READY-TO-RUN
#*
#* Description : This function is called by OSStart() to start the highest priority task that was created
#*               by the application before calling OSStart().
#*
#* Arguments   : none
#*
#* Note(s)     : 1) OSStartHighRdy() MUST:
#*                   a) Call OSTaskSwHook() then,
#*                   b) Set OSRunning to TRUE,
#*                   c) Switch to the highest priority task by loading the stack pointer of the highest
#*                      priority task into the SP register and execute an rfi instruction.
#*********************************************************************************************************
    .align  8
OSStartHighRdy:
    bl      OSTaskSwHook                            # Call OSTaskSwHook

    li      r0, 1                                   # Set OSRunning to 1
    lis     r11,OSRunning@ha
    stb     r0, OSRunning@l(r11)

    lis     r11,OSTCBHighRdy@ha                     # Get pointer to ready task TCB
    lwz     r11,OSTCBHighRdy@l(r11)

    lwz     r1, 0(r11)                              # Get new stack pointer

    epilogue                                        # Restore context
    rfi                                             # return from interrupt



#*********************************************************************************************************
#*                                         TASK LEVEL CONTEXT SWITCH
#*
#* Description : This function is called when a task makes a higher priority task ready-to-run.
#*
#* Arguments   : none
#*
#* Note(s)     : 1) Upon entry,
#*                  OSTCBCur     points to the OS_TCB of the task to suspend
#*                  OSTCBHighRdy points to the OS_TCB of the task to resume
#*********************************************************************************************************
    .align  8
OSCtxSw:
    prologue                                        # Save context

    lis     r11,OSTCBCur@ha                         # Get pointer to current TCB
    lwz     r11,OSTCBCur@l(r11)
    stw     r1, 0(r11)                              # Save stack pointer in current TCB

    bl      OSTaskSwHook                            # Call OSTaskSwHook

    lis     r11,OSTCBHighRdy@ha                     # Get pointer to ready task TCB
    lwz     r11,OSTCBHighRdy@l(r11)

    lis     r12,OSTCBCur@ha                         # Save as current task TCB ptr.
    stw     r11,OSTCBCur@l(r12)

    lis     r12,OSPrioHighRdy@ha                    # Get High Ready Priority
    lbz     r10,OSPrioHighRdy@l(r12)

    lis     r12,OSPrioCur@ha                        # Save as Current Priority
    stb     r10,OSPrioCur@l(r12)

    lwz     r1, 0(r11)                              # Get new stack pointer from TCB

    epilogue                                        # Restore context
    rfi                                             # return from interrupt



#*********************************************************************************************************
#*                                      INTERRUPT LEVEL CONTEXT SWITCH
#*
#* Description : This function is called by OSIntExit() to perform a context switch to a task that has
#*               been made ready-to-run by an ISR.
#*********************************************************************************************************
    .align  8
OSIntCtxSw:

    bl      OSTaskSwHook                            # Call OSTaskSwHook

    lis     r11,OSTCBHighRdy@ha                     # Get pointer to ready task TCB
    lwz     r11,OSTCBHighRdy@l(r11)

    lis     r12,OSTCBCur@ha                         # Save as current task TCB ptr.
    stw     r11,OSTCBCur@l(r12)

    lis     r12,OSPrioHighRdy@ha                    # Get High Ready Priority
    lbz     r10,OSPrioHighRdy@l(r12)

    lis     r12,OSPrioCur@ha                        # Save as Current Priority
    stb     r10,OSPrioCur@l(r12)

    lwz     r1, 0(r11)                              # Get new stack pointer

    epilogue                                        # Restore context
    rfi                                             # run task


#*********************************************************************************************************
#*                                     CPU ASSEMBLY PORT FILE END
#*********************************************************************************************************
    .end
