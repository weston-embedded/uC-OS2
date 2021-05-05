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
;                                               ARM7 Port
;                                            IAR C Compiler
;
; Filename : os_cpu_a.s
; Version  : V2.93.01
;********************************************************************************************************

        EXTERN  OSRunning                        ; External references
        EXTERN  OSTCBCur
        EXTERN  OSTCBHighRdy
        EXTERN  OSPrioCur
        EXTERN  OSPrioHighRdy
        EXTERN  OSIntCtxSwFlag

        EXTERN  OSIntEnter
        EXTERN  OSIntExit

        EXTERN  OSTaskSwHook


        PUBLIC  OSStartHighRdy                   ; Functions declared in this file

        PUBLIC  OSCtxSw
        PUBLIC  OS_IntCtxSw
        PUBLIC  OS_CPU_SR_Save
        PUBLIC  OS_CPU_SR_Restore


NO_INT  EQU     0xC0                             ; Mask used to disable interrupts (Both FIR and IRQ)



;*********************************************************************************************************
;                                   CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
;              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
;              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
;              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
;              into the CPU's status register.
;
; Prototypes :     OS_CPU_SR  OS_CPU_SR_Save(void);
;                  void       OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
;
;
; Note(s)    : 1) These functions are used in general like this:
;
;                 void Task (void *p_arg)
;                 {
;                 #if OS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register */
;                     OS_CPU_SR  cpu_sr;
;                 #endif
;
;                          :
;                          :
;                     OS_ENTER_CRITICAL();             /* cpu_sr = OS_CPU_SaveSR();                */
;                          :
;                          :
;                     OS_EXIT_CRITICAL();              /* OS_CPU_RestoreSR(cpu_sr);                */
;                          :
;                          :
;                 }
;
;              2) OS_CPU_SaveSR() is implemented as recommended by Atmel's application note:
;
;                    "Disabling Interrupts at Processor Level"
;*********************************************************************************************************

        RSEG CODE:CODE:NOROOT(2)
        CODE32

OS_CPU_SR_Save
        MRS     R0,CPSR                     ; Set IRQ and FIQ bits in CPSR to disable all interrupts
        ORR     R1,R0,#NO_INT
        MSR     CPSR_c,R1
        MRS     R1,CPSR                     ; Confirm that CPSR contains the proper interrupt disable flags
        AND     R1,R1,#NO_INT
        CMP     R1,#NO_INT
        BNE     OS_CPU_SR_Save              ; Not properly disabled (try again)
        MOV     PC,LR                       ; Disabled, return the original CPSR contents in R0


OS_CPU_SR_Restore
        MSR     CPSR_c,R0
        MOV     PC,LR


;*********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; Note : OSStartHighRdy() MUST:
;           a) Call OSTaskSwHook() then,
;           b) Set OSRunning to TRUE,
;           c) Switch to the highest priority task.
;*********************************************************************************************************

        RSEG CODE:CODE:NOROOT(2)
        CODE32

OSStartHighRdy

        BL      OSTaskSwHook            ; OSTaskSwHook();

        LDR     R4,??OS_Running         ; OSRunning = TRUE
        MOV     R5,#1
        STRB    R5,[R4]

        LDR     R4,??OS_TCBHighRdy      ; Get highest priority task TCB address
        LDR     R4,[R4]                 ; get stack pointer
        LDR     SP,[R4]                 ; switch to the new stack

        LDMFD   SP!,{R4}                ; pop new task's SPSR
        MSR     SPSR_cxsf,R4
        LDMFD   SP!,{R4}                ; pop new task's CPSR
        MSR     CPSR_cxsf,R4
        LDMFD   SP!,{R0-R12,LR,PC}      ; pop new task's R0-R12,LR & PC


;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From task level)
;
; Note(s): Upon entry:
;              OSTCBCur      points to the OS_TCB of the task to suspend
;              OSTCBHighRdy  points to the OS_TCB of the task to resume
;*********************************************************************************************************

        RSEG CODE:CODE:NOROOT(2)
        CODE32

