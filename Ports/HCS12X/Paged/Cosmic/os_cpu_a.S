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
;                                       PAGED S12XE Specific code
;                                         (COSMIC C V4.xxxxxx)
;
; Filename : os_cpu_a.S
; Version  : V2.93.01
;********************************************************************************************************

;********************************************************************************************************
;                                         CONFIGURATION CONSTANTS
;********************************************************************************************************

OS_TICK_OC:       equ        7         ; We will use Output Compare #7 to generate tick interrupts
	              xref _OSTickOCCnts
;********************************************************************************************************
;                                           I/O PORT ADDRESSES
;********************************************************************************************************

TFLG1:            equ    $004E         ; I/O port addresses.  Assumes all MC9S12XEP100 I/Os start at 0x0000
TC0:              equ    $0050
TC1:              equ    $0052
TC2:              equ    $0054
TC3:              equ    $0056
TC4:              equ    $0058
TC5:              equ    $005A
TC6:              equ    $005C
TC7:              equ    $005E

PPAGE:            equ    $0015         ; Addres of PPAGE register (assuming MC9S12XEP100 part)
RPAGE:            equ    $0016         ; Addres of RPAGE register (assuming MC9S12XEP100 part)
EPAGE:            equ    $0017         ; Addres of EPAGE register (assuming MC9S12XEP100 part)
GPAGE:            equ    $0010         ; Addres of GPAGE register (assuming MC9S12XEP100 part)

;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************

    xdef   _OSCPUSaveSR
    xdef   _OSCPURestoreSR
    xdef   _OSStartHighRdy
    xdef   _OSCtxSw
    xdef   _OSIntCtxSw
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
    xref   _OSTimeTick

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

_OSCPUSaveSR:
    tfr  ccrw,d                        ; It's assumed that 16-bit return value is in register D
    sei                                ; Disable interrupts
    rts                                ; Return to caller with D containing the previous CCR


_OSCPURestoreSR:
    tfr  d,ccrw                        ; D contains the CCR value to restore, move to CCR
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

    pula                               ;  3~, Get value of PPAGE register
    staa   PPAGE                       ;  3~, Store into CPU's PPAGE register

    pula                               ;  3~, Get value of RPAGE register
    staa   RPAGE                       ;  3~, Store into CPU's RPAGE register

    pula                               ;  3~, Get value of EPAGE register
    staa   EPAGE                       ;  3~, Store into CPU's EPAGE register

    pula                               ;  3~, Get value of GPAGE register
    staa   GPAGE                       ;  3~, Store into CPU's GPAGE register

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

_OSCtxSw:
    ldaa   GPAGE                       ;  3~, Get current value of GPAGE register
    psha                               ;  2~, Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ;  3~, Get current value of EPAGE register
    psha                               ;  2~, Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ;  3~, Get current value of RPAGE register
    psha                               ;  2~, Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ;  3~, Get current value of PPAGE register
    psha                               ;  2~, Push PPAGE register onto current task's stack

    ldy    _OSTCBCur                   ;  3~, OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ;  3~,

    jsr    _OSTaskSwHook               ;  4~, Call user task switch hook

    ldx    _OSTCBHighRdy               ;  3~, OSTCBCur  = OSTCBHighRdy
    stx    _OSTCBCur                   ;  3~

    ldab   _OSPrioHighRdy              ;  3~, OSPrioCur = OSPrioHighRdy
    stab   _OSPrioCur                  ;  3~

    lds    0,x                         ;  3~, Load SP into 68HC12

    pula                               ;  3~, Get value of PPAGE register
    staa   PPAGE                       ;  3~, Store into CPU's PPAGE register

    pula                               ;  3~, Get value of RPAGE register
    staa   RPAGE                       ;  3~, Store into CPU's RPAGE register

    pula                               ;  3~, Get value of EPAGE register
    staa   EPAGE                       ;  3~, Store into CPU's EPAGE register

    pula                               ;  3~, Get value of GPAGE register
    staa   GPAGE                       ;  3~, Store into CPU's GPAGE register

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

    pula                               ;  3~, Get value of PPAGE register
    staa   PPAGE                       ;  3~, Store into CPU's PPAGE register

    pula                               ;  3~, Get value of RPAGE register
    staa   RPAGE                       ;  3~, Store into CPU's RPAGE register

    pula                               ;  3~, Get value of EPAGE register
    staa   EPAGE                       ;  3~, Store into CPU's EPAGE register

    pula                               ;  3~, Get value of GPAGE register
    staa   GPAGE                       ;  3~, Store into CPU's GPAGE register

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
;                2) You must specify which output compare will be used by the tick ISR as follows:
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 0 to use OUTPUT COMPARE #0
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 1 to use OUTPUT COMPARE #1
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 2 to use OUTPUT COMPARE #2
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 3 to use OUTPUT COMPARE #3
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 4 to use OUTPUT COMPARE #4
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 5 to use OUTPUT COMPARE #5
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 6 to use OUTPUT COMPARE #6
;                       Set OS_TICK_OC in APP_CFG.H (AND in this file) to 7 to use OUTPUT COMPARE #7
;
;                3) TFLG1, TC0 ... TC7 are defined in this file.
;********************************************************************************************************

