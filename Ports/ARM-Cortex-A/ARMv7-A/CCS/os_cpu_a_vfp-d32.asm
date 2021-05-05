;********************************************************************************************************
;                                              uC/OS-II
;                                        The Real-Time Kernel
;
;                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
;
;                                 SPDX-License-Identifier: APACHE-2.0
;
;               This software is subject to an open source license and is distributed by
;                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
;                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
;
;********************************************************************************************************

;********************************************************************************************************
;
;                                             ARMv7-A Port
;
; Filename  : os_cpu_a_vfp-d32.asm
; Version   : V2.93.01
;********************************************************************************************************
; For       : ARMv7-A Cortex-A
; Mode      : ARM or Thumb
; Toolchain : TI TMS470 COMPILER
;********************************************************************************************************
; Note(s)   : (1) See Note #2 of os_cpu.h for important information about this file.
;********************************************************************************************************


;********************************************************************************************************
;                                          EXTERNAL REFERENCE
;********************************************************************************************************
                                                                ; External references.
    .global     OSRunning
    .global     OSPrioCur
    .global     OSPrioHighRdy
    .global     OSTCBCur
    .global     OSTCBHighRdy
    .global     OSIntNesting
    .global     OSIntExit
    .global     OSTaskSwHook
    .global     OS_CPU_ExceptStkBase
    .global     OS_CPU_ExceptStkPtr
    .global     OS_CPU_ExceptHndlr                               ; Chip Support/BSP specific exception handler.


OSRunningAddr            .word     OSRunning
OSPrioCurAddr            .word     OSPrioCur
OSPrioHighRdyAddr        .word     OSPrioHighRdy
OSTCBCurAddr             .word     OSTCBCur
OSTCBHighRdyAddr         .word     OSTCBHighRdy
OSIntNestingAddr         .word     OSIntNesting
OSIntExitAddr            .word     OSIntExit
OSTaskSwHookAddr         .word     OSTaskSwHook
OS_CPU_ExceptStkBaseAddr .word     OS_CPU_ExceptStkBase
OS_CPU_ExceptStkPtrAddr  .word     OS_CPU_ExceptStkPtr
OS_CPU_ExceptHndlrAddr   .word     OS_CPU_ExceptHndlr


;********************************************************************************************************
;                                            FUNCTIONS
;********************************************************************************************************

                                                                ; Functions declared in this file.
    .global  OS_CPU_SR_Save
    .global  OS_CPU_SR_Restore

    .global  OSStartHighRdy
    .global  OSCtxSw
    .global  OSIntCtxSw

                                                                ; Functions related to exception handling.
    .global  OS_CPU_ARM_ExceptUndefInstrHndlr
    .global  OS_CPU_ARM_ExceptSwiHndlr
    .global  OS_CPU_ARM_ExceptPrefetchAbortHndlr
    .global  OS_CPU_ARM_ExceptDataAbortHndlr
    .global  OS_CPU_ARM_ExceptIrqHndlr
    .global  OS_CPU_ARM_ExceptFiqHndlr

    .global  OS_CPU_SR_INT_Dis
    .global  OS_CPU_SR_INT_En
    .global  OS_CPU_SR_FIQ_Dis
    .global  OS_CPU_SR_FIQ_En
    .global  OS_CPU_SR_IRQ_Dis
    .global  OS_CPU_SR_IRQ_En

    .global  OS_CPU_ARM_DRegCntGet


;********************************************************************************************************
;                                               EQUATES
;********************************************************************************************************

OS_CPU_ARM_CONTROL_INT_DIS        .equ  0xC0                    ; Disable both FIQ and IRQ.
OS_CPU_ARM_CONTROL_FIQ_DIS        .equ  0x40                    ; Disable FIQ.
OS_CPU_ARM_CONTROL_IRQ_DIS        .equ  0x80                    ; Disable IRQ.
OS_CPU_ARM_CONTROL_THUMB          .equ  0x20                    ; Set THUMB mode.
OS_CPU_ARM_CONTROL_ARM            .equ  0x00                    ; Set ARM mode.

