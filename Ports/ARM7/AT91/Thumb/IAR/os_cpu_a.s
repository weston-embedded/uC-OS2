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
;                                         ARM7 Port, Thumb Mode
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
        EXTERN  OSIntNesting

        EXTERN  OSIntEnter
        EXTERN  OSIntExit

        EXTERN  OSTaskSwHook

        EXTERN  Tmr_TickHandler


        PUBLIC  OSStartHighRdy                   ; Functions declared in this file

        PUBLIC  OSCtxSw
        PUBLIC  OSIntCtxSw
        PUBLIC  OSCPUSaveSR
        PUBLIC  OSCPURestoreSR
        PUBLIC  OSTickISR


NO_INT  EQU     0xC0                             ; Mask used to disable interrupts (Both FIR and IRQ)



;*********************************************************************************************************
;                                       INTERRUPT VECTORS
;*********************************************************************************************************


        COMMON  INTVEC:CODE:ROOT(2)
        CODE32
        ORG     0x18
        LDR     PC,[PC,#-0x0F20]                 ; Vector to the proper ISR for the AT91 using the AIC.

        ORG     0x1C
        LDR     PC,[PC,#-0x0F20]


;*********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; Note : OSStartHighRdy() MUST:
;           a) Call OSTaskSwHook() then,
;           b) Set OSRunning to TRUE,
;           c) Switch to the highest priority task.
;*********************************************************************************************************

        RSEG    NEARFUNC_A:CODE:NOROOT(2)
        CODE32

OSStartHighRdy:
        LDR     R4,??OSTaskSwHook           ; OSTaskSwHook()
        MOV     LR,PC
        BX      R4

        LDR     R4,??OS_Running             ; OSRunning = TRUE
        MOV     R5,#1
        STRB    R5,[R4]

        LDR     R4,??OS_TCBHighRdy          ; SP = OSTCBHighRdy->OSTCBStkPtr
        LDR     R4,[R4]
        LDR     SP,[R4]

                                            ; ---- RESTORE CONTEXT OF HIGH PRIORITY TASK ----
        LDMFD   SP!,{R4}                    ; Pop new task's SPSR
        MSR     SPSR_cxsf,R4                ;    T-bit should be 1 indicating Task will run in Thumb mode

        LDMFD   SP!,{R0-R12,LR,PC}^         ; Pop new task's R0-R12, LR, PC and copy SPSR into CPSR


;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From task level)
;
; Note(s): 1) Upon entry:
;             OSTCBCur      points to the OS_TCB of the task to suspend
;             OSTCBHighRdy  points to the OS_TCB of the task to resume
;          2) It is assumed that OSCtxSw() is called from Thumb code and thus bit 0 of the LR register is
;             set to 1.
;          3) We need to save the context of the current task as if the task was interrupted and all the
;             registers were saved on the stack of that task.  For that, the SPSR T-bit must be set and the
;             LR must point at the return PC of the Thumb task.  Upon calling OSCtxSw(), the LR contains PC+1
;             (indicating that the caller is in Thumb mode).  We don't need that information since we will
;             alter the SPSR to reflect that the caller is a Thumb mode function.
;*********************************************************************************************************

OSCtxSw:
                                            ; ---- SAVE CONTEXT OF CURRENT TASK ----
        BIC     LR,LR,#1                    ; Correct return address
        STMFD   SP!,{LR}                    ; Push return PC

        ADD     LR,LR,#1
        STMFD   SP!,{LR}                    ; Push original LR

        STMFD   SP!,{R0-R12}                ; Push remaining registers

        MRS     R4,CPSR                     ; Push the SPSR (a copy of the CPSR but with the T-bit set)
        ORR     R4,R4,#0x20
        STMFD   SP!,{R4}

        LDR     R4,??OS_TCBCur              ; OSTCBCur->OSTCBStkPtr = SP
        LDR     R5,[r4]
        STR     SP,[r5]

        LDR     R6,??OSTaskSwHook           ; OSTaskSwHook()
        MOV     LR,PC
        BX      R6

        LDR     R4,??OS_PrioCur             ; OSPrioCur = OSPrioHighRdy
        LDR     R5,??OS_PrioHighRdy
        LDRB    R5,[R5]
        STRB    R5,[R4]

        LDR     R6,??OS_TCBHighRdy          ; OSTCBCur = OSTCBHighRdy
        LDR     R4,??OS_TCBCur
        LDR     R6,[R6]
        STR     R6,[R4]

        LDR     SP,[R6]                     ; SP = OSTCBHighRdy->OSTCBStkPtr

                                            ; ---- RESTORE CONTEXT OF HIGH PRIORITY TASK ----
        LDMFD   SP!,{R4}                    ; Pop new task's SPSR
        MSR     SPSR_cxsf,R4

        LDMFD   SP!,{R0-R12,LR,PC}^         ; Pop new task's R0-R12,LR and PC


;*********************************************************************************************************
;                                            IRQ HANDLER
;
; Description:  This handles all the IRQs
;*********************************************************************************************************

