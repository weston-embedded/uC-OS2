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
; Filename : os_cpu_a.s
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
;            3) Changed OSIntCtxSw for new uC/OS-II version 2.51 features. OSTickISR was also
;               changed to deal with the Stack Pointer in a different way than in the V2.04.
;               See the uC/OS-II release notes for details on how this is accomplished.
;********************************************************************************************************
*/

.equ     IMMADDR,    0x10000000
.equ     sTER0,     (IMMADDR+0x210)
.equ     REF_EVENT,  0x02


/*
;*************************************************************************************************
;                                       PUBLIC DECLARATIONS
;*************************************************************************************************
*/

        .global  OSCtxSw
        .global  OSIntCtxSw
        .global  OSIntExitCF
        .global  OSStartHighRdy
        .global  OSTickISR
        .global  OSInitVBR
        .global  OS_CPU_SR_Save
        .global  OS_CPU_SR_Restore

/*
;**************************************************************************************************
;                                     EXTERNAL DECLARATIONS
;**************************************************************************************************
*/

        .extern  OSCtxSwCtr
        .extern  OSIntExit
        .extern  OSIntNesting
        .extern  OSLockNesting
        .extern  OSPrioCur
        .extern  OSPrioHighRdy
        .extern  OSRdyGrp
        .extern  OSRdyTbl
        .extern  OSRunning
        .extern  OSTaskSwHook
        .extern  OSTCBCur
        .extern  OSTCBHighRdy
        .extern  OSTCBPrioTbl
        .extern  OSTimeTick
        .extern  OSUnMapTbl

        .extern  OSIntEnter

/*
;********************************************************************************************************
;                              OS_CPU_SR_Save() for OS_CRITICAL_METHOD #3
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
  .text

OS_CPU_SR_Save:
        MOVE    %SR,%D0                       /* Copy SR into D0                                       */
        MOVE.L  %D0,-(%A7)                    /* Save D0                                               */
        ORI.L   #0x0700,%D0                   /* Disable interrupts                                    */
        MOVE    %D0,%SR
        MOVE.L  (%A7)+,%D0                    /* Restore original state of SR                          */
        RTS

/*
;********************************************************************************************************
;                              OS_CPU_SR_Restore() for OS_CRITICAL_METHOD #3
;
; Description : This functions implements the OS_CRITICAL_METHOD #3 function to restore the state of the
;               interrupt flag.
;
; Arguments   : os_cpu_sr   is the contents of the SR to restore.  It is assumed that this 'argument' is
;                           passed in the D0 register of the CPU by the compiler.
;
; Returns     : None
;********************************************************************************************************
*/

OS_CPU_SR_Restore:
        MOVE    %D0,%SR
        RTS

/*
;*************************************************************************************************
;                              VECTOR BASE REGISTER INITIALIZATION
;
; This is to set the Vector Base Register to 0x00000000, in case the startup bootloader moved it
; somewhere else.
;*************************************************************************************************
*/

OSInitVBR:
        MOVE.L  #0x00000000, %D0
        MOVEC   %D0, %VBR
        RTS

/*
;**************************************************************************************************
;                            START HIGHEST PRIORITY TASK READY-TO-RUN
;
; Description: This function is called by OSStart() to start the highest priority task that was
;              created by your application before calling OSStart().
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
;*************************************************************************************************
*/

OSStartHighRdy:
       JSR      OSTaskSwHook              /* Invoke user defined context switch hook            */

       MOVEQ.L  #1, %D4                   /* OSRunning = TRUE;                                  */
       MOVE.B   %D4,OSRunning             /*   Indicates that we are multitasking               */

       MOVE.L   (OSTCBHighRdy),%A1        /* Point to TCB of highest priority task ready to run */
       MOVE.L   (%A1),%A7                 /* Get the stack pointer of the task to resume        */

       MOVEM.L  (%A7),%D0-%D7/%A0-%A6     /* Store all the regs                                 */
       LEA      60(%A7),%A7               /* Advance the stack pointer                          */

       RTE                                /* Return to task                                     */


