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
;                                          683xx Specific code
;                                             COSMIC C V4.1
;
; Filename : os_cpu_a.s
; Version  : V2.93.01
;********************************************************************************************************

;********************************************************************************************************
;                                         CONFIGURATION CONSTANTS
;********************************************************************************************************


;********************************************************************************************************
;                                           I/O PORT ADDRESSES
;********************************************************************************************************


;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************

    xdef   _OSCPUSaveSR
    xdef   _OSCPURestoreSR
    xdef   _OSCtxSw
    xdef   _OSIntCtxSw                     ; Satisfy OSIntExit() in OS_CORE.C
    xdef   _OSIntExit68K
    xdef   _OSStartHighRdy
    xdef   _OSTickISR

;********************************************************************************************************
;                                         EXTERNAL DECLARATIONS
;********************************************************************************************************

    xref   _OSCtxSwCtr
    xref   _OSIntExit
    xref   _OSIntNesting
    xref   _OSLockNesting
    xref   _OSPrioCur
    xref   _OSPrioHighRdy
    xref   _OSRdyGrp
    xref   _OSRdyTbl
    xref   _OSRunning
    xref   _OSTaskSwHook
    xref   _OSTCBCur
    xref   _OSTCBHighRdy
    xref   _OSTCBPrioTbl
    xref   _OSTimeTick
    xref   _OSUnMapTbl

;********************************************************************************************************
;                              OSCPUSaveSR() for OS_CRITICAL_METHOD #3
;
; Description : This functions implements the OS_CRITICAL_METHOD #3 function to preserve the state of the
;               interrupt disable flag in order to be able to restore it later.
;
; Arguments   : none
;
; Returns     : It is assumed that the return value is placed in the D7 register as expected by the
;               compiler.
;********************************************************************************************************

_OSCPUSaveSR:
    MOVE.W    SR,D7                         ; Copy SR into D7
    ORI       #$0700,SR                     ; Disable interrupts
    RTS

;********************************************************************************************************
;                              OSCPURestoreSR() for OS_CRITICAL_METHOD #3
;
; Description : This functions implements the OS_CRITICAL_METHOD #function to restore the state of the
;               interrupt flag.
;
; Arguments   : os_cpu_sr   is the contents of the SR to restore.  It is assumed that this 'argument' is
;                           passed in the D7 register of the CPU by the compiler.
;
; Returns     : None
;********************************************************************************************************

_OSCPURestoreSR:
    MOVE.W    D7,SR
    RTS


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
;                  OSTCBHighRdy->OSTCBStkPtr +  0  ---->  D0    (H)        Low Memory
;                                            +  2         D0    (L)
;                                            +  4         D1    (H)
;                                            +  6         D1    (L)
;                                            +  8         D2    (H)
;                                            + 10         D2    (L)
;                                            + 12         D3    (H)
;                                            + 14         D3    (L)
;                                            + 16         D4    (H)
;                                            + 18         D4    (L)
;                                            + 20         D5    (H)
;                                            + 22         D5    (L)
;                                            + 24         D6    (H)
;                                            + 26         D6    (L)
;                                            + 28         D7    (H)
;                                            + 30         D7    (L)
;                                            + 32         A0    (H)
;                                            + 34         A0    (L)
;                                            + 36         A1    (H)
;                                            + 38         A1    (L)
;                                            + 40         A2    (H)
;                                            + 42         A2    (L)
;                                            + 44         A3    (H)
;                                            + 46         A3    (L)
;                                            + 48         A4    (H)
;                                            + 50         A4    (L)
;                                            + 52         A5    (H)
;                                            + 54         A5    (L)
;                                            + 56         A6    (H)
;                                            + 58         A6    (L)
;                                            + 60         OS_INITIAL_SR
;                                            + 62         task  (H)
;                                            + 64         task  (L)
;                                            + 66         0x80 + 4 * TRAP#
;                                            + 68         task  (H)
;                                            + 70         task  (L)
;                                            + 72         pdata (H)
;                                            + 74         pdata (L)        High Memory
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task.
;********************************************************************************************************