OS_CPU_ARM_MODE_MASK              .equ  0x1F
OS_CPU_ARM_MODE_USR               .equ  0x10
OS_CPU_ARM_MODE_FIQ               .equ  0x11
OS_CPU_ARM_MODE_IRQ               .equ  0x12
OS_CPU_ARM_MODE_SVC               .equ  0x13
OS_CPU_ARM_MODE_ABT               .equ  0x17
OS_CPU_ARM_MODE_UND               .equ  0x1B
OS_CPU_ARM_MODE_SYS               .equ  0x1F

OS_CPU_ARM_EXCEPT_RESET           .equ  0x00
OS_CPU_ARM_EXCEPT_UNDEF_INSTR     .equ  0x01
OS_CPU_ARM_EXCEPT_SWI             .equ  0x02
OS_CPU_ARM_EXCEPT_PREFETCH_ABORT  .equ  0x03
OS_CPU_ARM_EXCEPT_DATA_ABORT      .equ  0x04
OS_CPU_ARM_EXCEPT_ADDR_ABORT      .equ  0x05
OS_CPU_ARM_EXCEPT_IRQ             .equ  0x06
OS_CPU_ARM_EXCEPT_FIQ             .equ  0x07

OS_CPU_ARM_FPEXC_EN               .equ  0x40000000


;********************************************************************************************************
;                                     CODE GENERATION DIRECTIVES
;********************************************************************************************************


     .text
     .state32


;********************************************************************************************************
;                                 FLOATING POINT REGISTER MACROS
;********************************************************************************************************

OS_CPU_ARM_FP_REG_POP    .macro  rx
                          POP     {rx}
                          VMSR    FPEXC, rx                     ;    ... Pop new task's FPEXC
                          FLDMIAD SP!, {D16-D31}
                          FLDMIAD SP!, {D0-D15}                 ;    ... Pop new task's General-Purpose floating point registers.
                          POP     {rx}
                          VMSR    FPSCR, rx                     ;    ... Pop new task's FPSCR.
                         .endm

OS_CPU_ARM_FP_REG_PUSH   .macro  rx
                          VMRS    rx, FPSCR                     ;     ... Save current FPSCR
                          PUSH    {rx}                          ;     ... Save general-purpose floating-point registers.
                          FSTMDBD SP!, {D0-D15}
                          FSTMDBD SP!, {D16-D31}
                          VMRS    rx, FPEXC                     ;     ... Save Floating point exception register.
                          PUSH    {rx}
                         .endm


;********************************************************************************************************
;                                  CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
;              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
;              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
;              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
;              into the CPU's status register.
;
; Prototypes : OS_CPU_SR  OS_CPU_SR_Save    (void);
;              void       OS_CPU_SR_Restore (OS_CPU_SR  os_cpu_sr);
;
;
; Note(s)    : (1) These functions are used in general like this:
;
;                 void Task (void  *p_arg)
;                 {
;                                                               /* Allocate storage for CPU status register.            */
;                 #if (OS_CRITICAL_METHOD == 3)
;                      OS_CPU_SR  os_cpu_sr;
;                 #endif
;
;                          :
;                          :
;                      OS_ENTER_CRITICAL();                     /* os_cpu_sr = OS_CPU_SR_Save();                        */
;                          :
;                          :
;                      OS_EXIT_CRITICAL();                      /* OS_CPU_SR_Restore(cpu_sr);                           */
;                          :
;                          :
;                 }
;********************************************************************************************************

OS_CPU_SR_Save

        MRS     R0, CPSR
        CPSID   IF                                              ; Set IRQ & FIQ bits in CPSR to DISABLE all interrupts
        DSB
        BX      LR                                              ; DISABLED, return the original CPSR contents in R0

OS_CPU_SR_Restore                                               ; See Note #2

        DSB
        MSR     CPSR_c, R0
        BX      LR


