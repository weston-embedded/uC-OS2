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
;                                           Renesas RL78 Port
;
; Filename  : os_cpu_a.asm
; Version   : V2.93.01
;********************************************************************************************************
; For       : Renesas RL78
; Toolchain : IAR EWRL78 v1.2x and up
;********************************************************************************************************

#include  "os_cpu_a.inc"

;********************************************************************************************************
;                                  PUBLIC AND EXTERNAL DECLARATIONS
;********************************************************************************************************

        PUBLIC  OSStartHighRdy
        PUBLIC  OSCtxSw
        PUBLIC  OSIntCtxSw
        PUBLIC  OSTickISR
        EXTERN  OSTaskSwHook
        EXTERN  OSTCBHighRdy
        EXTERN  OSRunning
        EXTERN  OSTCBCur
        EXTERN  OSPrioCur
        EXTERN  OSPrioHighRdy
        EXTERN  OSIntEnter
        EXTERN  OSTimeTick
        EXTERN  OSIntExit
        EXTERN  OSIntNesting


;********************************************************************************************************
;                                           MACRO DEFINITIONS
;********************************************************************************************************

        ASEGN   RCODE:CODE, 0x007E
        DW      OSCtxSw                                         ; Context Switch vector

        RSEG    CODE                                            ; Program code

;********************************************************************************************************
;                                  START HIGHEST PRIORITY READY TASK
;
; Description: This function is called by OSStart() to start the highest priority task that is ready to run.
;
; Note       : OSStartHighRdy() MUST:
;                 a) Call OSTaskSwHook() then,
;                 b) Set OSRunning to TRUE,
;                 c) Switch to the highest priority task.
;********************************************************************************************************

OSStartHighRdy:

        CALL    OSTaskSwHook                                    ; call OSTaskSwHook()
        MOVW    RP1, OSTCBHighRdy                               ; address of OSTCBHighRdy in RP1
        MOVW    RP0, 0x0000[RP1]                                ; RP0 = OSTCBHighRdy->OSTCBStkPtr
        MOVW    SP, RP0                                         ; stack pointer = RP0

        MOV     OSRunning, #0x01                                ; OSRunning = True

        POPALL                                                  ; restore all processor registers from new task's stack

        RETI                                                    ; return from interrupt

;********************************************************************************************************
;                                     TASK LEVEL CONTEXT SWITCH
;
; Description: This function is called by OS_Sched() to perform a task level context switch.
;
; Note       : OSCtxSw() MUST:
;                 a) Save the current task's registers onto the current task stack
;                 b) Save the SP into the current task's OS_TCB
;                 c) Call OSTaskSwHook()
;                 d) Copy OSPrioHighRdy to OSPrioCur
;                 e) Copy OSTCBHighRdy to OSTCBCur
;                 f) Load the SP with OSTCBHighRdy->OSTCBStkPtr
;                 g) Restore all the registers from the high priority task stack
;                 h) Perform a return from interrupt
;********************************************************************************************************

OSCtxSw:

        PUSHALL                                                 ; save processor registers on the stack

                                                                ; save current task's stack pointer into current task's OS_TCB
        MOVW    RP2, OSTCBCur                                   ; OSTCBCur in RP2
        MOVW    RP0, SP
        MOVW    [RP2], RP0                                      ; OSTCBCur->OSTCBStkPtr = SP

        CALL    OSTaskSwHook                                    ; call OSTaskSwHook

        MOVW    RP0, OSTCBHighRdy                               ; get address of OSTCBHighRdy
        MOVW    OSTCBCur, RP0                                   ; OSTCBCur = OSTCBHighRdy

        MOV     R1, OSPrioHighRdy
        MOV     OSPrioCur, R1                                   ; OSPrioCur = OSPrioHighRdy

        MOVW    RP1, OSTCBHighRdy                               ; get address of OSTCBHighRdy
        MOVW    RP0, 0x0000[RP1]                                ; RP0 = OSTCBHighRdy->OSTCBStkPtr
        MOVW    SP, RP0                                         ; stack pointer = RP0

        POPALL                                                  ; restore all processor registers from new task's stack

        RETB                                                    ; return from break (software level)

;********************************************************************************************************
;                                       ISR LEVEL CONTEXT SWITCH
;
; Description: This function is called by OSIntExit() to perform an ISR level context switch.
;
; Note       : OSIntCtxSw() MUST:
;                 a) Call OSTaskSwHook()
;                 b) Copy OSPrioHighRdy to OSPrioCur
;                 c) Copy OSTCBHighRdy to OSTCBCur
;                 d) Load the SP with OSTCBHighRdy->OSTCBStkPtr
;                 e) Restore all the registers from the high priority task stack
;                 f) Perform a return from interrupt
;********************************************************************************************************

OSIntCtxSw:
        CALL    OSTaskSwHook                                    ; call OSTaskSwHook

        MOVW    RP0, OSTCBHighRdy                               ; get address of OSTCBHighRdy
        MOVW    OSTCBCur, RP0                                   ; OSTCBCur = OSTCBHighRdy

        MOV     R1, OSPrioHighRdy
        MOV     OSPrioCur, R1                                   ; OSPrioCur = OSPrioHighRdy

        MOVW    RP1, OSTCBHighRdy                               ; get address of OSTCBHighRdy
        MOVW    RP0, 0x0000[RP1]                                ; RP0 = OSTCBHighRdy->OSTCBStkPtr
        MOVW    SP, RP0                                         ; stack pointer = RP0

        POPALL                                                  ; restore all processor registers from new task's stack

        RETI                                                    ; return from interrupt

;********************************************************************************************************
;                                              TICK ISR
;
; Description: This ISR handles tick interrupts.  This ISR uses the Watchdog timer as the tick source.
;
; Notes      : 1) The following C pseudo-code describes the operations being performed in the code below.
;
;              a) Save processor registers;
;              b) Increment OSIntNesting;
;              c) if (OSIntNesting == 1) {
;                     OSTCBCur->OSTCBStkPtr = SP;
;                 }
;              d) Call OSTimeTick();
;              e) Call OSIntExit();
;              f) Restore processosr Registers;
;
;           2) OS_CPU_TickHandler() must be registered in the proper vector address of timer that will be
;              used as the tick.
;
;           3) All the other ISRs must have the following implementation to secure proper register saving &
;              restoring when servicing an interrupt
;
;              MyISR
;                  OS_ISR_ENTER
;                  ISR Body here
;                  OS_ISR_EXIT
;********************************************************************************************************

OSTickISR:

        OS_ISR_ENTER

        CALL    OSTimeTick                                      ; call OSTimeTick()

        OS_ISR_EXIT

        END
