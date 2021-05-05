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
;                                       PAGED MC908 Specific code
;                                             (Codewarrior)
;
; Filename : os_cpu_a.s
; Version  : V2.93.01
;********************************************************************************************************
; Notes        : THIS FILE *MUST* BE LINKED INTO NON_BANKED MEMORY!
;********************************************************************************************************

NON_BANKED:       section

;********************************************************************************************************
;                                           I/O PORT ADDRESSES
;********************************************************************************************************

PPAGE:            equ    $0078         ; Addres of PPAGE register (Using MC9S08QE128 as a reference)

;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************

    xdef   OS_CPU_SR_Save
    xdef   OS_CPU_SR_Restore
    xdef   OSStartHighRdy
    xdef   OSCtxSw
    xdef   OSIntCtxSw
    xdef   Tmr_TickISR

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
    xref   Tmr_TickISR_Handler
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
; Note(s)     : 1) It's assumed that the compiler uses the A register to pass a single 8-bit argument
;                  to and from an assembly language function.
;********************************************************************************************************

OS_CPU_SR_Save:
    tpa                               ; Transfer the CCR to A.
    sei                               ; Disable interrupts
    rtc                               ; Return to caller with A containing the previous CCR

OS_CPU_SR_Restore:
    tap                               ; Restore the CCR from the function argument stored in A
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
;                  OSTCBHighRdy->OSTCBStkPtr +  0       H        (Stacked and unstacked by the OS port)
;                                            +  1       PPAGE    (Stacked and unstacked by the OS port)
;                                            +  2       CCR      (Stacked and unstacked by the MC9S08)
;                                            +  3       A        (Stacked and unstacked by the MC9S08)
;                                            +  6       X        (Stacked and unstacked by the MC9S08)
;                                            +  7       PC(H)    (Stacked and unstacked by the MC9S08)
;                                            +  8       PC(L)    (Stacked and unstacked by the MC9S08)
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task by loading the stack pointer of the
;                         highest priority task into the SP register and execute an RTI instruction.
;********************************************************************************************************

OSStartHighRdy:
    call   OSTaskSwHook               ; Invoke user defined context switch hook

    lda    #$01                       ; Indicate that we are multitasking
    sta    OSRunning                  ; Store the running state back to memory

    ldhx   OSTCBHighRdy               ; Point to TCB of highest priority task ready to run
    ldhx   0, x                       ; Dereference OSTCBHighRdy to obtain the stack address
    txs                               ; Transfer the tasks stack pointer back to the stack pointer register

    pulh                              ; Restore H

    pula                              ; Get value of PPAGE register
    sta    PPAGE                      ; Store into CPU's PPAGE register

    rti                               ; Run task

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
;                  SP +  0       PC(H)
;                     +  1       PC(L)
;
;               3) The stack frame of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0       H        (Stacked and unstacked by the OS port)
;                                            +  1       PPAGE    (Stacked and unstacked by the OS port)
;                                            +  2       CCR      (Stacked and unstacked by the MC9S08)
;                                            +  3       A        (Stacked and unstacked by the MC9S08)
;                                            +  6       X        (Stacked and unstacked by the MC9S08)
;                                            +  7       PC(H)    (Stacked and unstacked by the MC9S08)
;                                            +  8       PC(L)    (Stacked and unstacked by the MC9S08)
;********************************************************************************************************

OSCtxSw:
    lda    PPAGE                      ; Get current value of PPAGE register
    psha                              ; Push PPAGE register onto current task's stack

    pshh                              ; Push H on to the stack

    tsx                               ; Copy the stack pointer into HX
    pshx                              ; Store the stack pointer on the stack
    pshh
    ldhx   OSTCBCur                   ; Obtain OSTCBCur in HX

    pula                              ; OSTCBCur->OSTCBStkPtr = Stack Pointer
    sta    0, x
    pula
    sta    1, x

    call   OSTaskSwHook               ; Call user task switch hook

    lda    OSPrioHighRdy              ; OSPrioCur = OSPrioHighRdy
    sta    OSPrioCur

    ldhx   OSTCBHighRdy               ; OSTCBCur  = OSTCBHighRdy
    sthx   OSTCBCur
    ldhx   0, x                       ; Dereference OSTCBHighRdy to obtain the stack address
    txs                               ; Copy the next tasks stack pointer into the stack pointer register

    pulh                              ; Restore H

    pula                              ; Get value of PPAGE register
    sta    PPAGE                      ; Store into CPU's PPAGE register

    rti                               ; Run task

;********************************************************************************************************
;                                    INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is called by OSIntExit() to perform a context switch to a task that has
;               been made ready-to-run by an ISR. The H and PPAGE registers of the preempted task have
;               been stacked during the start of the ISR that is currently running.
;
; Arguments   : none
;
; Notes       : 1) The stack frame of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0       H        (Stacked and unstacked by the ISR entry code)
;                                            +  1       PPAGE    (Stacked and unstacked by the ISR entry code)
;                                            +  2       CCR      (Stacked and unstacked by the MC9S08)
;                                            +  3       A        (Stacked and unstacked by the MC9S08)
;                                            +  6       X        (Stacked and unstacked by the MC9S08)
;                                            +  7       PC(H)    (Stacked and unstacked by the MC9S08)
;                                            +  8       PC(L)    (Stacked and unstacked by the MC9S08)
;********************************************************************************************************

OSIntCtxSw:
    call   OSTaskSwHook               ; Call user task switch hook

    lda    OSPrioHighRdy              ; OSPrioCur = OSPrioHighRdy
    sta    OSPrioCur

    ldhx   OSTCBHighRdy               ; OSTCBCur  = OSTCBHighRdy
    sthx   OSTCBCur
    ldhx   0, x                       ; Dereference OSTCBHighRdy to obtain the stack address
    txs                               ; Copy the next tasks stack pointer into the stack pointer register

    pulh                              ; Restore H

    pula                              ; Get value of PPAGE register
    sta    PPAGE                      ; Store into CPU's PPAGE register

    rti                               ; Run task

;********************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.  You
;               must setup the MC9S08's interrupt vector table so that an OUTPUT COMPARE interrupt
;               vectors to this function.
;
; Arguments   : none
;
; Notes       : 1) The 'tick ISR' assumes the we are using the Output Compare specified by OS_TICK_OC
;                 (see APP_CFG.H and this file) to generate a tick that occurs every (1 / OS_TICKS_PER_SECOND)
;                  seconds.
;
;               2) All USER interrupts should be modeled EXACTLY like this where the only
;                  line to be modified is the call to your ISR_Handler and perhaps the call to
;                  the label name OSTickISR1.
;********************************************************************************************************

Tmr_TickISR:
    lda    PPAGE                      ; Get current value of PPAGE register
    psha                              ; Push PPAGE register onto current task's stack

    pshh                              ; Push the H register on to the stack

    lda    OSIntNesting               ;
    add    #1                         ; Notify uC/OS-II about the interrupt
    sta    OSIntNesting

    cmpa   #$01                       ;
    bne    Tmr_TickISR1               ;

    tsx                               ;     Copy the stack pointer into HX
    pshx                              ;     Store the stack pointer on the stack
    pshh
    ldhx   OSTCBCur                   ;     Obtain OSTCBCur in HX

    pula                              ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sta    0, x
    pula
    sta    1, x                       ; }

Tmr_TickISR1:
    call   Tmr_TickISR_Handler

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR

    pulh                              ; Restore the H register

    pula                              ; Get value of PPAGE register
    sta    PPAGE                      ; Store into CPU's PPAGE register

    rti                               ; Return from interrupt, no higher priority tasks ready.