OSCtxSw
        STMFD   SP!,{LR}                ; push PC (lr should be pushed in place of PC)
        STMFD   SP!,{R0-R12,LR}         ; push LR & register file
        MRS     R4,CPSR
        STMFD   SP!,{R4}                ; push current PSR
        MRS     R4,SPSR
        STMFD   SP!,{R4}                ; push current SPSR

        LDR     R4,??OS_TCBCur          ; Get current task's OS_TCB address
        LDR     R5,[R4]
        STR     SP,[R5]                 ; store SP in preempted tasks's TCB

        BL      OSTaskSwHook            ; OSTaskSwHook();

        LDR     R4,??OS_PrioCur         ; OSPrioCur = OSPrioHighRdy
        LDR     R5,??OS_PrioHighRdy
        LDRB    R6,[R5]
        STRB    R6,[R4]

        LDR     R4,??OS_TCBCur          ; Get the current task's OS_TCB address
        LDR     R6,??OS_TCBHighRdy      ; Get highest priority task's OS_TCB address
        LDR     R6,[R6]
        LDR     SP,[R6]                 ; get new task's stack pointer

        STR     R6,[R4]                 ; OSTCBCur = OSTCBHighRdy

        LDMFD   SP!,{R4}                ; pop new task's SPSR
        MSR     SPSR_cxsf,R4
        LDMFD   SP!,{R4}                ; pop new task's PSR
        MSR     CPSR_cxsf,r4
        LDMFD   SP!,{R0-R12,LR,PC}      ; pop new task's R0-R12,LR & PC


;*********************************************************************************************************
;                                  INTERRUPT LEVEL CONTEXT SWITCH
;
; Description:  This code performs a context switch if a higher priority task has been made ready-to-run
;               during an ISR.
;*********************************************************************************************************

        RSEG CODE:CODE:NOROOT(2)
        CODE32

OS_IntCtxSw
        LDR     R0,??OS_IntCtxSwFlag    ; OSIntCtxSwFlag = FALSE
        MOV     R1,#0
        STR     R1,[R0]

        LDMFD   SP!,{R0-R3,R12,LR}      ; Clean up IRQ stack
        STMFD   SP!,{R0-R3}             ; We will use R0-R3 as temporary registers
        MOV     R1,SP
        ADD     SP,SP,#16
        SUB     R2,LR,#4

        MRS     R3,SPSR                 ; Disable interrupts for when we go back to SYS mode
        ORR     R0,R3,#NO_INT
        MSR     SPSR_c,R0

        LDR     R0,=.+8                 ; Switch back to SYS mode (Code below, current location + 2 instructions)
        MOVS    PC,R0                   ; Restore PC and CPSR

                                        ; SAVE OLD TASK'S CONTEXT ONTO OLD TASK'S STACK
        STMFD   SP!,{R2}                ; Push task's PC
        STMFD   SP!,{R4-R12,LR}         ; Push task's LR,R12-R4
        MOV     R4,R1                   ; Move R0-R3 from IRQ stack to SYS stack
        MOV     R5,R3
        LDMFD   R4!,{R0-R3}             ; Load R0-R3 from IRQ stack
        STMFD   SP!,{R0-R3}             ; Push R0-R3
        STMFD   SP!,{R5}                ; Push task's CPSR
        MRS     R4,SPSR
        STMFD   SP!,{R4}                ; Push task's SPSR

        LDR     R4,??OS_TCBCur          ; OSTCBCur->OSTCBStkPtr = SP
        LDR     R5,[R4]
        STR     SP,[R5]

        BL      OSTaskSwHook            ; call Task Switch Hook

        LDR     R4,??OS_PrioCur         ; OSPrioCur = OSPrioHighRdy
        LDR     R5,??OS_PrioHighRdy
        LDRB    R6,[R5]
        STRB    R6,[R4]

        LDR     R4,??OS_TCBCur          ; OSTCBCur = OSTCBHighRdy
        LDR     R6,??OS_TCBHighRdy
        LDR     R6,[R6]
        LDR     SP,[R6]

        STR     R6,[R4]

        LDMFD   SP!,{R4}                ; pop new task's SPSR
        MSR     SPSR_cxsf,R4
        LDMFD   SP!,{R4}                ; pop new task's PSR
        MSR     CPSR_cxsf,R4

        LDMFD   SP!,{R0-R12,LR,PC}      ; pop new task's R0-R12,LR & PC


;*********************************************************************************************************
;                                     POINTERS TO VARIABLES
;*********************************************************************************************************

        DATA

??OS_IntCtxSwFlag:
        DC32    OSIntCtxSwFlag

??OS_PrioCur:
        DC32    OSPrioCur

??OS_PrioHighRdy:
        DC32    OSPrioHighRdy

??OS_Running:
        DC32    OSRunning

??OS_TCBCur:
        DC32    OSTCBCur

??OS_TCBHighRdy:
        DC32    OSTCBHighRdy

        END
