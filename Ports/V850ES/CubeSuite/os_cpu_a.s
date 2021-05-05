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
# Toolchain : CubeSuite V1.20
#             CA850 compiler
#********************************************************************************************************

.include "os_cpu_a.inc"

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
    .extern  _OSTimeTick
                                                                -- Functions declared in this file
    .globl   _OS_CPU_SR_Save
    .globl   _OS_CPU_SR_Restore
    .globl   _OS_CPU_TickHandler
    .globl   _OSStartHighRdy
    .globl   _OSIntCtxSw
    .globl   _OSCtxSw

#********************************************************************************************************
#                                      INTERRUPT VECTOR ENTRY FOR TRAP 00
#********************************************************************************************************

    .section "TRAP00", text                                     -- TRAP instruction for Context Switching
    .globl    __trap00
__trap00:
    jr _OSCtxSw

#********************************************************************************************************
#                                      CODE GENERATION DIRECTIVES
#********************************************************************************************************

    .text
    .align  4

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

    OS_CTX_RESTORE  sp                                          -- Restore Task Context

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
    OS_CTX_SAVE sp                                              -- Save current Task context

    mov   #_OSTCBCur, r11                                       -- OSTCBCur->OSTCBStkPtr = SP;
    ld.w  0[r11]    , r11
    st.w  sp        , 0[r11]

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

    OS_CTX_RESTORE sp                                           -- Restore new Task's context

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

    OS_CTX_RESTORE sp

    reti                                                        -- Return from interrupt starts new task


#********************************************************************************************************
#                                        OS_CPU_TickHandler
#
# Note(s) : 1) The pseudo-code for _OS_CPU_TickHandler() is:
#              a) Save processor registers;
#              b) Increment OSIntNestingCtr;
#              c) if (OSIntNestingCtr == 1) {
#                     OSTCBCurPtr->OSTCBStkPtr = SP;
#                 }
#              d) Call OSTimeTick();
#              e) Call OSIntExit();
#              f) Restore processosr Registers;
#
#           2) OS_CPU_TickHandler() must be registered in the proper vector address of timer that will be
#              used as the tick.
#
#           3) All the other ISRs must have the following implementation to secure proper register saving &
#              restoring when servicing an interrupt
#
#              MyISR
#                  OS_ISR_ENTER
#                  ISR Body here
#                  OS_ISR_EXIT
#********************************************************************************************************

_OS_CPU_TickHandler:
    OS_ISR_ENTER

    jarl _OSTimeTick, lp                                        -- Call OSTimeTick();

    OS_ISR_EXIT


#********************************************************************************************************
#                                     ASSEMBLY LANGUAGE PORT FILE END
#********************************************************************************************************
