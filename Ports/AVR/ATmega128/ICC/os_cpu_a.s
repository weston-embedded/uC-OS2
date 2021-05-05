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
;                                       ATmega128  Specific code
;                                           ICC AVR Compiler
;
; Filename : os_cpu_a.s
; Version  : V2.93.01
;********************************************************************************************************

                .include  "os_cpu_i.h"
                .area     text(rel)


;********************************************************************************************************
;                            DISABLE/ENABLE INTERRUPTS USING OS_CRITICAL_METHOD #3
;
; Description : These functions are used to disable and enable interrupts using OS_CRITICAL_METHOD #3.
;
;               OS_CPU_SR  OSCPUSaveSR (void)
;                     Get current value of SREG
;                     Disable interrupts
;                     Return original value of SREG
;
;               void  OSCPURestoreSR (OS_CPU_SR cpu_sr)
;                     Set SREG to cpu_sr
;                     Return
;********************************************************************************************************

_OS_CPU_SR_Save::
                IN      R16,SREG                    ; Get current state of interrupts disable flag
                CLI                                 ; Disable interrupts
                RET                                 ; Return original SREG value in R16


_OS_CPU_SR_Restore::
                OUT     SREG,R16                    ; Restore SREG
                RET                                 ; Return


;********************************************************************************************************
;                               START HIGHEST PRIORITY TASK READY-TO-RUN
;
; Description : This function is called by OSStart() to start the highest priority task that was created
;               by your application before calling OSStart().
;
; Note(s)     : 1) The (data)stack frame is assumed to look as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr --> SPL of (return) stack pointer           (Low memory)
;                                                SPH of (return) stack pointer
;                                                Flags to load in status register
;                                                RAMPZ
;                                                R31
;                                                R30
;                                                R27
;                                                .
;                                                .
;                                                R0
;                                                PCH
;                                                PCL                                     (High memory)
;
;                  where the stack pointer points to the task start address.
;
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task.
;********************************************************************************************************

_OSStartHighRdy::
                CALL    _OSTaskSwHook               ; Invoke user defined context switch hook
                LDS     R16,_OSRunning              ; Indicate that we are multitasking
                INC     R16                         ;
                STS     _OSRunning,R16              ;

                LDS     R30,_OSTCBHighRdy           ; Let Z point to TCB of highest priority task
                LDS     R31,_OSTCBHighRdy+1         ; ready to run
                LD      R28,Z+                      ; Load Y (R29:R28) pointer
                LD      R29,Z+                      ;

                POP_SP                              ; Restore stack pointer
                POP_SREG_INT                        ; Restore status register (Disable Interrupts)
                POP_ALL                             ; Restore all registers
                RETI                                ; Start task


;********************************************************************************************************
;                                       TASK LEVEL CONTEXT SWITCH
;
; Description : This function is called when a task makes a higher priority task ready-to-run.
;
; Note(s)     : 1) Upon entry,
;                  OSTCBCur     points to the OS_TCB of the task to suspend
;                  OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows:
;
;                                       SP+0 --> LSB of task code address
;                                         +1     MSB of task code address                (High memory)
;
;               3) The saved context of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr --> SPL of (return) stack pointer           (Low memory)
;                                                SPH of (return) stack pointer
;                                                Flags to load in status register
;                                                RAMPZ
;                                                R31
;                                                R30
;                                                R27
;                                                .
;                                                .
;                                                R0
;                                                PCH
;                                                PCL                                     (High memory)
;********************************************************************************************************

_OSCtxSw::
                PUSH_ALL                            ; Save current task's context
                PUSH_SREG
                PUSH_SP

                LDS     R30,_OSTCBCur               ; Z = OSTCBCur->OSTCBStkPtr
                LDS     R31,_OSTCBCur+1             ;
                ST      Z+,R28                      ; Save Y (R29:R28) pointer
                ST      Z+,R29                      ;

                CALL    _OSTaskSwHook               ; Call user defined task switch hook

                LDS     R16,_OSPrioHighRdy          ; OSPrioCur = OSPrioHighRdy
                STS     _OSPrioCur,R16

                LDS     R30,_OSTCBHighRdy           ; Let Z point to TCB of highest priority task
                LDS     R31,_OSTCBHighRdy+1         ; ready to run
                STS     _OSTCBCur,R30               ; OSTCBCur = OSTCBHighRdy
                STS     _OSTCBCur+1,R31             ;

                LD      R28,Z+                      ; Restore Y pointer
                LD      R29,Z+                      ;

                POP_SP                              ; Restore stack pointer
                LD      R16,Y+                      ; Restore status register
                SBRC    R16,7                       ; Skip next instruction if interrupts DISABLED
                RJMP    _OSCtxSw_1

                OUT     SREG,R16                    ; Interrupts of task to return to are DISABLED
                POP_ALL
                RET

_OSCtxSw_1:     CBR     R16,BIT07                   ; Interrupts of task to return to are ENABLED
                OUT     SREG,R16
                POP_ALL                             ; Restore all registers
                RETI


;*********************************************************************************************************
;                                INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is called by OSIntExit() to perform a context switch to a task that has
;               been made ready-to-run by an ISR.
;
; Note(s)     : 1) Upon entry,
;                  OSTCBCur     points to the OS_TCB of the task to suspend
;                  OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows:
;
;                  OSTCBCur->OSTCBStkPtr ------> SPL of (return) stack pointer           (Low memory)
;                                                SPH of (return) stack pointer
;                                                Flags to load in status register
;                                                RAMPZ
;                                                R31
;                                                R30
;                                                R27
;                                                .
;                                                .
;                                                R0
;                                                PCH
;                                                PCL                                     (High memory)
;
;               3) The saved context of the task to resume looks as follows:
;
;                  OSTCBHighRdy->OSTCBStkPtr --> SPL of (return) stack pointer           (Low memory)
;                                                SPH of (return) stack pointer
;                                                Flags to load in status register
;                                                RAMPZ
;                                                R31
;                                                R30
;                                                R27
;                                                .
;                                                .
;                                                R0
;                                                PCH
;                                                PCL                                     (High memory)
;*********************************************************************************************************

_OSIntCtxSw::
                CALL    _OSTaskSwHook               ; Call user defined task switch hook

                LDS     R16,_OSPrioHighRdy          ; OSPrioCur = OSPrioHighRdy
                STS     _OSPrioCur,R16              ;

                LDS     R30,_OSTCBHighRdy           ; Z = OSTCBHighRdy->OSTCBStkPtr
                LDS     R31,_OSTCBHighRdy+1         ;
                STS     _OSTCBCur,R30               ; OSTCBCur = OSTCBHighRdy
                STS     _OSTCBCur+1,R31             ;

                LD      R28,Z+                      ; Restore Y pointer
                LD      R29,Z+                      ;

                POP_SP                              ; Restore stack pointer
                LD      R16,Y+                      ; Restore status register
                SBRC    R16,7                       ; Skip next instruction if interrupts DISABLED
                RJMP    _OSIntCtxSw_1

                OUT     SREG,R16                    ; Interrupts of task to return to are DISABLED
                POP_ALL
                RET

_OSIntCtxSw_1:  CBR     R16,BIT07                   ; Interrupts of task to return to are ENABLED
                OUT     SREG,R16
                POP_ALL                             ; Restore all registers
                RETI
