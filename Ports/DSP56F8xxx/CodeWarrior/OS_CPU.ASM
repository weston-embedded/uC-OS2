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
;                                   Freescale DSP568xx Specific code
;                        Metrowerks CodeWarrior for Freescale Embedded DSP568xx
;
; Filename : os_cpu.asm
; Version  : V2.93.01
;********************************************************************************************************

      SECTION rtlib

      include "os_portasm.h"

      GLOBAL FOSTaskStkInit
      GLOBAL FOSStartHighRdy
      GLOBAL FOSCtxSw
      GLOBAL FOSIntCtxSw
      GLOBAL FOSTickISR
      GLOBAL FtimerTick
      GLOBAL FOS_DISABLE_INTERRUPTS
      GLOBAL FOS_ENABLE_INTERRUPTS

      XREF   FOSIntExit
      XREF   FOSTimeTick
      XREF   FOSTaskSwHook

      XREF   FOSIntNesting
      XREF   FOSTickDOSCtr
      XREF   FValueOSPrioHighRdy
      XREF   FSetOSPrioCur
      XREF   FSetOSRunning
      XREF   FOSTCBCur
      XREF   FOSTCBHighRdy

      XREF   FTickSetSP

      XREF   FPE_state


;
; After executing the SWI through the SDK Dispatcher and calling the uC/OS ISR,
; the stack frame looks like this:
;
;                                               PC          (Low memory)
;                                               SR
;                                               Optional alignment word
;                                               SP
;                                               Y
;                                               R0
;                                               A2
;                                               B2
;                                               X0
;                                               N3
;                                               M01
;                                               OMR
;                                               C2
;                                               D2
;                                               A10
;                                               B10
;                                               N
;                                               R1
;                                               R2
;                                               R3
;                                               LA
;                                               LA2
;                                               LC2
;                                               LC
;                                               HWS
;                                               HWS
;                       PC of DispatchRestore
;       OSTCBCur->OSTCBStkPtr -->     SR of DispatchRestore     (High memory)
;
; OSCtwSw and OSIntCtwSw add the CW Floating Point state on the stack, as follows;
;
;                OSTCBCur->OSTCBStkPtr -->      C10           (Low memory)
;                                               D10
;                                               R5
;                                               FPE_State     (High memory)
;
; OSTaskStkInit creates the above stack contents in preparation for
; starting a uC/OS task as if it had been suspended thru OSCtwSw.
;
; WARNING:  OSTaskStkInit must be kept in sync with registers saved by Dispatcher.
;

;/*
;*********************************************************************************************************
;*                                        INITIALIZE A TASK'S STACK
;*
;* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
;*              stack frame of the task being created.  This function is highly processor specific.
;*
;* Arguments  : task          is a pointer to the task code
;*
;*              pdata         is a pointer to a user supplied data area that will be passed to the task
;*                            when the task first executes.
;*
;*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
;*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
;*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
;*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
;*                            of the stack.
;*
;*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
;*                            (see uCOS_II.H for OS_TASK_OPT_???).
;*
;* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
;*              been placed on the stack in the proper order.
;*
;* Note(s)    : Interrupts are enabled when your task starts executing.
;*
;*********************************************************************************************************
;*/

;asm OS_STK *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
;{
; /*
;   Input Parameters:
;
;     Y0       =>  opt
;     R2       =>  pd
;     R3       =>  pdata
;     R4       =>  ptos
;
;   Register Usage During Execution:
;
;     R2       =>  pd
;     R3       =>  pdata
;     R4       =>  ptos
;     Y        =>  temp
;     X0       =>  temp
;           A        =>  temp
;
; */

FDispatchRestore:

    move.l  x:(SP)-,R4

    ; restore hardware stack
    move.w  hws,A  ; clear HWS to ensure reload
    move.w  hws,A
    move.l  x:(SP)-,hws
    move.l  x:(SP)-,hws

    ; restore all saved registers
    move.l  x:(SP)-,lc
    move.l  x:(SP)-,lc2
    move.l  x:(SP)-,la2
    move.l  x:(SP)-,la

    move.l  x:(SP)-,R3
    move.l  x:(SP)-,R2
    move.l  x:(SP)-,R1
    move.l  x:(SP)-,N
    move.l  x:(SP)-,B
    move.l  x:(SP)-,A

    move.l  x:(SP)-,D2
    move.l  x:(SP)-,C2
    move.l  x:(SP)-,OMR
    move.l  x:(SP)-,M01
    move.l  x:(SP)-,N3
    move.l  x:(SP)-,X0
    move.l  x:(SP)-,B2
    move.l  x:(SP)-,A2
    move.l  x:(SP)-,R0
    move.l  x:(SP)-,Y
    move.l  x:(SP)-,SP
    nop
    nop

    rti



FOSTaskStkInit:

    adda    #1,R4               ; Align SP

    move.l  R4,A
    bfclr   #$0001,A0
    move.l  A10,R4

    move.w  R2,X:(R4)+          ; PC portion of return address

    move.l  R2,A                ; Isolate SR portion of return address
    andc    #$001F,A1
    move.w  #10,Y0
    asll.w  A1,Y0,X0
    move.w  X0,x:(R4)           ; Create new SR with interrupts enabled

    move.l  R4,A                ; Store SP to simulate AlignSP
    adda    #1,R4
    nop
    move.l  A10,x:(R4)+

    move.l  #$0000,Y
    rep     #6
    move.l  Y,x:(R4)+           ; Clear following 6 registers
                                ;    Y, R0, A2, B2, X0, N3

    move.w  #0,A
    move.w  M01,A0
    move.l  A10,x:(R4)+
    move.w  OMR,A0
    move.l  A10,x:(R4)+

    rep     #6
    move.l  Y,x:(R4)+           ; Clear following 6 registers
                                ;    C2, D2, A10, B10, N, R1

    move.l  R3,x:(R4)+          ; Save pdata as R2 (task parameter)

    rep     #8
    move.l  Y,x:(R4)+           ; Clear following 8 registers
                                ;    R3, LA, LA2, LC2, LC, HWS, HWS, R4

    move.l  #>>FDispatchRestore,A ; Create jsr return to DispatchRestore
    move.w  A0,x:(R4)+          ; PC portion of DispatchRestore address
    andc    #$001F,A1
    move.w  #10,Y0
    asll.w  A1,Y0,X0
    move.w  X0,x:(R4)+

    move.l  #$0000,Y
    rep     #3
    move.l  Y,x:(R4)+           ; Clear following 3 registers
                                ;    C10, D10, R5

    move.w  Y1,x:(R4)+          ; Save FPE_state as long (0)
    move.w  Y1,x:(R4)           ; Note that SP (R4) is now odd

    tfra    R4,R2               ; return ((OS_STK *)stk);

    rts
;}

            PAGE
