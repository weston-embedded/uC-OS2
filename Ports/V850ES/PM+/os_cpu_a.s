#********************************************************************************************************
#                                              uC/OS-II
#                                        The Real-Time Kernel
#
#                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
#
#                                 SPDX-License-Identifier: APACHE-2.0
#
#               This software is subject to an open source license and is distributed by
#                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
#                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
#
#********************************************************************************************************

#********************************************************************************************************
#
#                                          Renesas V850ES Port
#
# Filename  : os_cpu_a.s
# Version   : V2.93.01
#********************************************************************************************************
# For       : Renesas V850ES
# Toolchain : PM+ v6.32
#             CA850 v3.44 compiler
#********************************************************************************************************

#********************************************************************************************************
#                                           PUBLIC FUNCTIONS
#********************************************************************************************************
                                                                -- External References
    .extern  _OSRunning
    .extern  _OSPrioCur
    .extern  _OSPrioHighRdy
    .extern  _OSTCBCur
    .extern  _OSTCBHighRdy
    .extern  _OSTaskSwHook
    .extern  _OSIntNesting
    .extern  _OSIntExit
    .extern  _OS_CPU_IntHandlerSrc
                                                                -- Functions declared in this file
    .globl   _OS_CPU_SR_Save
    .globl   _OS_CPU_SR_Restore
    .globl   _OS_CPU_IntHandler
    .globl   _OSStartHighRdy
    .globl   _OSIntCtxSw
    .globl   _OSCtxSw

#********************************************************************************************************
#                                                EQUATES
#********************************************************************************************************

                                                                -- SYSTEM REGISTER
    .set    EIPC , 0
    .set    EIPSW, 1
    .set    ECR  , 4
    .set    PSW  , 5
    .set    CTPC , 16
    .set    CTPSW, 17

                                                                -- PROGRAM REGISTER STACK OFFSET
    .set    STK_OFFSET_R1   , 0
    .set    STK_OFFSET_R2   , STK_OFFSET_R1    + 4
    .set    STK_OFFSET_R6   , STK_OFFSET_R2    + 4
    .set    STK_OFFSET_R7   , STK_OFFSET_R6    + 4
    .set    STK_OFFSET_R8   , STK_OFFSET_R7    + 4
    .set    STK_OFFSET_R9   , STK_OFFSET_R8    + 4
    .set    STK_OFFSET_R10  , STK_OFFSET_R9    + 4
    .set    STK_OFFSET_R11  , STK_OFFSET_R10   + 4
    .set    STK_OFFSET_R12  , STK_OFFSET_R11   + 4
    .set    STK_OFFSET_R13  , STK_OFFSET_R12   + 4
    .set    STK_OFFSET_R14  , STK_OFFSET_R13   + 4
    .set    STK_OFFSET_R15  , STK_OFFSET_R14   + 4
    .set    STK_OFFSET_R16  , STK_OFFSET_R15   + 4
    .set    STK_OFFSET_R17  , STK_OFFSET_R16   + 4
    .set    STK_OFFSET_R18  , STK_OFFSET_R17   + 4
    .set    STK_OFFSET_R19  , STK_OFFSET_R18   + 4
    .set    STK_OFFSET_R20  , STK_OFFSET_R19   + 4
    .set    STK_OFFSET_R21  , STK_OFFSET_R20   + 4
    .set    STK_OFFSET_R22  , STK_OFFSET_R21   + 4
    .set    STK_OFFSET_R23  , STK_OFFSET_R22   + 4
    .set    STK_OFFSET_R24  , STK_OFFSET_R23   + 4
    .set    STK_OFFSET_R25  , STK_OFFSET_R24   + 4
    .set    STK_OFFSET_R26  , STK_OFFSET_R25   + 4
    .set    STK_OFFSET_R27  , STK_OFFSET_R26   + 4
    .set    STK_OFFSET_R28  , STK_OFFSET_R27   + 4
    .set    STK_OFFSET_R29  , STK_OFFSET_R28   + 4
    .set    STK_OFFSET_R30  , STK_OFFSET_R29   + 4
    .set    STK_OFFSET_R31  , STK_OFFSET_R30   + 4
    .set    STK_OFFSET_EIPC , STK_OFFSET_R31   + 4
                                                                -- SYSTEM REGISTER STACK OFFSET
    .set    STK_OFFSET_EIPSW, STK_OFFSET_EIPC  + 4
    .set    STK_OFFSET_CTPC , STK_OFFSET_EIPSW + 4
    .set    STK_OFFSET_CTPSW, STK_OFFSET_CTPC  + 4
    .set    STK_CTX_SIZE    , STK_OFFSET_CTPSW + 4

