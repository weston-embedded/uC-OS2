@********************************************************************************************************
@                                              uC/OS-II
@                                        The Real-Time Kernel
@
@                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
@
@                                 SPDX-License-Identifier: APACHE-2.0
@
@               This software is subject to an open source license and is distributed by
@                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
@                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
@
@********************************************************************************************************

@********************************************************************************************************
@
@                                               ARM9 Port
@                                             Sharp LH79520
@                                            GNU C Compiler
@
@ Filename : os_cpu_a.s
@ Version  : V2.93.01
@********************************************************************************************************

        .extern  OSRunning                      @ .external references
        .extern  OSTCBCur
        .extern  OSTCBHighRdy
        .extern  OSPrioCur
        .extern  OSPrioHighRdy
        .extern  OSIntCtxSwFlag

        .extern  OSIntEnter
        .extern  OSIntExit

        .extern  OSTaskSwHook

        .extern  Tmr_TickHandler


        .global  OSStartHighRdy                 @ Functions declared in this file

        .global  OSCtxSw
        .global  OS_IntCtxSw

        .global  OS_CPU_SaveSR
        .global  OS_CPU_RestoreSR
        .global  OS_CPU_Tick_ISR

        .equ NO_INT, 0xC0                       @ Mask used to disable interrupts (Both FIQ and IRQ)

@*********************************************************************************************************
@                                          START MULTITASKING
@                                       void OSStartHighRdy(void)
@
@ Note : OSStartHighRdy() MUST:
@           a) Call OSTaskSwHook() then,
@           b) Set OSRunning to TRUE,
@           c) Switch to the highest priority task.
@*********************************************************************************************************

        .code 32

OSStartHighRdy:

        BL      OSTaskSwHook                    @ Call user defined task switch hook

        LDR     R4,=OSRunning                   @ OSRunning = TRUE
        MOV     R5,#1
        STRB    R5,[R4]

        LDR     R4,=OSTCBHighRdy                @ Get highest priority task TCB address
        LDR     R4,[R4]                         @ get stack pointer
        LDR     SP,[R4]                         @ switch to the new stack

        LDMFD   SP!,{R4}                        @ pop new task's SPSR
        MSR     SPSR_cxsf,R4
        LDMFD   SP!,{R4}                        @ pop new task's CPSR
        MSR     CPSR_cxsf,R4
        LDMFD   SP!,{R0-R12,LR,PC}              @ pop new task's R0-R12,LR & PC

@*********************************************************************************************************
@                                PERFORM A CONTEXT SWITCH (From task level)
@
@ Note(s): Upon entry:
@              OSTCBCur      points to the OS_TCB of the task to suspend
@              OSTCBHighRdy  points to the OS_TCB of the task to resume
@*********************************************************************************************************

        .code 32

OSCtxSw:
        STMFD   SP!,{LR}                        @ push pc (lr should be pushed in place of PC)
        STMFD   SP!,{R0-R12,LR}                 @ push lr & register file
        MRS     R4,CPSR
        STMFD   SP!,{R4}                        @ push current PSR
        MRS     R4,SPSR
        STMFD   SP!,{R4}                        @ push current SPSR

        LDR     R4,=OSPrioCur                   @ OSPrioCur = OSPrioHighRdy
        LDR     R5,=OSPrioHighRdy
        LDRB    R6,[r5]
        STRB    R6,[r4]

        LDR     R4,=OSTCBCur                    @ Get current task's OS_TCB address
        LDR     R5,[r4]
        STR     SP,[r5]                         @ store sp in preempted tasks's TCB

        BL      OSTaskSwHook                    @ call Task Switch Hook

        LDR     R6,=OSTCBHighRdy                @ Get highest priority task's OS_TCB address
        LDR     R6,[R6]
        LDR     SP,[R6]                         @ get new task's stack pointer

        STR     R6,[R4]                         @ OSTCBCur = OSTCBHighRdy

        LDMFD   SP!,{R4}                        @ pop new task's SPSR
        MSR     SPSR_cxsf,R4
        LDMFD   SP!,{R4}                        @ pop new task's PSR
        MSR     CPSR_cxsf,r4
        LDMFD   SP!,{R0-R12,LR,PC}              @ pop new task's R0-R12,LR & PC

@*********************************************************************************************************
@                                   CRITICAL SECTION METHOD 3 FUNCTIONS
@
@ Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
@              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
@              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
@              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
@              into the CPU's status register.
@
@ Prototypes :     OS_CPU_SR  OS_CPU_SaveSR(void)@
@                  void       OS_CPU_RestoreSR(OS_CPU_SR cpu_sr)@
@
@
@ Note(s)    : 1) These functions are used in general like this:
@
@                 void Task (void *p_arg)
@                 {
@                 #if OS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register */
@                     OS_CPU_SR  cpu_sr@
@                 #endif
@
@                          :
@                          :
@                     OS_ENTER_CRITICAL()@             /* cpu_sr = OS_CPU_SaveSR()@                */
@                          :
@                          :
@                     OS_EXIT_CRITICAL()@              /* OS_CPU_RestoreSR(cpu_sr)@                */
@                          :
@                          :
@                 }
@
@              2) OS_CPU_SaveSR() is implemented as recommended by Atmel's application note:
@
@                    "Disabling Interrupts at Processor Level"
@*********************************************************************************************************

