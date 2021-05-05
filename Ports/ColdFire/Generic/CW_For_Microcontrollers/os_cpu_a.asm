/*
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
*/

/*
;********************************************************************************************************
; Filename : os_cpu_a.asm
; Version  : V2.93.01
;********************************************************************************************************
; Note(s)  : 1) This port uses the MOVEM.L (A7),D0-D7/A0-A6, LEA 60(A7)A7 construct instead of
;               the traditional 68xxx MOVEM.L (A7)+,D0-D7/A0-A6.  It is perfectly in order to
;               push/pop individual registers using MOVEM.L (A7)+,D2, etc. but it's a bit slower
;               (and more verbose).
;
;            2) The LEA instruction is required because the ColdFire cannot push multiple
;               registers directly to the stack.
;
;            3) Changed OSIntCtxSw for new uC/OS-II version 2.51 features.
;********************************************************************************************************
*/

/*
;*************************************************************************************************
;                                       PUBLIC DECLARATIONS
;*************************************************************************************************
*/

        .global  _OS_CPU_SR_Save
        .global  _OS_CPU_SR_Restore
        .global  _OSCtxSw
        .global  _OSIntCtxSw
        .global  _OSStartHighRdy

		.global  _OS_My_ISR

/*
;**************************************************************************************************
;                                     EXTERNAL DECLARATIONS
;**************************************************************************************************
*/

        .extern  _OSRunning
        .extern  _OSIntExit
        .extern  _OSIntNesting
        .extern  _OSLockNesting
        .extern  _OSPrioCur
        .extern  _OSPrioHighRdy
        .extern  _OSRdyGrp
        .extern  _OSRdyTbl
        .extern  _OSTaskSwHook
        .extern  _OSTCBCur
        .extern  _OSTCBHighRdy
        .extern  _OSIntEnter

        .text
        .align 4

/*
;********************************************************************************************************
;                              OSCPUSaveSR() for OS_CRITICAL_METHOD #3
;
; Description : This functions implements the OS_CRITICAL_METHOD #3 function to preserve the state of the
;               interrupt disable flag in order to be able to restore it later.
;
; Arguments   : none
;
; Returns     : It is assumed that the return value is placed in the D0 register as expected by the
;               compiler.
;********************************************************************************************************
*/

_OS_CPU_SR_Save:
        MOVE.W   SR,D0            ; Copy SR into D0
        MOVE.L   D0,-(A7)         ; Save D0
        ORI.L    #0x0700,D0       ; Disable interrupts
        MOVE     D0,SR
        MOVE.L   (A7)+,D0         ; Restore D0
        RTS

/*
;********************************************************************************************************
;                              OSCPURestoreSR() for OS_CRITICAL_METHOD #3
;
; Description : This functions implements the OS_CRITICAL_METHOD #3 function to restore the state of the
;               interrupt flag.
;
; Arguments   : os_cpu_sr   is the contents of the SR to restore.  It is assumed that this 'argument' is
;                           passed on the stack as follows:
;
;               A7 +  0 ->  D0
;                  +  4     return address
;                  +  8     return address
;                  +  4     D0
;               A7 +  0 ->
;
; Returns     : None
;********************************************************************************************************
*/

_OS_CPU_SR_Restore:
        MOVE.L   D0,-(A7)         ; Save D0
        MOVE.W   10(A7),D0
        MOVE.W    D0,SR
        MOVE.L   (A7)+,D0         ; Restore D0
        RTS

/*
;*******************************************************************************************
;                            START HIGHEST PRIORITY TASK READY-TO-RUN
;
; Description: This function is called by OSStart() to start the highest priority task that
;              was created by your application before calling OSStart().
;
; Arguments  : none
;
; Note(s)    : 1) The stack frame is assumed to look as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0  ---->  D0         Low Memory
;                                            +  4         D1
;                                            +  8         D2
;                                            + 12         D3
;                                            + 16         D4
;                                            + 20         D5
;                                            + 24         D6
;                                            + 28         D7
;                                            + 32         A0
;                                            + 36         A1
;                                            + 40         A2
;                                            + 44         A3
;                                            + 48         A4
;                                            + 52         A5
;                                            + 56         A6
;                                            + 60         Format, Vector, OS_INITIAL_SR
;                                            + 64         task
;                                            + 68         task
;                                            + 72         p_arg       High Memory
;
;              2) OSStartHighRdy() MUST:
;                    a) Call OSTaskSwHook() then,
;                    b) Set OSRunning to TRUE,
;                    c) Switch to the highest priority task.
;*******************************************************************************************
*/

_OSStartHighRdy:
       JSR       _OSTaskSwHook            /* Invoke user defined context switch hook       */

       MOVEQ.L   #1, D4                   /* OSRunning = TRUE;                             */
       MOVE.B    D4,_OSRunning            /*   Indicates that we are multitasking          */

       MOVE.L    (_OSTCBHighRdy),A1       /* Point to TCB of highest prio task ready to run*/
       MOVE.L    (A1),A7                  /* Get the stack pointer of the task to resume   */

       MOVEM.L   (A7),D0-D7/A0-A6     	  /* Store all the regs                            */
       LEA       60(A7),A7                /* Advance the stack pointer                     */

       RTE                                /* Return to task                                */


