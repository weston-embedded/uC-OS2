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
;                                           Renesas M16C Port
;
; Filename  : os_cpu_a.asm
; Version   : V2.93.01
;********************************************************************************************************
; For       : Renesas M16C
; Toolchain : IAR's EW for M16C
;********************************************************************************************************

;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

    RSEG        CSTACK

    RSEG        ISTACK

    RSEG        UDATA0

    EXTERN      OSTCBCur               ; Declared as OS_TCB *               , 32-bit long
    EXTERN      OSTCBHighRdy           ; Declared as OS_TCB *               , 32-bit long
    EXTERN      OSPrioCur              ; Declared as INT8U                  ,  8-bit long
    EXTERN      OSPrioHighRdy          ; Declared as INT8U                  ,  8-bit long
    EXTERN      OSIntNesting           ; Declared as INT8U                  ,  8-bit long
    EXTERN      OSRunning              ; Declared as Boolean (unsigned char),  8-bit long

    RSEG        CODE(1)

    EXTERN      OSIntExit              ; External functions written in C
    EXTERN      OSTimeTick
    EXTERN      OSTaskSwHook

    PUBLIC      OSStartHighRdy
    PUBLIC      OSCtxSw
    PUBLIC      OSIntCtxSw
    PUBLIC      OSTickISR

;********************************************************************************************************
;                                           START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; Note(s) : 1) OSStartHighRdy() MUST:
;              a) Call OSTaskSwHook() then,
;              b) Set OSRunning to TRUE,
;              c) Switch to the highest priority task.
;********************************************************************************************************

    .EVEN

OSStartHighRdy:
    JSR         OSTaskSwHook

    MOV.W       OSTCBHighRdy, A0               ; ISP = OSTCBHighRdy->OSTCBStkPtr
    LDC         [A0], ISP

    MOV.B       #01H, OSRunning                ; OSRunning = TRUE

    POPM        R0,R1,R2,R3,A0,A1,SB,FB

    REIT

;********************************************************************************************************
;                         PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
;
; Note(s) : 1) OSCtxSw() is called in SVC mode with BOTH FIQ and IRQ interrupts DISABLED.
;
;           2) The pseudo-code for OSCtxSw() is:
;              a) Save the current task's context onto the current task's stack,
;              b) OSTCBCur->OSTCBStkPtr = SP;
;              c) OSTaskSwHook();
;              d) OSPrioCur             = OSPrioHighRdy;
;              e) OSTCBCur              = OSTCBHighRdy;
;              f) SP                    = OSTCBHighRdy->OSTCBStkPtr;
;              g) Restore the new task's context from the new task's stack,
;              h) Return to new task's code.
;
;           3) Upon entry:
;              OSTCBCur      points to the OS_TCB of the task to suspend,
;              OSTCBHighRdy  points to the OS_TCB of the task to resume.
;
;           4) OSCtxSw must be mapped to interrupt #0 in the vector table.
;********************************************************************************************************

    .EVEN

OSCtxSw:
    PUSHM       R0,R1,R2,R3,A0,A1,SB,FB

    MOV.W       OSTCBCur, A0                    ; OSTCBCur->OSTCBStkPtr = SP
    STC         ISP, [A0]

    JSR         OSTaskSwHook                    ; OSTaskSwHook()

    MOV.W       OSTCBHighRdy,  OSTCBCur         ; OSTCBCur  = OSTCBHighRdy

    MOV.W       OSPrioHighRdy, OSPrioCur        ; OSPrioCur = OSPrioHighRdy

    MOV.W       OSTCBHighRdy, A0                ; SP        = OSTCBHighRdy->OSTCBStkPtr
    LDC         [A0], ISP

    POPM        R0,R1,R2,R3,A0,A1,SB,FB         ; Restore all processor registers from the new task's stack

    REIT


;********************************************************************************************************
;                     PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
;
; Note(s) : 1) OSIntCtxSw() is called in SVC mode with BOTH FIQ and IRQ interrupts DISABLED.
;
;           2) The pseudo-code for OSCtxSw() is:
;              a) OSTaskSwHook();
;              b) OSPrioCur             = OSPrioHighRdy;
;              c) OSTCBCur              = OSTCBHighRdy;
;              d) SP                    = OSTCBHighRdy->OSTCBStkPtr;
;              e) Restore the new task's context from the new task's stack,
;              f) Return to new task's code.
;
;           3) Upon entry:
;              OSTCBCur      points to the OS_TCB of the task to suspend,
;              OSTCBHighRdy  points to the OS_TCB of the task to resume.
;********************************************************************************************************

    .EVEN

OSIntCtxSw:
    JSR         OSTaskSwHook                    ; OSTaskSwHook()

    MOV.W       OSTCBHighRdy, OSTCBCur          ; OSTCBCur  = OSTCBHighRdy

    MOV.W       OSPrioHighRdy, OSPrioCur        ; OSPrioCur = OSPrioHighRdy

    MOV.W       OSTCBHighRdy, A0                ; SP        = OSTCBHighRdy->OSTCBStkPtr
    LDC         [A0], ISP

    POPM        R0,R1,R2,R3,A0,A1,SB,FB         ; Restore all processor registers from the new task's stack

    REIT

;********************************************************************************************************
;                                    uC/OS-II TIME TICK ISR
;                                     void OSTickISR(void)
;
; Note(s) : 1) OSTickISR() should be placed on the appropriate interrupt vector.
;
;           2) Pseudo code:
;              a) Save all registers
;              b) OSIntNesting++
;              c) if (OSIntNesting == 1) {
;                     OSTCBCur->OSTCBStkPtr = SP
;                 }
;              d) OSTimeTick();
;              e) OSIntExit();
;              f) Restore all registers
;              g) Return from interrupt;
;********************************************************************************************************

    .EVEN

OSTickISR:

    PUSHM       R0,R1,R2,R3,A0,A1,SB,FB             ; Save current task's registers

    INC.B       OSIntNesting                        ; OSIntNesting++
    CMP.B       #1,OSIntNesting                     ; if (OSIntNesting == 1) {
    JNE         OSTickISR1

    MOV.W       OSTCBCur, A0                        ;     OSTCBCur->OSTCBStkPtr = SP
    STC         ISP, [A0]                           ; }

OSTickISR1:
    JSR         OSTimeTick                          ; OSTimeTick()

    JSR         OSIntExit                           ; OSIntExit()

    POPM        R0,R1,R2,R3,A0,A1,SB,FB             ; Restore registers from the new task's stack

    REIT



    END
