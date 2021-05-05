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
;                                            Fujitsu FR Port
;
; Filename : os_cpu_a.asm
; Version  : V2.93.01
;********************************************************************************************************


;*********************************************************************************************************
;                                          GLOCAL REFERENCES
;*********************************************************************************************************

       .PROGRAM     OS_CPU_A

       .EXPORT      _OS_CPU_SR_Save
       .EXPORT      _OS_CPU_SR_Restore

       .EXPORT      _OSStartHighRdy
       .EXPORT      _OSCtxSw
       .EXPORT      _OSIntCtxSw

       .IMPORT      _OSTaskSwHook
       .IMPORT      _OSPrioHighRdy
       .IMPORT      _OSPrioCur
       .IMPORT      _OSRunning
       .IMPORT      _OSTCBCur
       .IMPORT      _OSTCBHighRdy

;*********************************************************************************************************
;                                               INCLUDES
;*********************************************************************************************************

#include            "os_cpu_i.asm"

        .section    CODE, code, align=4


;*********************************************************************************************************
;                                   CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
;              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
;              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
;              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
;              into the CPU's status register.
;
; Prototypes :     OS_CPU_SR  OS_CPU_SR_Save(void);
;                  void       OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
;
;
; Note(s)    : 1) These functions are used in general like this:
;
;                 void Task (void *p_arg)
;                 {
;                 #if OS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register */
;                     OS_CPU_SR  cpu_sr;
;                 #endif
;
;                          :
;                          :
;                     OS_ENTER_CRITICAL();             /* cpu_sr = OS_CPU_SaveSR();                */
;                          :
;                          :
;                     OS_EXIT_CRITICAL();              /* OS_CPU_RestoreSR(cpu_sr);                */
;                          :
;                          :
;                 }
;*********************************************************************************************************


_OS_CPU_SR_Save:
        MOV         PS, R4                   ; Save state of PS
        ANDCCR      #0xEF                    ; Disable interrupts
        RET

_OS_CPU_SR_Restore:
        MOV         R4, PS                   ; Restore state of PS
        RET

;*********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; Note(s) : 1) OSStartHighRdy() MUST:
;              a) Call OSTaskSwHook() then,
;              b) Set OSRunning to TRUE,
;              c) Switch to the highest priority task.
;*********************************************************************************************************

_OSStartHighRdy:

        CALL        _OSTaskSwHook            ; Call user defined task switch hook

        LDI         #1, R1                   ; OSRunning = TRUE
        LDI         #_OSRunning, R0
        STB         R1, @R0

        LDI         #_OSTCBHighRdy, R12      ; Resume stack pointer
        LD          @R12, R13
        LD          @R13, R15

        POP_ALL                              ; Restore context of resisters

        RETI                                 ; Run task


;*********************************************************************************************************
;                         PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
;
; Note(s) : 1) OSCtxSw() is actually an Software Interrupt handler
;
;           2) The pseudo-code for OSCtxSw() is:
;              a) Save the current task's context onto the current task's stack
;              b) OSTCBCur->OSTCBStkPtr = SP;
;              c) OSTaskSwHook();
;              d) OSPrioCur = OSPrioHighRdy;
;              e) OSTCBCur  = OSTCBHighRdy;
;              f) SP        = OSTCBHighRdy->OSTCBStkPtr;
;              g) Restore the new task's context from the new task's stack
;              h) Return to new task's code
;
;           3) Upon entry:
;              OSTCBCur      points to the OS_TCB of the task to suspend
;              OSTCBHighRdy  points to the OS_TCB of the task to resume
;*********************************************************************************************************

_OSCtxSw:
        PUSH_ALL                             ; Save context of interrupted task

        LDI         #_OSTCBCur, R0           ; OSTCBCur->OSTCBStkPtr = SSP
        LD          @R0, R1
        ST          R15, @R1

        CALL        _OSTaskSwHook            ; Call user defined task switch hook

        LDI         #_OSPrioHighRdy, R0      ; OSPrioCur = OSPrioHighRdy
        LDUB        @R0, R3
        LDI         #_OSPrioCur, R1
        STB         R3, @R1

        LDI         #_OSTCBHighRdy, R0       ; OSTCBCur = OSTCBHighRdy
        LD          @R0, R2
        LDI         #_OSTCBCur, R1
        ST          R2, @R1
        LD          @R2, R15                 ; SSP = OSTCBHighRdy->OSTCBStkPtr

        POP_ALL                              ; Restore context of interrupted task

        RETI


;*********************************************************************************************************
;                   PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
;
; Note(s) : 1) OSIntCtxSw() is called in SYS mode with BOTH FIQ and IRQ interrupts DISABLED
;
;           2) The pseudo-code for OSCtxSw() is:
;              a) OSTaskSwHook();
;              b) OSPrioCur = OSPrioHighRdy;
;              c) OSTCBCur  = OSTCBHighRdy;
;              d) SP        = OSTCBHighRdy->OSTCBStkPtr;
;              e) Restore the new task's context from the new task's stack
;              f) Return to new task's code
;
;           3) Upon entry:
;              OSTCBCur      points to the OS_TCB of the task to suspend
;              OSTCBHighRdy  points to the OS_TCB of the task to resume
;*********************************************************************************************************

_OSIntCtxSw:
        CALL        _OSTaskSwHook            ; Call user defined task switch hook

        LDI         #_OSPrioHighRdy, R0      ; OSPrioCur = OSPrioHighRdy
        LDUB        @R0, R3
        LDI         #_OSPrioCur, R1
        STB         R3, @R1

        LDI         #_OSTCBHighRdy, R0       ; OSTCBCur = OSTCBHighRdy
        LD          @R0, R2
        LDI         #_OSTCBCur, R1
        ST          R2, @R1
        LD          @R2, R15                 ; SSP = OSTCBHighRdy->OSTCBStkPtr

        POP_ALL                              ; Restore context of interrupted task

        RETI

       .END
