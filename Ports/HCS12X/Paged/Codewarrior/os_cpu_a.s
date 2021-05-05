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
;                                       PAGED S12X Specific code
;                                             (CODEWARRIOR)
;
; Filename : os_cpu_a.s
; Version  : V2.93.01
;********************************************************************************************************
; Notes    : THIS FILE *MUST* BE LINKED INTO NON_BANKED MEMORY!
;********************************************************************************************************

NON_BANKED:       section

;********************************************************************************************************
;                                           I/O PORT ADDRESSES
;********************************************************************************************************

PPAGE:            equ    $0015         ; Addres of PPAGE register (assuming MC9S12XEP100 part)
RPAGE:            equ    $0016         ; Addres of RPAGE register (assuming MC9S12XEP100 part)
EPAGE:            equ    $0017         ; Addres of EPAGE register (assuming MC9S12XEP100 part)
GPAGE:            equ    $0010         ; Addres of GPAGE register (assuming MC9S12XEP100 part)

;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************

    xdef   OS_CPU_SR_Save
    xdef   OS_CPU_SR_Restore
    xdef   OSStartHighRdy
    xdef   OSCtxSw
    xdef   OSIntCtxSw
    xdef   OSTickISR

;********************************************************************************************************
;                                         EXTERNAL DECLARATIONS
;********************************************************************************************************

    xref   OSIntExit
    xref   OSIntNesting
    xref   OSPrioCur
    xref   OSPrioHighRdy
    xref   OSRunning
    xref   OSTaskSwHook
    xref   OSTCBCur
    xref   OSTCBHighRdy
    xref   OSTickISR_Handler
    xref   OSTimeTick

;********************************************************************************************************
;                                  SAVE THE CCR AND DISABLE INTERRUPTS
;                                                  &
;                                              RESTORE CCR
;
; Description : These function implements OS_CRITICAL_METHOD #3
;
; Arguments   : The function prototypes for the two functions are:
;               1) OS_CPU_SR  OSCPUSaveSR(void)
;                             where OS_CPU_SR is the contents of the CCR register prior to disabling
;                             interrupts.
;               2) void       OSCPURestoreSR(OS_CPU_SR os_cpu_sr);
;                             'os_cpu_sr' the the value of the CCR to restore.
;
; Note(s)     : 1) It's assumed that the compiler uses the D register to pass a single 16-bit argument
;                  to and from an assembly language function.
;********************************************************************************************************

OS_CPU_SR_Save:
    tfr  ccrw, d                       ; It's assumed that the 16-bit return value is in register D
    sei                                ; Disable interrupts
    rtc                                ; Return to caller with D containing the previous CCR

OS_CPU_SR_Restore:
    tfr  d,ccrw                        ; D contains the CCR word value to restore, move to CCRW
    rtc

;********************************************************************************************************
;                               START HIGHEST PRIORITY TASK READY-TO-RUN
;
; Description : This function is called by OSStart() to start the highest priority task that was created
;               by your application before calling OSStart().
;
; Arguments   : none
;
; Note(s)     : 1) The stack frame is assumed to look as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0  -->  gPAGE
;                                            +  1       ePAGE
;                                            +  2       rPAGE
;                                            +  3       pPAGE
;                                            +  1       CCRW
;                                            +  2       B
;                                            +  3       A
;                                            +  4       X (H)
;                                            +  5       X (L)
;                                            +  6       Y (H)
;                                            +  7       Y (L)
;                                            +  8       PC(H)
;                                            +  9       PC(L)
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task by loading the stack pointer of the
;                         highest priority task into the SP register and execute an RTI instruction.
;********************************************************************************************************

OSStartHighRdy:
    call   OSTaskSwHook                ; Invoke user defined context switch hook

    ldab   #$01                        ; Indicate that we are multitasking
    stab   OSRunning

    ldx    OSTCBHighRdy                ; Point to TCB of highest priority task ready to run
    lds    0,x                         ; Load SP into 68HC12

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti                                ; Run task

;********************************************************************************************************
;                                       TASK LEVEL CONTEXT SWITCH
;
; Description : This function is called when a task makes a higher priority task ready-to-run.
;
; Arguments   : none
;
; Note(s)     : 1) Upon entry,
;                  OSTCBCur     points to the OS_TCB of the task to suspend
;                  OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows:
;
;                  SP            CCR
;                     +  2       B
;                     +  3       A
;                     +  4       X (H)
;                     +  5       X (L)
;                     +  6       Y (H)
;                     +  7       Y (L)
;                     +  8       PC(H)
;                     +  9       PC(L)
;
;               3) The stack frame of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0  -->  gPAGE
;                                            +  1       ePAGE
;                                            +  2       rPAGE
;                                            +  3       pPAGE
;                                            +  4       CCR
;                                            +  6       B
;                                            +  7       A
;                                            +  8       X (H)
;                                            +  9       X (L)
;                                            + 10       Y (H)
;                                            + 11       Y (L)
;                                            + 12       PC(H)
;                                            + 13       PC(L)
;********************************************************************************************************

OSCtxSw:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    ldy    OSTCBCur                    ; OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y

    call   OSTaskSwHook                ; Call user task switch hook

    ldx    OSTCBHighRdy                ; OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur

    ldab   OSPrioHighRdy               ; OSPrioCur = OSPrioHighRdy
    stab   OSPrioCur

    lds    0,x                         ; Load SP into 68HC12

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti                                ; Run task

;********************************************************************************************************
;                                    INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is called by OSIntExit() to perform a context switch to a task that has
;               been made ready-to-run by an ISR. The GPAGE, EPAGE, RPAGE and PPAGE CPU registers of the
;               preempted task have already been stacked during the start of the ISR that is currently
;               running.
;
; Arguments   : none
;********************************************************************************************************

OSIntCtxSw:
    call   OSTaskSwHook                ; Call user task switch hook

    ldx    OSTCBHighRdy                ; OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur

    ldab   OSPrioHighRdy               ; OSPrioCur = OSPrioHighRdy
    stab   OSPrioCur

    lds    0,x                         ; Load the SP of the next task

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti                                ; Run task

;********************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.  You
;               must setup the S12XE's interrupt vector table so that an OUTPUT COMPARE interrupt
;               vectors to this function.
;
; Arguments   : none
;
; Notes       :  1) The 'tick ISR' assumes the we are using the Output Compare specified by OS_TICK_OC
;                   (see APP_CFG.H and this file) to generate a tick that occurs every OS_TICK_OC_CNTS
;                   (see APP_CFG.H) which corresponds to the number of FRT (Free Running Timer)
;                   counts to the next interrupt.
;
;                2) All USER interrupts should be modeled EXACTLY like this where the only
;                   line to be modified is the call to your ISR_Handler and perhaps the call to
;                   the label name OSTickISR1.
;********************************************************************************************************

OSTickISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    OSTickISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }

OSTickISR1:
    call   OSTickISR_Handler

;   cli                                ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                   ; Notify uC/OS-II about end of ISR

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti                                ; Return from interrupt, no higher priority tasks ready.