;********************************************************************************************************
;                                         START MULTITASKING
;                                      void OSStartHighRdy(void)
;
; Note(s) : 1) OSStartHighRdy() MUST:
;              a) Call OSTaskSwHook() then,
;              b) Set OSRunning to OS_STATE_OS_RUNNING,
;              c) Switch to the highest priority task.
;********************************************************************************************************

OSStartHighRdy
                                                                ; Change to SVC mode.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)
    CLREX                                                       ; Clear exclusive monitor.

    BL      OSTaskSwHook                                        ; OSTaskSwHook();

    LDR     R0, OSRunningAddr                                   ; OSRunning = TRUE;
    MOV     R1, #1
    STRB    R1, [R0]
                                                                ; SWITCH TO HIGHEST PRIORITY TASK:
    LDR     R0, OSTCBHighRdyAddr                                ;    Get highest priority task TCB address,
    LDR     R0, [R0]                                            ;    Get stack pointer,
    LDR     SP, [R0]                                            ;    Switch to the new stack,

    OS_CPU_ARM_FP_REG_POP R0

    LDR     R0, [SP], #4                                        ;    Pop new task's CPSR,
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^                              ;    Pop new task's context.


;********************************************************************************************************
;                       PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
;
; Note(s) : 1) OSCtxSw() is called in SVC mode with BOTH FIQ and IRQ interrupts DISABLED.
;
;           2) The pseudo-code for OSCtxSw() is:
;              a) Save the current task's context onto the current task's stack,
;              b) OSTCBCur->StkPtr = SP;
;              c) OSTaskSwHook();
;              d) OSPrioCur           = OSPrioHighRdy;
;              e) OSTCBCurPtr         = OSTCBHighRdy;
;              f) SP                  = OSTCBHighRdy->StkPtr;
;              g) Restore the new task's context from the new task's stack,
;              h) Return to new task's code.
;
;           3) Upon entry:
;              OSTCBCurPtr      points to the OS_TCB of the task to suspend,
;              OSTCBHighRdyPtr  points to the OS_TCB of the task to resume.
;********************************************************************************************************

OSCtxSw
                                                                ; SAVE CURRENT TASK'S CONTEXT:
    STMFD   SP!, {LR}                                           ; Push return address,
    STMFD   SP!, {LR}
    STMFD   SP!, {R0-R12}                                       ; Push registers,
    MRS     R0, CPSR                                            ; Push current CPSR,
    TST     LR, #1                                              ; See if called from Thumb mode,
    ORRNE   R0, R0, #OS_CPU_ARM_CONTROL_THUMB                   ; If yes, set the T-bit.
    STMFD   SP!, {R0}

    OS_CPU_ARM_FP_REG_PUSH  R0                                  ; Push FP context

    CLREX                                                       ; Clear exclusive monitor.

    LDR     R0, OSTCBCurAddr                                    ; OSTCBCur->StkPtr = SP;
    LDR     R1, [R0]
    STR     SP, [R1]

    BL      OSTaskSwHook                                        ; OSTaskSwHook();

    LDR     R0, OSPrioCurAddr                                   ; OSPrioCur   = OSPrioHighRdy;
    LDR     R1, OSPrioHighRdyAddr
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R0, OSTCBCurAddr                                    ; OSTCBCur = OSTCBHighRdy;
    LDR     R1, OSTCBHighRdyAddr
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     SP, [R2]                                            ; SP = OSTCBHighRdy->OSTCBStkPtr;

                                                                ; RESTORE NEW TASK'S CONTEXT:
    OS_CPU_ARM_FP_REG_POP    R0                                 ;    Pop new task's FP context.

    LDMFD   SP!, {R0}                                           ;    Pop new task's CPSR,
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^                              ;    Pop new task's context.