OS_CPU_SaveSR:
        MRS     R0,CPSR                         @ Set IRQ and FIQ bits in CPSR to disable all interrupts
        ORR     R1,R0,#NO_INT
        MSR     CPSR_c,R1
        MRS     R1,CPSR                         @ Confirm that CPSR contains the proper interrupt disable flags
        AND     R1,R1,#NO_INT
        CMP     R1,#NO_INT
        BNE     OS_CPU_SaveSR                   @ Not properly disabled (try again)
        MOV     PC,LR                           @ Disabled, return the original CPSR contents in R0

OS_CPU_RestoreSR:
        MSR     CPSR_c,R0
        MOV     PC,LR

@*********************************************************************************************************
@                                            TICK HANDLER
@
@ Description:  This handles all the Timer #0 interrupt which is used to generate the uC/OS-II tick.
@*********************************************************************************************************

OS_CPU_Tick_ISR:

        STMFD   SP!,{R0-R3,R12,LR}

        BL      OSIntEnter                      @ Indicate beginning of ISR
        BL      Tmr_TickHandler                 @ Handle interrupt (see BSP.C)
        BL      OSIntExit                       @ Indicate end of ISR

        LDR     R0,=OSIntCtxSwFlag              @ See if we need to do a context switch
        LDR     R1,[R0]
        CMP     R1,#1
        BEQ     OS_IntCtxSw                     @ Yes, Switch to Higher Priority Task

        LDMFD   SP!,{R0-R3,R12,LR}              @ No,  Restore registers of interrupted task's stack
        SUBS    PC,LR,#4                        @ Return from IRQ

@*********************************************************************************************************
@                                  INTERRUPT LEVEL CONTEXT SWITCH
@
@ Description:  This code performs a context switch if a higher priority task has been made ready-to-run
@               during an ISR.
@*********************************************************************************************************

OS_IntCtxSw:
        LDR     R0,=OSIntCtxSwFlag              @ OSIntCtxSwFlag = FALSE
        MOV     R1,#0
        STR     R1,[R0]

        LDMFD   SP!,{R0-R3,R12,LR}              @ Clean up IRQ stack
        STMFD   SP!,{R0-R3}                     @ We will use R0-R3 as temporary registers
        MOV     R1,SP
        ADD     SP,SP,#16
        SUB     R2,LR,#4

        MRS     R3,SPSR                         @ Disable interrupts for when we go back to SVC mode
        ORR     R0,R3,#NO_INT
        MSR     SPSR_c,R0

        LDR     R0,=.+8                         @ Switch back to SVC mode (Code below, current location + 2 instructions)
        MOVS    PC,R0                           @ Restore PC and CPSR

                                                @ SAVE OLD TASK'S CONTEXT ONTO OLD TASK'S STACK
        STMFD   SP!,{R2}                        @ Push task's PC
        STMFD   SP!,{R4-R12,LR}                 @ Push task's LR,R12-R4
        MOV     R4,R1                           @ Move R0-R3 from IRQ stack to SVC stack
        MOV     R5,R3
        LDMFD   R4!,{R0-R3}                     @ Load R0-R3 from IRQ stack
        STMFD   SP!,{R0-R3}                     @ Push R0-R3
        STMFD   SP!,{R5}                        @ Push task's CPSR
        MRS     R4,SPSR
        STMFD   SP!,{R4}                        @ Push task's SPSR

        LDR     R4,=OSPrioCur                   @ OSPrioCur = OSPrioHighRdy
        LDR     R5,=OSPrioHighRdy
        LDRB    R5,[R5]
        STRB    R5,[R4]

        LDR     R4,=OSTCBCur                    @ Get current task's OS_TCB address
        LDR     R5,[R4]
        STR     SP,[R5]                         @ store sp in preempted tasks's TCB

        BL      OSTaskSwHook                    @ call Task Switch Hook

        LDR     R6,=OSTCBHighRdy                @ Get highest priority task's OS_TCB address
        LDR     R6,[R6]
        LDR     SP,[R6]                         @ get new task's stack pointer

        STR     R6,[R4]                         @ OSTCBCur = OSTCBHighRdy

        LDMFD   SP!,{R4}                        @ pop new task's SPSR
        MSR     SPSR_cxsf,R4
        LDMFD   SP!,{R4}                        @ pop new task's PSR
        MSR     CPSR_cxsf,R4

        LDMFD   SP!,{R0-R12,LR,PC}              @ pop new task's R0-R12,LR & PC

        .ltorg