/*
;*************************************************************************************************
;                                     TASK LEVEL CONTEXT SWITCH
;
; Description : This function is called when a task makes a higher priority task ready-to-run.
;
; Arguments   : none
;
; Note(s)     : 1) Upon entry,
;                   OSTCBCur     points to the OS_TCB of the task to suspend
;                   OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows (the registers for
;                  task to suspend need to be saved):
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
;*************************************************************************************************
*/

OSCtxSw:
       LEA      -60(%A7),%A7
       MOVEM.L  %D0-%D7/%A0-%A6,(%A7)     /* Save the registers of the current task             */

       MOVE.L   (OSTCBCur),%A1            /* Save the stack pointer in the suspended task TCB   */
       MOVE.L   %A7,(%A1)

       JSR      OSTaskSwHook              /* Invoke user defined context switch hook            */

       MOVE.L   (OSTCBHighRdy),%A1        /* OSTCBCur = OSTCBHighRdy                            */
       MOVE.L   %A1,(OSTCBCur)
       MOVE.L   (%A1),%A7                 /* Get the stack pointer of the task to resume        */

       MOVE.B   (OSPrioHighRdy),%D0       /* OSPrioCur = OSPrioHighRdy                          */
       MOVE.B   %D0,(OSPrioCur)

       MOVEM.L  (%A7),%D0-%D7/%A0-%A6     /* Restore the CPU registers                          */
       LEA      60(%A7),%A7

       RTE                                /* Run task                                           */

/*
;*************************************************************************************************
;                                 INTERRUPT LEVEL CONTEXT SWITCH
;
;
; Description : This function is provided for backward compatibility and to satisfy OSIntExit()
;               in OS_CORE.C.
;
; Arguments   : none
;*************************************************************************************************
*/

OSIntCtxSw:
      JSR       OSTaskSwHook              /* Invoke user defined context switch hook            */

      MOVE.L    (OSTCBHighRdy),%A1        /* OSTCBCur  = OSTCBHighRdy                           */
      MOVE.L    %A1,(OSTCBCur)
      MOVE.L    (%A1),%A7                 /* SP        = OSTCBHighRdy->OSTCBStkPtr              */

      MOVE.B    (OSPrioHighRdy),%D0       /* OSPrioCur = OSPrioHighRdy                          */
      MOVE.B    %D0,(OSPrioCur)

      MOVEM.L   (%A7),%D0-%D7/%A0-%A6     /* Restore ALL CPU registers from new task's stack    */
      LEA       60(%A7),%A7

      RTE                                 /* Run task                                           */

/*
;*******************************************************************************************************
;                                             ISR EXIT
;
; Note(s) : 1) This function MUST be JUMPed to instead of calling OSIntExit() at the end of an ISR.
;*******************************************************************************************************
*/