;********************************************************************************************************
;                   PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
;
; Note(s) : 1) OSIntCtxSw() is called in SVC mode with BOTH FIQ and IRQ interrupts DISABLED.
;
;           2) The pseudo-code for OSCtxSw() is:
;              a) OSTaskSwHook();
;              b) OSPrioCur   = OSPrioHighRdy;
;              c) OSTCBCurPtr = OSTCBHighRdyPtr;
;              d) SP          = OSTCBHighRdyPtr->OSTCBStkPtr;
;              e) Restore the new task's context from the new task's stack,
;              f) Return to new task's code.
;
;           3) Upon entry:
;              OSTCBCurPtr      points to the OS_TCB of the task to suspend,
;              OSTCBHighRdyPtr  points to the OS_TCB of the task to resume.
;********************************************************************************************************

OSIntCtxSw

    BL      OSTaskSwHook                                        ; OSTaskSwHook();

    LDR     R0, OSPrioCurAddr                                   ; OSPrioCur = OSPrioHighRdy;
    LDR     R1, OSPrioHighRdyAddr
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R0, OSTCBCurAddr                                    ; OSTCBCurPtr = OSTCBHighRdy;
    LDR     R1, OSTCBHighRdyAddr
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     SP, [R2]                                            ; SP = OSTCBHighRdyPtr->OSTCBStkPtr;

                                                                ; RESTORE NEW TASK'S CONTEXT:
    OS_CPU_ARM_FP_REG_POP R0                                    ;    Pop new task's FP context.

    LDMFD   SP!, {R0}                                           ;    Pop new task's CPSR,
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^                              ;    Pop new task's context.


;********************************************************************************************************
;                               UNDEFINED INSTRUCTION EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2     Return PC
;********************************************************************************************************

OS_CPU_ARM_ExceptUndefInstrHndlr
                                                                ; LR offset to return from this exception:  0.
    STMFD   SP!, {R0-R3}                                        ; Push working registers.
    MOV     R2, LR                                              ; Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_UNDEF_INSTR                  ; Set exception ID to OS_CPU_ARM_EXCEPT_UNDEF_INSTR.
    B            OS_CPU_ARM_ExceptHndlr                         ; Branch to global exception handler.


;********************************************************************************************************
;                                SOFTWARE INTERRUPT EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2     Return PC
;********************************************************************************************************

OS_CPU_ARM_ExceptSwiHndlr
                                                                ; LR offset to return from this exception:  0.
    STMFD   SP!, {R0-R3}                                        ; Push working registers.
    MOV     R2, LR                                              ; Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_SWI                          ; Set exception ID to OS_CPU_ARM_EXCEPT_SWI.
    B            OS_CPU_ARM_ExceptHndlr                         ; Branch to global exception handler.


;********************************************************************************************************
;                                  PREFETCH ABORT EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2     Return PC
;********************************************************************************************************

OS_CPU_ARM_ExceptPrefetchAbortHndlr
    SUB     LR, LR, #4                                          ; LR offset to return from this exception: -4.
    STMFD   SP!, {R0-R3}                                        ; Push working registers.
    MOV     R2, LR                                              ; Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_PREFETCH_ABORT               ; Set exception ID to OS_CPU_ARM_EXCEPT_PREFETCH_ABORT.
    B            OS_CPU_ARM_ExceptHndlr                         ; Branch to global exception handler.


;********************************************************************************************************
;                                    DATA ABORT EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2     Return PC
;********************************************************************************************************

OS_CPU_ARM_ExceptDataAbortHndlr
    SUB     LR, LR, #8                                          ; LR offset to return from this exception: -8.
    STMFD   SP!, {R0-R3}                                        ; Push working registers.
    MOV     R2, LR                                              ; Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_DATA_ABORT                   ; Set exception ID to OS_CPU_ARM_EXCEPT_DATA_ABORT.
    B            OS_CPU_ARM_ExceptHndlr                         ; Branch to global exception handler.


;********************************************************************************************************
;                                   ADDRESS ABORT EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2     Return PC
;********************************************************************************************************