_OSStartHighRdy:
    JSR       _OSTaskSwHook            ; Invoke user defined context switch hook
    ADDQ.B    #1,_OSRunning            ; Indicate that we are multitasking

    MOVE.L    (_OSTCBHighRdy),A1       ; Point to TCB of highest priority task ready to run
    MOVE.L    (A1),A7                  ; Get the stack pointer of the task to resume

    MOVEM.L   (A7)+,A0-A6/D0-D7        ; Restore the CPU registers

    RTE                                ; Run task

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
;               2) The stack frame of the task to suspend looks as follows (the registers for
;                  task to suspend need to be saved):
;
;                                         SP +  0  ---->  SR                   Low Memory
;                                            +  2         0x80 + 4 * TRAP#
;                                            +  4         PC of task  (H)
;                                            +  6         PC of task  (L)      High Memory
;
;               3) The stack frame of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0  ---->  D0    (H)           Low Memory
;                                            +  2         D0    (L)
;                                            +  4         D1    (H)
;                                            +  6         D1    (L)
;                                            +  8         D2    (H)
;                                            + 10         D2    (L)
;                                            + 12         D3    (H)
;                                            + 14         D3    (L)
;                                            + 16         D4    (H)
;                                            + 18         D4    (L)
;                                            + 20         D5    (H)
;                                            + 22         D5    (L)
;                                            + 24         D6    (H)
;                                            + 26         D6    (L)
;                                            + 28         D7    (H)
;                                            + 30         D7    (L)
;                                            + 32         A0    (H)
;                                            + 34         A0    (L)
;                                            + 36         A1    (H)
;                                            + 38         A1    (L)
;                                            + 40         A2    (H)
;                                            + 42         A2    (L)
;                                            + 44         A3    (H)
;                                            + 46         A3    (L)
;                                            + 48         A4    (H)
;                                            + 50         A4    (L)
;                                            + 52         A5    (H)
;                                            + 54         A5    (L)
;                                            + 56         A6    (H)
;                                            + 58         A6    (L)
;                                            + 60         OS_INITIAL_SR
;                                            + 62         PC of task  (H)
;                                            + 64         PC of task  (L)
;                                            + 66         0x80 + 4 * TRAP#    High Memory
;********************************************************************************************************

_OSCtxSw:
    MOVEM.L   A0-A6/D0-D7,-(A7)              ; 72~ Save the registers of the current task

    MOVE.L    (_OSTCBCur),A1                 ;  4~ Save the stack pointer in the suspended task TCB
    MOVE.L    A7,(A1)                        ;  4~

    JSR       _OSTaskSwHook                  ; 13~ Invoke user defined context switch hook

    MOVE.L    (_OSTCBHighRdy),A1             ;  4~ OSTCBCur  = OSTCBHighRdy
    MOVE.L    A1,(_OSTCBCur)                 ;  4~
    MOVE.L    (A1),A7                        ;  4~ Get the stack pointer of the task to resume

    MOVE.B    _OSPrioHighRdy,_OSPrioCur      ;  6~ OSPrioCur = OSPrioHighRdy

    MOVEM.L   (A7)+,A0-A6/D0-D7              ; 70~ Restore the CPU registers

    RTE                                      ; 24~ Run task

;********************************************************************************************************
;                                      INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is provided for backward compatibility and your ISR MUST NOT call
;               OSIntExit().  Instead, your ISR MUST JUMP to OSIntExit68K().
;
; Arguments   : none
;
; Note(s)     : 1) The stack frame upon entry:
;
;********************************************************************************************************

_OSIntCtxSw:
    ADDA.L    #18,A7                         ; Adjust the stack

    MOVE.L    (_OSTCBCur),A1                 ; Save the stack pointer in the suspended task TCB
    MOVE.L    A7,(A1)

    JSR       _OSTaskSwHook                  ; Invoke user defined context switch hook

    MOVE.L    (_OSTCBHighRdy),A1             ; OSTCBCur  = OSTCBHighRdy
    MOVE.L    A1,(_OSTCBCur)

    MOVE.B    _OSPrioHighRdy,_OSPrioCur      ; OSPrioCur = OSPrioHighRdy

    MOVE.L    (A1),A7                        ; Get the stack pointer of the task to resume

    MOVEM.L   (A7)+,A0-A6/D0-D7              ; Restore the CPU registers

    RTE                                      ; Run task

;********************************************************************************************************
;                                      INTERRUPT EXIT FUNCTION
;
; Description : Your ISR MUST JUMP to this function when it is done.
;
; Arguments   : none
;
; Note(s)     : 1) Stack frame upon entry:
;
;                  SP +  0  ---->  D0    (H)                         Low Memory
;                     +  2         D0    (L)
;                     +  4         D1    (H)
;                     +  6         D1    (L)
;                     +  8         D2    (H)
;                     + 10         D2    (L)
;                     + 12         D3    (H)
;                     + 14         D3    (L)
;                     + 16         D4    (H)
;                     + 18         D4    (L)
;                     + 20         D5    (H)
;                     + 22         D5    (L)
;                     + 24         D6    (H)
;                     + 26         D6    (L)
;                     + 28         D7    (H)
;                     + 30         D7    (L)
;                     + 32         A0    (H)
;                     + 34         A0    (L)
;                     + 36         A1    (H)
;                     + 38         A1    (L)
;                     + 40         A2    (H)
;                     + 42         A2    (L)
;                     + 44         A3    (H)
;                     + 46         A3    (L)
;                     + 48         A4    (H)
;                     + 50         A4    (L)
;                     + 52         A5    (H)
;                     + 54         A5    (L)
;                     + 56         A6    (H)
;                     + 58         A6    (L)
;                     + 60         Task or ISR's SR
;                     + 62         PC of task  (H)
;                     + 64         PC of task  (L)
;                     + 66         0x80 + 4 * TRAP#                  High Memory
;********************************************************************************************************

