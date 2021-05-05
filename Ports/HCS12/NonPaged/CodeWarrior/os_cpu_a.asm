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
;                                      NON PAGED S12 Specific code
;                                            Codewarrior 4.x
;
; Filename : os_cpu_a.asm
; Version  : V2.93.01
;********************************************************************************************************
; Notes    : THIS FILE *MUST* BE LINKED INTO NON_BANKED MEMORY!
;********************************************************************************************************

NON_BANKED:       section

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
;                              OSCPUSaveSR() for OS_CRITICAL_METHOD #3
;
; Description : This functions implements the OS_CRITICAL_METHOD #3 function to preserve the state of the
;               interrupt disable flag in order to be able to restore it later.
;
; Arguments   : none
;
; Returns     : It is assumed that the return value is placed in the B register as expected by the
;               compiler.
;********************************************************************************************************

OS_CPU_SR_Save:
    tfr    ccr,b                       ; Save CCR in B
    sei                                ; Disable interrupts
    rts

;********************************************************************************************************
;                              OSCPURestoreSR() for OS_CRITICAL_METHOD #3
;
; Description : This functions implements the OS_CRITICAL_METHOD #function to restore the state of the
;               interrupt flag.
;
; Arguments   : os_cpu_sr   is the contents of the CCR to restore.  It is assumed that this 'argument' is
;                           passed in the B register of the CPU by the compiler.
;
; Returns     : None
;********************************************************************************************************

OS_CPU_SR_Restore:
    tfr    b,ccr
    rts

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
;                  OSTCBHighRdy->OSTCBStkPtr +  0  -->  CCR
;                                            +  1       B
;                                            +  2       A
;                                            +  3       X (H)
;                                            +  4       X (L)
;                                            +  5       Y (H)
;                                            +  6       Y (L)
;                                            +  7       PC(H)
;                                            +  8       PC(L)
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task by loading the stack pointer of the
;                         highest priority task into the SP register and execute an RTI instruction.
;********************************************************************************************************

OSStartHighRdy:
    jsr    OSTaskSwHook                ;  4~, Invoke user defined context switch hook

    ldab   #$01                        ;  2~, Indicate that we are multitasking
    stab   OSRunning                   ;  4~

    ldx    OSTCBHighRdy                ;  3~, Point to TCB of highest priority task ready to run
    lds    0,x                         ;  3~, Load SP into 68HC12

    rti                                ;  8~, Run task

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
;                  SP +  0  -->  CCR
;                     +  1       B
;                     +  2       A
;                     +  3       X (H)
;                     +  4       X (L)
;                     +  5       Y (H)
;                     +  6       Y (L)
;                     +  7       PC(H)
;                     +  8       PC(L)
;
;               3) The stack frame of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0  -->  CCR
;                                            +  1       B
;                                            +  2       A
;                                            +  3       X (H)
;                                            +  4       X (L)
;                                            +  5       Y (H)
;                                            +  6       Y (L)
;                                            +  7       PC(H)
;                                            +  8       PC(L)
;********************************************************************************************************


OSCtxSw:
    ldy    OSTCBCur                    ;  3~, OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ;  3~,

    jsr    OSTaskSwHook                ;  4~, Call user task switch hook

    ldx    OSTCBHighRdy                ;  3~, OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur                    ;  3~

    ldab   OSPrioHighRdy               ;  3~, OSPrioCur = OSPrioHighRdy
    stab   OSPrioCur                   ;  3~

    lds    0,x                         ;  3~, Load SP into 68HC12

    rti                                ;  8~, Run task

;********************************************************************************************************
;                                    INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is called by OSIntExit() to perform a context switch to a task that has
;               been made ready-to-run by an ISR.
;
; Arguments   : none
;********************************************************************************************************

OSIntCtxSw:
    jsr    OSTaskSwHook                ;  4~, Call user task switch hook

    ldx    OSTCBHighRdy                ;  3~, OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur                    ;  3~

    ldab   OSPrioHighRdy               ;  3~, OSPrioCur = OSPrioHighRdy
    stab   OSPrioCur                   ;  3~

    lds    0,x                         ;  3~, Load SP into 68HC12

    rti                                ;  8~, Run task

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
    inc    OSIntNesting                ;  4~, Notify uC/OS-II about ISR

    ldab   OSIntNesting                ;  4~, if (OSIntNesting == 1) {
    cmpb   #$01                        ;  2~
    bne    OSTickISR1                  ;  3~

    ldy    OSTCBCur                    ;  3~,     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ;  3~, }

OSTickISR1:
    jsr    OSTickISR_Handler

;   cli                                ;  2~, Enable interrupts to allow interrupt nesting

    jsr    OSIntExit                   ;  6~+, Notify uC/OS-II about end of ISR

    rti                                ;  12~, Return from interrupt, no higher priority tasks ready.