OS_CPU_ARM_ExceptAddrAbortHndlr
    SUB     LR, LR, #8                                          ; LR offset to return from this exception: -8.
    STMFD   SP!, {R0-R3}                                        ; Push working registers.
    MOV     R2, LR                                              ; Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_ADDR_ABORT                   ; Set exception ID to OS_CPU_ARM_EXCEPT_ADDR_ABORT.
    B            OS_CPU_ARM_ExceptHndlr                         ; Branch to global exception handler.


;********************************************************************************************************
;                                 INTERRUPT REQUEST EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2     Return PC
;********************************************************************************************************

OS_CPU_ARM_ExceptIrqHndlr
    SUB     LR, LR, #4                                          ; LR offset to return from this exception: -4.
    STMFD   SP!, {R0-R3}                                        ; Push working registers.
    MOV     R2, LR                                              ; Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_IRQ                          ; Set exception ID to OS_CPU_ARM_EXCEPT_IRQ.
    B            OS_CPU_ARM_ExceptHndlr                         ; Branch to global exception handler.


;********************************************************************************************************
;                              FAST INTERRUPT REQUEST EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2     Return PC
;********************************************************************************************************

OS_CPU_ARM_ExceptFiqHndlr
    SUB     LR, LR, #4                                          ; LR offset to return from this exception: -4.
    STMFD   SP!, {R0-R3}                                        ; Push working registers.
    MOV     R2, LR                                              ; Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_FIQ                          ; Set exception ID to OS_CPU_ARM_EXCEPT_FIQ.
    B            OS_CPU_ARM_ExceptHndlr                         ; Branch to global exception handler.


;********************************************************************************************************
;                                      GLOBAL EXCEPTION HANDLER
;
; Register Usage:  R0     Exception Type
;                  R1     Exception's SPSR
;                  R2     Return PC
;                  R3     Exception's SP
;
; Note(s)       : 1) An exception can occur in three different circumstances; in each of these, the
;                    SVC stack pointer will point to a different entity :
;
;                    a) CONDITION: An exception occurs before the OS has been fully initialized.
;                       SVC STACK: Should point to a stack initialized by the application's startup code.
;                       STK USAGE: Interrupted context -- SVC stack.
;                                  Exception           -- SVC stack.
;                                  Nested exceptions   -- SVC stack.
;
;                    b) CONDITION: An exception interrupts a task.
;                       SVC STACK: Should point to task stack.
;                       STK USAGE: Interrupted context -- Task stack.
;                                  Exception           -- Exception stack 'OS_CPU_ExceptStk[]'.
;                                  Nested exceptions   -- Exception stack 'OS_CPU_ExceptStk[]'.
;
;                    c) CONDITION: An exception interrupts another exception.
;                       SVC STACK: Should point to location in exception stack, 'OS_CPU_ExceptStk[]'.
;                       STK USAGE: Interrupted context -- Exception stack 'OS_CPU_ExceptStk[]'.
;                                  Exception           -- Exception stack 'OS_CPU_ExceptStk[]'.
;                                  Nested exceptions   -- Exception stack 'OS_CPU_ExceptStk[]'.
;********************************************************************************************************

OS_CPU_ARM_ExceptHndlr

    MRS     R1, SPSR                                            ; Save CPSR (i.e. exception's SPSR).
    MOV     R3, SP                                              ; Save exception's stack pointer.

                                                                ; Adjust exception stack pointer.  This is needed because
                                                                ; exception stack is not used when restoring task context.
    ADD     SP, SP, #(4 * 4)

                                                                ; Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)
    CLREX                                                       ; Clear exclusive monitor.

    STMFD   SP!, {R2}                                           ;   Push task's PC,
    STMFD   SP!, {LR}                                           ;   Push task's LR,
    STMFD   SP!, {R4-R12}                                       ;   Push task's R12-R4,
    LDMFD   R3!, {R5-R8}                                        ;   Move task's R3-R0 from exception stack to task's stack.
    STMFD   SP!, {R5-R8}
    STMFD   SP!, {R1}                                           ;   Push task's CPSR (i.e. exception SPSR).

    OS_CPU_ARM_FP_REG_PUSH   R1
                                                                ; if (OSRunning == 1)
    LDR     R3, OSRunningAddr
    LDRB    R4, [R3]
    CMP     R4, #1
    BNE     OS_CPU_ARM_ExceptHndlr_BreakNothing

                                                                ; HANDLE NESTING COUNTER:
    LDR     R3, OSIntNestingAddr                                ;   OSIntNesting++;
    LDRB    R4, [R3]
    ADD     R4, R4, #1
    STRB    R4, [R3]

    CMP     R4, #1                                              ; if (OSIntNesting == 1)
    BNE     OS_CPU_ARM_ExceptHndlr_BreakExcept