#********************************************************************************************************
#                                      INTERRUPT VECTOR ENTRY FOR TRAP 00
#********************************************************************************************************

    .section "TRAP00", text                                     -- TRAP instruction for Context Switching
    .globl	__trap00
__trap00:
    jr _OSCtxSw

#********************************************************************************************************
#                                      CODE GENERATION DIRECTIVES
#********************************************************************************************************

    .text
    .align  4

#********************************************************************************************************
#                                            MACRO DEFINITIONS
#********************************************************************************************************

                                                                -- RESTORE PROCESSOR REGISTER
.macro	POPALL	SP
	ld.w  STK_OFFSET_R1[SP] , r1
	ld.w  STK_OFFSET_R6[SP] , r6
	ld.w  STK_OFFSET_R7[SP] , r7
	ld.w  STK_OFFSET_R8[SP] , r8
	ld.w  STK_OFFSET_R9[SP] , r9
	ld.w  STK_OFFSET_R10[SP], r10
	ld.w  STK_OFFSET_R11[SP], r11
	ld.w  STK_OFFSET_R12[SP], r12
	ld.w  STK_OFFSET_R13[SP], r13
	ld.w  STK_OFFSET_R14[SP], r14
	ld.w  STK_OFFSET_R15[SP], r15
	ld.w  STK_OFFSET_R16[SP], r16
	ld.w  STK_OFFSET_R17[SP], r17
	ld.w  STK_OFFSET_R18[SP], r18
	ld.w  STK_OFFSET_R19[SP], r19
	ld.w  STK_OFFSET_R20[SP], r20
	ld.w  STK_OFFSET_R21[SP], r21
	ld.w  STK_OFFSET_R22[SP], r22
	ld.w  STK_OFFSET_R23[SP], r23
	ld.w  STK_OFFSET_R24[SP], r24
	ld.w  STK_OFFSET_R25[SP], r25
	ld.w  STK_OFFSET_R26[SP], r26
	ld.w  STK_OFFSET_R27[SP], r27
	ld.w  STK_OFFSET_R28[SP], r28
	ld.w  STK_OFFSET_R29[SP], r29
	ld.w  STK_OFFSET_R30[SP], r30
	ld.w  STK_OFFSET_R31[SP], r31

	ld.w  STK_OFFSET_EIPSW[SP], r2                              -- Restore task's EIPSW
	ldsr  r2, EIPSW

	ld.w  STK_OFFSET_EIPC[SP], r2                               -- Restore task's EIPC
	ldsr  r2, EIPC

	ld.w  STK_OFFSET_CTPC[SP], r2                               -- Restore task's CTPC
	ldsr  r2, CTPC

	ld.w  STK_OFFSET_CTPSW[SP], r2                              -- Restore task's CTPSW
	ldsr  r2, CTPSW

	ld.w  STK_OFFSET_R2[SP] , r2

	addi STK_CTX_SIZE, SP, SP                                   -- Adjust the Stack Pointer
.endm

	                                                            -- SAVE PROCESSOR REGISTER
