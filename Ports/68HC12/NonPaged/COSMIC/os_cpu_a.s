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
;                                         68HC12 Specific code
;
; Filename : os_cpu_a.s
; Version  : V2.93.01
;********************************************************************************************************


;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************

    xdef   _OSStartHighRdy
    xdef   _OSCtxSw
    xdef   _OSIntCtxSw
    xdef   _OS_CPU_SR_Save
    xdef   _OS_CPU_SR_Restore
    xdef   _OSTickISR

;********************************************************************************************************
;                                         EXTERNAL DECLARATIONS
;********************************************************************************************************

    xref   _OSIntExit
    xref   _OSIntNesting
    xref   _OSPrioCur
    xref   _OSPrioHighRdy
    xref   _OSRunning
    xref   _OSTaskSwHook
    xref   _OSTCBCur
    xref   _OSTCBHighRdy
    xref   _OSTickISR_Handler

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

_OSStartHighRdy:
    jsr    _OSTaskSwHook               ;  4~, Invoke user defined context switch hook

    ldab   #$01                        ;  2~, Indicate that we are multitasking
    stab   _OSRunning                  ;  4~

    ldx    _OSTCBHighRdy               ;  3~, Point to TCB of highest priority task ready to run
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
;                  SP +  0 -->   PC(H)
;                     +  1       PC(L)
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

_OSCtxSw:
    pshy                               ;  2~, Save context of 'old' task
    pshx                               ;  2~
    psha                               ;  2~
    pshb                               ;  2~
    pshc                               ;  2~

    ldy    _OSTCBCur                   ;  3~, OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ;  3~,

    jsr    _OSTaskSwHook               ;  4~, Call user task switch hook

    ldx    _OSTCBHighRdy               ;  3~, OSTCBCur  = OSTCBHighRdy
    stx    _OSTCBCur                   ;  3~

    ldab   _OSPrioHighRdy              ;  3~, OSPrioCur = OSPrioHighRdy
    stab   _OSPrioCur                  ;  3~

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

_OSIntCtxSw:
    jsr    _OSTaskSwHook               ;  4~, Call user task switch hook

    ldx    _OSTCBHighRdy               ;  3~, OSTCBCur  = OSTCBHighRdy
    stx    _OSTCBCur                   ;  3~

    ldab   _OSPrioHighRdy              ;  3~, OSPrioCur = OSPrioHighRdy
    stab   _OSPrioCur                  ;  3~

    lds    0,x                         ;  3~, Load SP into 68HC12

    rti                                ;  8~, Run task


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

_OS_CPU_SR_Save:
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

_OS_CPU_SR_Restore:
    tfr    b,ccr
    rts


;********************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.  You
;               must setup the 68HC12's interrupt vector table so that an OUTPUT COMPARE interrupt
;               vectors to this function.
;
; Arguments   : none
;********************************************************************************************************

_OSTickISR:
    inc    _OSIntNesting               ;  4~, Notify uC/OS-II about ISR

    ldab   _OSIntNesting               ;  4~, if (OSIntNesting == 1) {
    cmpb   #$01                        ;  2~
    bne    _OSTickISR1                 ;  3~

    ldy    _OSTCBCur                   ;  3~,     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ;  3~, }

_OSTickISR1:
    jsr    _OSTickISR_Handler          ;  4~, OSTickISR_Handler();

    jsr    _OSIntExit                  ; 6~+, Notify uC/OS-II about end of ISR

    rti                                ; 12~, Return from interrupt, no higher priority tasks ready.