;********************************************************************************************************
;                                 EXCEPTION HANDLER: TASK INTERRUPTED
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2
;                  R3
;********************************************************************************************************

OS_CPU_ARM_ExceptHndlr_BreakTask

    LDR     R3, OSTCBCurAddr                                    ; OSTCBCurPtr->StkPtr = SP;
    LDR     R4, [R3]
    STR     SP, [R4]

    LDR     R3, OS_CPU_ExceptStkBaseAddr                        ; Switch to exception stack.
    LDR     SP, [R3]

                                                                ; EXECUTE EXCEPTION HANDLER:
    BL      OS_CPU_ExceptHndlr                                  ; OS_CPU_ExceptHndlr(except_type = R0)

                                                                ; Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

                                                                ; Call OSIntExit().  This call MAY never return if a ready
                                                                ; task with higher priority than the interrupted one is
                                                                ; found.
    BL      OSIntExit


    LDR     R3, OSTCBCurAddr                                    ; SP = OSTCBCurPtr->StkPtr;
    LDR     R4,  [R3]
    LDR     SP,  [R4]

    OS_CPU_ARM_FP_REG_POP R0
                                                                ; RESTORE NEW TASK'S CONTEXT:
    LDMFD   SP!, {R0}                                           ;    Pop new task's CPSR,
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^                              ;    Pop new task's context.


;********************************************************************************************************
;                              EXCEPTION HANDLER: EXCEPTION INTERRUPTED
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2
;                  R3
;********************************************************************************************************

OS_CPU_ARM_ExceptHndlr_BreakExcept

    MOV     R1, SP
    AND     R1, R1, #4
    SUB     SP, SP, R1
    STMFD   SP!, {R1, LR}
                                                                ; EXECUTE EXCEPTION HANDLER:
    BL       OS_CPU_ExceptHndlr                                 ; OS_CPU_ExceptHndlr(except_type = R0)

    LDMIA   SP!, {R1, LR}
    ADD     SP, SP, R1

                                                                ; Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

                                                                ; HANDLE NESTING COUNTER:
    LDR     R3, OSIntNestingAddr                                ;   OSIntNestingCtr--;
    LDRB    R4, [R3]
    SUB     R4, R4, #1
    STRB    R4, [R3]

    OS_CPU_ARM_FP_REG_POP R0
                                                                ; RESTORE OLD CONTEXT:
    LDMFD   SP!, {R0}                                           ;    Pop old CPSR,
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^                              ;   Pull working registers and return from exception.


;********************************************************************************************************
;                              EXCEPTION HANDLER: 'NOTHING' INTERRUPTED
;
; Register Usage:  R0     Exception Type
;                  R1
;                  R2
;                  R3
;********************************************************************************************************

OS_CPU_ARM_ExceptHndlr_BreakNothing

    MOV     R1, SP
    AND     R1, R1, #4
    SUB     SP, SP, R1
    STMFD   SP!, {R1, LR}

                                                                ; EXECUTE EXCEPTION HANDLER:
    BL      OS_CPU_ExceptHndlr                                  ; OS_CPU_ExceptHndlr(except_type = R0)

    LDMIA   SP!, {R1, LR}
    ADD     SP, SP, R1

                                                                ; Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

    OS_CPU_ARM_FP_REG_POP R0
                                                                ; RESTORE OLD CONTEXT:
    LDMFD   SP!, {R0}                                           ;   Pop old CPSR,
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^                              ;   Pull working registers and return from exception.