_OSIntExit68K:
    ORI       #$0700,SR                     ; Disable interrupts

    SUBQ.B    #1,_OSIntNesting              ; OSIntNesting--;
    BNE       OSIntExit68K_Exit

    MOVE.B    _OSLockNesting,D0			    ; if (OSLockNesting == 0) {
    BNE       OSIntExit68K_Exit

    MOVE.W    (60,A7),D0                    ;     if (LAST nested ISR) {
    ANDI.W    #$0700,D0
    BNE       OSIntExit68K_Exit

    MOVEQ.L   #0,D0                         ;         y = OSUnMapTbl[OSRdyGrp];
    MOVE.B    _OSRdyGrp,D0
    MOVE.L    D0,A0
    MOVE.B    (_OSUnMapTbl,A0),D2

    MOVE.B    D2,D0                         ;         OSPrioHighRdy = (INT8U)((y << 3) + OSUnMapTbl[OSRdyTbl[y]]);
    LSL.B     #3,D0
    MOVEQ.L   #0,D1
    MOVE.B    D2,D1
    MOVE.L    D1,A0
    MOVEQ.L   #0,D1
    MOVE.B    (_OSRdyTbl,A0),D1
    MOVE.L    D1,A0
    ADD.B     (_OSUnMapTbl,A0),D0

    MOVE.B    D0,_OSPrioHighRdy             ;          if (OSPrioHighRdy != OSPrioCur) {
    CMP.B     _OSPrioCur,D0
    BEQ       OSIntExit68K_Exit

    MOVEQ.L   #0,D1                         ;              OSTCBHighRdy  = OSTCBPrioTbl[OSPrioHighRdy];
    MOVE.B    D0,D1
    LSL.L     #2,D1
    MOVE.L    D1,A0
    MOVE.L    (_OSTCBPrioTbl,A0),_OSTCBHighRdy


    ADDQ.L    #1,_OSCtxSwCtr                ;              OSCtxSwCtr++;
                                            ;              PERFORM INTERRUPT LEVEL CONTEXT SWITCH:
    MOVE.L    (_OSTCBCur),A1                ;                  Save the stack pointer in the suspended task TCB
    MOVE.L    A7,(A1)

    JSR       _OSTaskSwHook                 ;                  Invoke user defined context switch hook

    MOVE.L    (_OSTCBHighRdy),A1            ;                  OSTCBCur  = OSTCBHighRdy
    MOVE.L    A1,(_OSTCBCur)

    MOVE.B    _OSPrioHighRdy,_OSPrioCur     ;                  OSPrioCur = OSPrioHighRdy

    MOVE.L    (A1),A7                       ;                  Get the stack pointer of the task to resume

OSIntExit68K_Exit:
    MOVEM.L   (A7)+,A0-A6/D0-D7             ; Restore the CPU registers

    RTE                                     ; Return to task or nested ISR

;********************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.
;
; Arguments   : none
;
; Note(s)    : The following C-like pseudo-code describe the operation being performed in the code below.
;
;              OSIntNesting++;
;              Save all registers on the current task's stack;
;              if (OSIntNesting == 1) {
;                 OSTCBCur->OSTCBStkPtr = SP
;              }
;              Clear the interrupt source;
;              OSTimeTick();              Notify uC/OS-II that a tick has occured
;              JUMP to OSIntExit68K;      Notify uC/OS-II about end of ISR
;********************************************************************************************************

_OSTickISR:
    ADDQ.B    #1,_OSIntNesting               ; OSIntNesting++;

    MOVEM.L   A0-A6/D0-D7,-(A7)              ; Save the registers of the current task

    CMPI      #1,_OSIntNesting               ; if (OSIntNesting == 1) {
    BNE       _OSTickISR1
;
    MOVE.L    (_OSTCBCur),A1                 ;     OSTCBCur->OSTCBStkPtr = SP;
    MOVE.L    A7,(A1)                        ; }

_OSTickISR1:
;   CLEAR the INTERRUPT source!

    JSR       _OSTimeTick                    ; Call uC/OS-II's tick updating function

    JMP       _OSIntExit68K                  ; Exit ISR through uC/OS-II

    END