OSTickISR:
                                            ; ---- SAVE CONTEXT OF CURRENT TASK ----
                                            ; IRQ Mode:
        STMFD   SP!,{R0-R3,R12,LR}          ;   Push working registers on the IRQ stack

        MOV     R1,SP                       ;   Save IRQ SP
        ADD     SP,SP,#(6*4)                ;   Clean up IRQ stack

        SUB     R2,LR,#4                    ;   Correct return address and save

        MRS     R3,SPSR                     ;   Save SPSR of interrupted task

        MOV     R0,#0xD3                    ;   Disable interrupts for when we go back to SVC mode
        MSR     SPSR_c,R0                   ;   Prepare to switch to SVC mode

        LDR     R0,=.+8                     ;   Setup PC for SVC mode code (see below), current location + 2 instructions
        MOVS    PC,R0                       ;   Restore PC and CPSR

                                            ; SVC Mode:
        STMFD   SP!,{R2}                    ;   Push task's returns address
        STMFD   SP!,{LR}                    ;   Push task's LR
        STMFD   SP!,{R4-R12}                ;   Push task's R12-R4
        MOV     R4,R1                       ;   Move R0-R3 from IRQ stack to SVC stack
        MOV     R5,R3
        LDMFD   R4!,{R0-R3}                 ;   Load R0-R3 from IRQ stack
        STMFD   SP!,{R0-R3}                 ;   Push R0-R3
        STMFD   SP!,{R5}                    ;   Push task's SPSR

                                            ; ---- HANDLE NESTING COUNTER ----
        LDR     R0,??OSIntNesting           ; OSIntNesting++;
        LDRB    R1,[R0]
        ADD     R1,R1,#1
        STRB    R1,[R0]

        CMP     R1,#1                       ; if (OSIntNesting == 1) {
        BNE     OSTickISR_1

        LDR     R4,??OS_TCBCur              ;     OSTCBCur->OSTCBStkPtr = SP
        LDR     R5,[R4]
        STR     SP,[R5]

OSTickISR_1:                          ; }

        LDR     R3,??Tmr_TickHandler        ; Handle timer interrupt (see BSP.C)
        MOV     LR,PC
        BX      R3

                                            ; ---- EXIT INTERRUPT ----
        LDR     R3,??OSIntExit              ; OSIntExit()
        MOV     LR,PC
        BX      R3
                                            ; ---- RESTORE CONTEXT OF HIGH PRIORITY TASK ----
        LDMFD   SP!,{R4}                    ; Pop new task's SPSR
        MSR     SPSR_cxsf,R4

        LDMFD   SP!,{R0-R12,LR,PC}^         ; Pop new task's R0-R12,LR and PC


;*********************************************************************************************************
;                                  INTERRUPT LEVEL CONTEXT SWITCH
;
; Description:  This code performs a context switch if a higher priority task has been made ready-to-run
;               during an ISR.
;*********************************************************************************************************

OSIntCtxSw:
        LDR     R6,??OSTaskSwHook           ; OSTaskSwHook()
        MOV     LR,PC
        BX      R6

        LDR     R4,??OS_PrioCur             ; OSPrioCur = OSPrioHighRdy
        LDR     R5,??OS_PrioHighRdy
        LDRB    R5,[R5]
        STRB    R5,[R4]

        LDR     R6,??OS_TCBHighRdy          ; OSTCBCur = OSTCBHighRdy
        LDR     R4,??OS_TCBCur
        LDR     R6,[R6]
        STR     R6,[R4]

        LDR     SP,[R6]                     ; SP = OSTCBHighRdy->OSTCBStkPtr

                                            ; ---- RESTORE CONTEXT OF HIGH PRIORITY TASK ----
        LDMFD   SP!,{R4}                    ; Pop new task's SPSR
        MSR     SPSR_cxsf,R4

        LDMFD   SP!,{R0-R12,LR,PC}^         ; Pop new task's R0-R12,LR and return from IRQ (SPSR copied to CPSR)


;*********************************************************************************************************
;                                   CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
;              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
;              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
;              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
;              into the CPU's status register.
;
; Prototypes :     OS_CPU_SR  OSCPUSaveSR(void);
;                  void       OSCPURestoreSR(OS_CPU_SR cpu_sr);
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
;                     OS_ENTER_CRITICAL();             /* cpu_sr = OSCPUSaveSR();                  */
;                          :
;                          :
;                     OS_EXIT_CRITICAL();              /* OSCPURestoreSR(cpu_sr);                  */
;                          :
;                          :
;                 }
;
;              2) OSCPUSaveSR() is implemented as recommended by Atmel's application note:
;
;                    "Disabling Interrupts at Processor Level"
;*********************************************************************************************************

OSCPUSaveSR:
        MRS     R0,CPSR                     ; Set IRQ and FIQ bits in CPSR to disable all interrupts
        ORR     R1,R0,#NO_INT
        MSR     CPSR_c,R1
        MRS     R1,CPSR                     ; Confirm that CPSR contains the proper interrupt disable flags
        AND     R1,R1,#NO_INT
        CMP     R1,#NO_INT
        BNE     OSCPUSaveSR                 ; Not properly disabled (try again)
        BX      LR                          ; Disabled, return the original CPSR contents in R0


OSCPURestoreSR:
        MSR     CPSR_c,R0
        BX      LR


;*********************************************************************************************************
;                                     POINTERS TO VARIABLES
;*********************************************************************************************************

        DATA

??OSIntEnter:
        DC32    OSIntEnter      | 0x01

??OSIntExit:
        DC32    OSIntExit       | 0x01

??Tmr_TickHandler:
        DC32    Tmr_TickHandler | 0x01

??OSTaskSwHook:
        DC32    OSTaskSwHook

??OSIntNesting:
        DC32    OSIntNesting

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