;********************************************************************************************************
;********************************************************************************************************
;                                 ENABLE & DISABLE INTERRUPTS, IRQs, FIQs
;********************************************************************************************************
;********************************************************************************************************

;********************************************************************************************************
;                                       ENABLE & DISABLE INTERRUPTS
;
; Note(s) : 1) OS_CPU_SR_INT_En() can be called by OS_CPU_ExceptHndlr() AFTER the external
;              interrupt source has been cleared.  This function will enable IRQs and FIQs so that
;              nesting can occur.
;
;           2) OS_CPU_ARM_INT_Dis() can be called to disable IRQs and FIQs so that nesting will not occur.
;********************************************************************************************************

OS_CPU_SR_INT_En

    DSB
    MRS     R0, CPSR
    BIC     R0, R0, #OS_CPU_ARM_CONTROL_INT_DIS                 ; Clear IRQ and FIQ bits in CPSR to enable all interrupts.
    MSR     CPSR_c, R0
    BX      LR


OS_CPU_SR_INT_Dis

    MRS     R0, CPSR
    ORR     R0, R0, #OS_CPU_ARM_CONTROL_INT_DIS                 ; Set IRQ and FIQ bits in CPSR to disable all interrupts.
    MSR     CPSR_c, R0
    DSB
    BX      LR


;********************************************************************************************************
;                                          ENABLE & DISABLE IRQs
;
; Note(s) : 1) OS_CPU_SR_IRQ_En() can be called by OS_CPU_ExceptHndlr() AFTER the external
;              interrupt source has been cleared.  This function will enable IRQs so that IRQ nesting
;              can occur.
;
;           2) OS_CPU_ARM_IRQ_Dis() can be called to disable IRQs so that IRQ nesting will not occur.
;********************************************************************************************************

OS_CPU_SR_IRQ_En

    DSB
    MRS     R0, CPSR
    BIC     R0, R0, #OS_CPU_ARM_CONTROL_IRQ_DIS                 ; Clear IRQ bit in CPSR to enable IRQs.
    MSR     CPSR_c, R0
    BX      LR


OS_CPU_SR_IRQ_Dis

    MRS     R0, CPSR
    ORR     R0, R0, #OS_CPU_ARM_CONTROL_IRQ_DIS                 ; Set IRQ bit in CPSR to disable IRQs.
    MSR     CPSR_c, R0
    DSB
    BX      LR


;********************************************************************************************************
;                                          ENABLE & DISABLE FIQs
;
; Note(s) : 1) OS_CPU_SR_FIQ_En() can be called by OS_CPU_ExceptHndlr() AFTER the external
;              interrupt source has been cleared.  This function will enable FIQs so that FIQ nesting
;              can occur.
;
;           2) OS_CPU_ARM_FIQ_Dis() can be called to disable FIQs so that FIQ nesting will not occur.
;********************************************************************************************************

OS_CPU_SR_FIQ_En

    DSB
    MRS     R0, CPSR
    BIC     R0, R0, #OS_CPU_ARM_CONTROL_FIQ_DIS                 ; Clear FIQ bit in CPSR to enable FIQs.
    MSR     CPSR_c, R0
    BX      LR


OS_CPU_SR_FIQ_Dis

    MRS     R0, CPSR
    ORR     R0, R0, #OS_CPU_ARM_CONTROL_FIQ_DIS                 ; Set FIQ bit in CPSR to disable FIQs.
    MSR     CPSR_c, R0
    DSB
    BX      LR


;********************************************************************************************************
;                              VFP/NEON REGISTER COUNT
;
; Register Usage:  R0     Double Register Count
;********************************************************************************************************

OS_CPU_ARM_DRegCntGet

    MOV     R0, #32
    BX      LR