_OSTickISR:
    ldaa   GPAGE                       ;  3~, Get current value of GPAGE register
    psha                               ;  2~, Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ;  3~, Get current value of EPAGE register
    psha                               ;  2~, Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ;  3~, Get current value of RPAGE register
    psha                               ;  2~, Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ;  3~, Get current value of PPAGE register
    psha                               ;  2~, Push PPAGE register onto current task's stack

    inc    _OSIntNesting               ;  4~, Notify uC/OS-II about ISR

    ldab   _OSIntNesting               ;  4~, if (OSIntNesting == 1) {
    cmpb   #$01                        ;  2~
    bne    _OSTickISR1                 ;  3~

    ldy    _OSTCBCur                   ;  3~,     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ;  3~, }

_OSTickISR1:
if OS_TICK_OC == 0
    ldab   #$01                        ;  2~, Clear C0F interrupt flag (bit 0)
    stab   TFLG1                       ;  4~
    ldd    TC0                         ;  5~, Set TC0 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC0                         ;  5~
endif

if OS_TICK_OC == 1
    ldab   #$02                        ;  2~, Clear C1F interrupt flag (bit 1)
    stab   TFLG1                       ;  4~
    ldd    TC1                         ;  5~, Set TC1 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC1                         ;  5~
endif

if OS_TICK_OC == 2
    ldab   #$04                        ;  2~, Clear C2F interrupt flag (bit 2)
    stab   TFLG1                       ;  4~
    ldd    TC2                         ;  5~, Set TC2 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC2                         ;  5~
endif

if OS_TICK_OC == 3
    ldab   #$08                        ;  2~, Clear C3F interrupt flag (bit 3)
    stab   TFLG1                       ;  4~
    ldd    TC3                         ;  5~, Set TC3 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC3                         ;  5~
endif

if OS_TICK_OC == 4
    ldab   #$10                        ;  2~, Clear C4F interrupt flag (bit 4)
    stab   TFLG1                       ;  4~
    ldd    TC4                         ;  5~, Set TC4 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC4                         ;  5~
endif

if OS_TICK_OC == 5
    ldab   #$20                        ;  2~, Clear C5F interrupt flag (bit 5)
    stab   TFLG1                       ;  4~
    ldd    TC5                         ;  5~, Set TC5 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC5                         ;  5~
endif

if OS_TICK_OC == 6
    ldab   #$40                        ;  2~, Clear C6F interrupt flag (bit 6)
    stab   TFLG1                       ;  4~
    ldd    TC6                         ;  5~, Set TC6 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC6                         ;  5~
endif

if OS_TICK_OC == 7
    ldab   #$80                        ;  2~, Clear C7F interrupt flag (bit 7)
    stab   TFLG1                       ;  4~
    ldd    TC7                         ;  5~, Set TC7 to present time + desired counts to next ISR
    addd   _OSTickOCCnts               ;  3~
    std    TC7                         ;  5~
endif

    cli                                ;  2~, Enable interrupts to allow interrupt nesting

    jsr    _OSTimeTick                 ; 6~+, Call uC/OS-II's tick updating function

    jsr    _OSIntExit                  ; 6~+, Notify uC/OS-II about end of ISR

    pula                               ;  3~, Get value of PPAGE register
    staa   PPAGE                       ;  3~, Store into CPU's PPAGE register

    pula                               ;  3~, Get value of RPAGE register
    staa   RPAGE                       ;  3~, Store into CPU's RPAGE register

    pula                               ;  3~, Get value of EPAGE register
    staa   EPAGE                       ;  3~, Store into CPU's EPAGE register

    pula                               ;  3~, Get value of GPAGE register
    staa   GPAGE                       ;  3~, Store into CPU's GPAGE register

    rti                                ; 12~, Return from interrupt, no higher priority tasks ready.