;*********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
;
; Note : OSStartHighRdy() MUST:
;           a) Call OSTaskSwHook() then,
;           b) Set OSRunning to TRUE,
;           c) Switch to the highest priority task.
;*********************************************************************************************************

FOSStartHighRdy:


    jsr      FOSTaskSwHook                  ; Call user defined task switch hook
;
    LoadRx   X:FOSTCBHighRdy,R2             ; R2 = OSTCBHighRdy
    LoadRx   X:(R2),R2
    tfra     R2,SP                          ; SP = OSTCBHighRdy->OSTCBStkPtr
    nop
    nop
;
    jsr      FSetOSRunning                  ; Indicate that multitasking has started
;
    bra      RestartTask                    ; Run task


            PAGE
;*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From task level)
;                                           void OSCtxSw(void)
;
; Note(s): 1) Upon entry,
;             OSTCBCur     points to the OS_TCB of the task to suspend
;             OSTCBHighRdy points to the OS_TCB of the task to resume
;
;*********************************************************************************************************

FOSIntCtxSw:
FOSCtxSw:
;
    adda    #2,SP
    move.l  C10,x:(SP)+
    move.l  D10,x:(SP)+
    move.l  R5,x:(SP)+

    ;
    ; Save FP Emulation State
    ;
    move.w  x:FPE_state,A
    move.l  A10,x:(SP)

    ;
    ; uC/OS logic
    ;
    jsr     FOSTaskSwHook           ; Call user defined task switch hook

    LoadRx  x:FOSTCBCur,R2          ; Pre-load OSTCBCur for uC/OS logic

;   StoreRx SP,X:(R2)               ; Save stack top for current task

    tfra    SP,R5
    StoreRx R5,X:(R2)

    LoadRx  x:FOSTCBHighRdy,R2      ; OSTCBCur = OSTCBHighRdy (Step 1)

;   LoadRx  x:(R2),SP               ; SP = OSTCBHighRdy->OSTCBStkPtr
    LoadRx  x:(R2),R5
    tfra    R5,SP

    StoreRx R2,x:FOSTCBCur          ; OSTCBCur = OSTCBHighRdy (Step 2)

    jsr     FValueOSPrioHighRdy     ; OSPrioCur = OSPrioHighRdy
    jsr     FSetOSPrioCur           ; Finish OSPrioCur = OSPrioHighRdy

RestartTask:                            ; Return to new task

    move.l  x:(SP)-,y1              ; Restore Floating Point emulation state
    move.w  y1,x:FPE_state
    move.l  x:(SP)-,R5              ; Restore R5
    move.l  x:(SP)-,D               ; Restore D10
    move.l  x:(SP)-,C               ; Restore C10

    rts                             ; Return to Caller (DispatchRestore or uC/OS)


;/*
;*********************************************************************************************************
;*
;*  asm void OS_DISABLE_INTERRUPTS (OS_CPU_SR * cpu_sr);
;*
;*********************************************************************************************************
;*/

FOS_DISABLE_INTERRUPTS:

    move.w  SR,x:(R2)
    bfset   #$0300,SR
    rts

;/*
;*********************************************************************************************************
;*
;*  asm void OS_ENABLE_INTERRUPTS (OS_CPU_SR cpu_sr);
;*
;*********************************************************************************************************
;*/
FOS_ENABLE_INTERRUPTS:

    moveu.w  Y0,SR
    rts



        ENDSEC

        END