.macro	PUSHALL   SP
   	addi  -STK_CTX_SIZE, SP, SP                                 -- Adjust the Stack Pointer

	st.w   r1, STK_OFFSET_R1[SP]
	st.w   r2, STK_OFFSET_R2[SP]
	st.w   r6, STK_OFFSET_R6[SP]
	st.w   r7, STK_OFFSET_R7[SP]
	st.w   r8, STK_OFFSET_R8[SP]
	st.w   r9, STK_OFFSET_R9[SP]
	st.w  r10, STK_OFFSET_R10[SP]
	st.w  r11, STK_OFFSET_R11[SP]
	st.w  r12, STK_OFFSET_R12[SP]
	st.w  r13, STK_OFFSET_R13[SP]
	st.w  r14, STK_OFFSET_R14[SP]
	st.w  r15, STK_OFFSET_R15[SP]
	st.w  r16, STK_OFFSET_R16[SP]
	st.w  r17, STK_OFFSET_R17[SP]
	st.w  r18, STK_OFFSET_R18[SP]
	st.w  r19, STK_OFFSET_R19[SP]
	st.w  r20, STK_OFFSET_R20[SP]
	st.w  r21, STK_OFFSET_R21[SP]
	st.w  r22, STK_OFFSET_R22[SP]
	st.w  r23, STK_OFFSET_R23[SP]
	st.w  r24, STK_OFFSET_R24[SP]
	st.w  r25, STK_OFFSET_R25[SP]
	st.w  r26, STK_OFFSET_R26[SP]
	st.w  r27, STK_OFFSET_R27[SP]
	st.w  r28, STK_OFFSET_R28[SP]
	st.w  r29, STK_OFFSET_R29[SP]
	st.w  r30, STK_OFFSET_R30[SP]
	st.w  r31, STK_OFFSET_R31[SP]

	stsr  EIPC, r2
	st.w  r2, STK_OFFSET_EIPC[SP]                               -- Restore task's EIPC

	stsr  EIPSW, r2
	st.w  r2, STK_OFFSET_EIPSW[SP]                              -- Restore task's EIPSW

	stsr  CTPC, r2
	st.w  r2, STK_OFFSET_CTPC[SP]                               -- Restore task's CTPC

	stsr  CTPSW, r2
	st.w  r2, STK_OFFSET_CTPSW[SP]                              -- Restore task's CTPSW
.endm

#********************************************************************************************************
#                                  CRITICAL SECTION METHOD 3 FUNCTIONS
#
# Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
#              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
#              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
#              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
#              into the CPU's status register.
#
# Prototypes : OS_CPU_SR  OS_CPU_SR_Save    (void);
#              void       OS_CPU_SR_Restore (OS_CPU_SR  os_cpu_sr);
#
#
# Note(s)    : (1) These functions are used in general like this:
#
#                 void Task (void  *p_arg)
#                 {
#                                                               /* Allocate storage for CPU status register.            */
#                 #if (OS_CRITICAL_METHOD == 3)
#                      OS_CPU_SR  os_cpu_sr;
#                 #endif
#
#                          :
#                          :
#                      OS_ENTER_CRITICAL();                     /* os_cpu_sr = OS_CPU_SR_Save();                        */
#                          :
#                          :
#                      OS_EXIT_CRITICAL();                      /* OS_CPU_SR_Restore(cpu_sr);                           */
#                          :
#                          :
#                 }
#********************************************************************************************************

_OS_CPU_SR_Save:
	stsr  PSW, r10                                              -- Save PSW
	di                                                          -- Disable interrupts
	jmp   [lp]

_OS_CPU_SR_Restore:
	ldsr  r6, PSW                                               -- Restore PSW
	jmp   [lp]

#********************************************************************************************************
#                                           START MULTITASKING
#                                       void OSStartHighRdy(void)
#
# Note(s) : 1) OSStartHighRdy() MUST:
#              a) Call OSTaskSwHook() then,
#              b) Set OSRunning to TRUE,
#              c) Switch to the highest priority task.
#
#********************************************************************************************************

_OSStartHighRdy:
	jarl  _OSTaskSwHook, lp                                     -- Call OSTaskSwHook();

	mov   0x01, r2                                              -- OSRunning = TRUE;
	st.b  r2  , #_OSRunning[r0]

  	mov   #_OSTCBHighRdy, r11                                   -- SWITCH TO HIGHEST PRIORITY TASK:
	ld.w  0[r11]        , r11
	ld.w  0[r11]        , sp

	POPALL sp                                                   -- Restore Task Context

	reti

#********************************************************************************************************
#                         PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
#
# Note(s) : 1) The pseudo-code for OSCtxSw() is:
#              a) Save the current task's context onto the current task's stack,
#              b) OSTCBCur->OSTCBStkPtr = sp;
#              c) OSTaskSwHook();
#              d) OSPrioCur             = OSPrioHighRdy;
#              e) OSTCBCur              = OSTCBHighRdy;
#              f) SP                    = OSTCBHighRdy->OSTCBStkPtr;
#              g) Restore the new task's context from the new task's stack,
#              h) Return to new task's code.
#
#           2) Upon entry:
#              OSTCBCur      points to the OS_TCB of the task to suspend,
#              OSTCBHighRdy  points to the OS_TCB of the task to resume.
#********************************************************************************************************