OSIntExitCF:
      MOVE      %SR,%D0                   /* Disable interrupts                                                  */
      ORI.L     #0x0700,%D0
      MOVE      %D0,%SR

      MOVE.B    (OSIntNesting),%D0        /* OSIntNesting--                                                      */
      ANDI.L    #0x00FF,%D0
      SUBQ.L    #1,%D0
      MOVE.B    %D0,(OSIntNesting)

      MOVE.L    (60,%A7),%D0              /*  if (LAST nested ISR) {                                             */
      ANDI.L    #0x0700,%D0
      BNE       OSIntExitCF_Exit

      TST.B     OSLockNesting             /*     if (OSLockNesting == 0)                                         */
      BNE       OSIntExitCF_Exit

      MOVEQ.L   #0,%D0                    /*         y = OSUnMapTbl[OSRdyGrp];                                   */
      MOVE.B    OSRdyGrp,%D0
      LEA       OSUnMapTbl,%A0
      MOVE.B    (%A0,%D0.L),%D3           /*         y is in D3                                                  */

      MOVEQ.L   #0,%D0
      MOVE.B    %D3,%D0                   /*         copy y into D0                                              */
      LEA       OSRdyTbl,%A0
      MOVEQ.L   #0,%D1
      MOVE.B    (%A0,%D0.L),%D1           /*         D1            = OSRdyTbl[y]                                 */
      LEA       OSUnMapTbl,%A0            /*         OSPrioHighRdy = (INT8U)((y << 3) + OSUnMapTbl[OSRdyTbl[y]]) */
      MOVE.B    %D3,%D0
      LSL.L     #3,%D0                    /*         D0            = (y << 3)                                    */
      MOVE.B    (%A0,%D1.L),%D1           /*         D1            = OSUnMapTbl[OSRdyTbl[y]]                     */
      ADD.L     %D1,%D0
      MOVE.B    %D0,OSPrioHighRdy

      MOVE.B    OSPrioHighRdy,%D0
      MOVE.B    OSPrioCur,%D1
      ANDI.L    #0xFF,%D1
      ANDI.L    #0xFF,%D0
      CMP.L     %D1,%D0                   /*         if (OSPrioHighRdy != OSPrioCur) {                           */
      BEQ       OSIntExitCF_Exit

      MOVEQ.L   #0,%D0                    /*             OSTCBHighRdy = OSTCBPrioTbl[OSPrioHighRdy];             */
      MOVE.B    OSPrioHighRdy,%D0
      LEA       OSTCBPrioTbl,%A0
      MOVEA.L   (%A0,%D0.L*4),%A0
      MOVE.L    %A0,OSTCBHighRdy

      ADDQ.L    #1,OSCtxSwCtr             /*             OSCtxSwCtr++;                                           */

                                          /*             PERFORM INTERRUPT LEVEL CONTEXT SWITCH:                 */
      JSR       OSTaskSwHook              /*             Invoke user defined context switch hook                 */

      MOVE.L    (OSTCBHighRdy),%A1        /*             OSTCBCur          = OSTCBHighRdy                        */
      MOVE.L    %A1,(OSTCBCur)
      MOVE.L    (%A1),%A7                 /*             CPU stack pointer = OSTCBHighRdy->OSTCBStkPtr           */

      MOVE.B    (OSPrioHighRdy),%D0       /*             OSPrioCur         = OSPrioHighRdy                       */
      MOVE.B    %D0,(OSPrioCur)

OSIntExitCF_Exit:
      MOVEM.L   (%A7),%D0-%D7/%A0-%A6     /* Restore processor registers from stack                              */
      LEA       60(%A7),%A7
      RTE                                 /* Return to task or nested ISR                                        */


/*
;*******************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.
;
;               This function is installed as the timer tick ISR and called each time TIMER0 on the
;               MCF5272 reaches it's reference value.
;
; Arguments   : none
;
; Notes       : 1) You MUST save ALL the CPU registers as shown below
;               2) You MUST increment 'OSIntNesting' and NOT call OSIntEnter()
;               3) You MUST JUMP to OSIntExitCF() instead of call the function.
;               4) You MUST NOT use OSIntExit() to exit an ISR with the MCF5272.
;*******************************************************************************************************
*/

OSTickISR:
      LEA       -60(%A7),%A7              /* Save processor registers onto stack                      */
      MOVEM.L   %D0-%D7/%A0-%A6,(%A7)

      MOVE      %SR, %D0                  /* Disable interrupts                                       */
      ORI.L     #0x0700,%D0
      MOVE      %D0, %SR

      MOVEQ.L   #0,%D0                    /* OSIntNesting++                                           */
      MOVE.B    (OSIntNesting),%D0
      ADDQ.L    #1,%D0
      MOVE.B    %D0,(OSIntNesting)

      CMPI.L    #1, %D0                   /* if (OSIntNesting == 1)                                   */
      BNE       OSTickISR_1
      MOVE.L    (OSTCBCur), %A1           /*     OSTCBCur-<OSTCBStkPtr = SP                           */
      MOVE.L    %A7,(%A1)

OSTickISR_1:
      MOVEA.L   #sTER0,%A0                /* Clear the reference event                                */
      MOVE.W    (%A0),%D0                 /* Again a point of note, since ORI only works with         */
      ORI.L     #REF_EVENT,%D0            /* data registers you must do the copy unlike 68k which     */
      MOVE.W    %D0,(%A0)                 /* has more assembly language options.                      */

      JSR       OSTimeTick                /* Tell uC/OS-II that a tick has occurred                   */
      JMP       OSIntExitCF               /* Exit the ISR                                             */