/*
;*******************************************************************************************
;                                     TASK LEVEL CONTEXT SWITCH
;
; Description : This function is called when a task makes a higher prio task ready-to-run.
;
; Arguments   : none
;
; Note(s)     : 1) Upon entry,
;                   OSTCBCur     points to the OS_TCB of the task to suspend
;                   OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows (the registers
;                  for task to suspend need to be saved):
;
;                                         SP +  0  ---->  Format, Vector, SR   Low Memory
;                                            +  4         PC of task           High Memory
;
;               3) The stack frame of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr +  0  ---->  D0                   Low Memory
;                                            +  4         D1
;                                            +  8         D2
;                                            + 12         D3
;                                            + 16         D4
;                                            + 20         D5
;                                            + 24         D6
;                                            + 28         D7
;                                            + 32         A0
;                                            + 36         A1
;                                            + 40         A2
;                                            + 44         A3
;                                            + 48         A4
;                                            + 52         A5
;                                            + 56         A6
;                                            + 60         SR of task
;                                            + 64         PC of task           High Memory
;
;*******************************************************************************************
*/

_OSCtxSw:
       LEA       -60(A7),A7
       MOVEM.L   D0-D7/A0-A6,(A7)     	  /* Save the registers of the current task        */

       MOVE.L    (_OSTCBCur),A1           /* Save stack pointer in the suspended task TCB  */
       MOVE.L    A7,(A1)

       JSR       _OSTaskSwHook            /* Invoke user defined context switch hook       */

       MOVE.L    (_OSTCBHighRdy),A1       /* OSTCBCur = OSTCBHighRdy                       */
       MOVE.L    A1,(_OSTCBCur)
       MOVE.L    (A1),A7                  /* Get the stack pointer of the task to resume   */

       MOVE.B    (_OSPrioHighRdy),D0      /* OSPrioCur = OSPrioHighRdy                     */
       MOVE.B    D0,(_OSPrioCur)

       MOVEM.L   (A7),D0-D7/A0-A6     	  /* Restore the CPU registers                     */
       LEA       60(A7),A7

       RTE                                /* Run task                                      */

/*
;*******************************************************************************************
;                                 INTERRUPT LEVEL CONTEXT SWITCH
;
;
; Description : This function is provided for backward compatibility and to
;  				satisfy OSIntExit()
;               in OS_CORE.C.
;
; Arguments   : none
;*******************************************************************************************
*/

_OSIntCtxSw:
      JSR        _OSTaskSwHook            /* Invoke user defined context switch hook       */

      MOVE.B     (_OSPrioHighRdy),D0      /* OSPrioCur = OSPrioHighRdy                     */
      MOVE.B     D0,(_OSPrioCur)

      MOVE.L     (_OSTCBHighRdy),A1       /* OSTCBCur  = OSTCBHighRdy                      */
      MOVE.L     A1,(_OSTCBCur)
      MOVE.L     (A1),A7                  /* SP        = OSTCBHighRdy->OSTCBStkPtr         */

      MOVEM.L    (A7),D0-D7/A0-A6     	  /* Restore ALL CPU registers from new task stack */
      LEA        60(A7),A7

      RTE                                 /* Run task                                      */

/*
;*******************************************************************************************
;                                        GENERIC ISR
;
; Description : This function shows how to write ISRs
;
; Arguments   : none
;
; Notes       : 1) You MUST save ALL the CPU registers as shown below
;               2) You MUST increment 'OSIntNesting' and NOT call OSIntEnter()
;               3) You MUST JUMP to OSIntExitCF() instead of call the function.
;               4) You MUST NOT use OSIntExit() to exit an ISR with the MCF5275.
;*******************************************************************************************
*/

_OS_My_ISR:
      MOVE.W     #0x2700,SR               /* Disable interrupts                            */

      LEA        -60(A7),A7               /* Save processor registers onto stack           */
      MOVEM.L    D0-D7/A0-A6,(A7)

      MOVEQ.L    #0,D0                    /* OSIntNesting++                                */
      MOVE.L     (_OSIntNesting),D0
      ADDQ.L     #1,D0
      MOVE.L     D0,(_OSIntNesting)

      CMPI.L     #1, D0                   /* if (OSIntNesting == 1)                        */
      BNE        _OS_My_ISR_1
      MOVE.L     (_OSTCBCur), A1          /*     OSTCBCur-<OSTCBStkPtr = SP                */
      MOVE.L     A7,(A1)

_OS_My_ISR_1:

      JSR        _OS_My_ISR_Handler       /* OS_My_ISR_Handler()                           */

      JSR        _OSIntExit               /* Exit the ISR                                  */

      MOVEM.L    (A7),D0-D7/A0-A6         /* Restore processor registers from stack        */
      LEA        60(A7),A7

      RTE                                 /* Return to task or nested ISR                  */


_OS_My_ISR_Handler:
      RTS

.end