_OSCtxSw:
	PUSHALL sp                                                  -- Save current Task context

	mov   #_OSTCBCur, r11                                       -- OSTCBCur->OSTCBStkPtr = SP;
	ld.w  0[r11]	, r11
	st.w  sp	    , 0[r11]

	jarl  _OSTaskSwHook, lp                                     -- OSTaskSwHook();

   	mov   #_OSPrioHighRdy, r11                                  -- OSPrioCur = OSPrioHighRdy;
   	ld.b  0[r11]	     , r12
	mov   #_OSPrioCur    , r11
	st.b  r12            , 0[r11]

	mov   #_OSTCBHighRdy, r11                                   -- OSTCBCur = OSTCBHighRdy;
	ld.w  0[r11]	    , r12
	mov   #_OSTCBCur    , r11
	st.w  r12           , 0[r11]

	ld.w  0[r12], sp                                            -- SP = OSTCBHighRdy->OSTCBStkPtr;

	POPALL sp                                                   -- Restore new Task's context

	reti                                                        -- return from trap

#********************************************************************************************************
#                     PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
#
# Note(s) : 1) The pseudo-code for OSIntCtxSw() is:
#              a) OSTaskSwHook();
#              b) OSPrioCur             = OSPrioHighRdy;
#              c) OSTCBCur              = OSTCBHighRdy;
#              d) SP                    = OSTCBHighRdy->OSTCBStkPtr;
#              e) Restore the new task's context from the new task's stack,
#              f) Return to new task's code.
#
#           2) Upon entry:
#              OSTCBCur      points to the OS_TCB of the task to suspend,
#              OSTCBHighRdy  points to the OS_TCB of the task to resume.
#********************************************************************************************************

_OSIntCtxSw:
	jarl  _OSTaskSwHook, lp                                     -- OSTaskSwHook();

	mov   #_OSPrioHighRdy, r11                                  -- OSPrioCur = OSPrioHighRdy;
	ld.b  0[r11]         , r12
	mov   #_OSPrioCur    , r11
	st.b  r12            , 0[r11]

	mov   #_OSTCBHighRdy, r11                                   -- OSTCBCur = OSTCBHighRdy;
	ld.w  0[r11]        , r12
	mov   #_OSTCBCur    , r11
	st.w  r12           , 0[r11]

	ld.w  0[r12], sp                                            -- SP = OSTCBHighRdy->OSTCBStkPtr;

	POPALL sp

	reti                                                        -- Return from interrupt starts new task

#********************************************************************************************************
#                                        INTERRUPT/EXCEPTION  HANDLER
#
# Note(s) : 1) The pseudo-code for OS_CPU_IntHandler() is:
#              a) Save processor registers;
#              b) Increment OSIntNesting;
#              c) if (OSIntNesting == 1) {
#                     OSTCBCur->OSTCBStkPtr = SP;
#                 }
#              d) Call OSIntExit();
#              e) Call OS_CPU_IntHandlerSrc();
#              f) Restore processosr Registers;
#
#           2) OS_CPU_IntHandlerSrc() must be implemented to handle maskable interrupts according to
#              to exception code provided by the system register ECR.
#
#********************************************************************************************************

_OS_CPU_IntHandler:
	PUSHALL sp                                                  -- Saves Processor registers

	ld.b #_OSIntNesting[r0], r2                                 -- increment OSIntNesting
	add 0x1, r2
	st.b r2, #_OSIntNesting[r0]

	cmp  0x1, r2
	bne  _OS_CPU_IntHandler01                                   -- if (OSIntNesting == 1) {

	mov   #_OSTCBCur, r11                                       --     OSTCBCur->OSTCBStkPtr = SP;
	ld.w  0[r11]    , r11
	st.w  sp        , 0[r11]                                    -- }

_OS_CPU_IntHandler01:
	stsr ECR, r6                                                -- get interrupt/exception source code that occured.
	jarl _OS_CPU_IntHandlerSrc, lp

	jarl _OSIntExit, lp

	POPALL sp                                                   -- Restore processor register

	reti
