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
;                                        ATmega128 Specific code
;                                           GNU AVR Compiler
;
; Filename : os_cpu_i.h
; Version  : V2.93.01
;********************************************************************************************************

;********************************************************************************************************
;                                            BIT DEFINITIONS
;********************************************************************************************************

BIT00   = 0x01
BIT01   = 0x02
BIT02   = 0x04
BIT03   = 0x08
BIT04   = 0x10
BIT05   = 0x20
BIT06   = 0x40
BIT07   = 0x80

;********************************************************************************************************
;                                           I/O PORT ADDRESSES
;********************************************************************************************************

SREG    = 0x3F
SPH     = 0x3E
SPL     = 0x3D
EIND    = 0x3C
RAMPZ   = 0x3B
RAMPY   = 0x3A
RAMPX   = 0x39
RAMPD   = 0x38

;********************************************************************************************************
;                                         MACROS
;********************************************************************************************************

;*
;*********************************************************************************************************
;*                               PUSH ALL THE REGISTER INTO THE STACK
;*
;* Description : Save all the register into the stack.
;*                           +-------+
;*             [RO..RAMPZ] ->|  RO   |
;*                           |  SREG |
;*                           |  R1   |
;*                           |  R2   |
;*                           |   .   |
;*                           |   .   |
;*                           | RAMPZ |
;*                           +-------+
;*********************************************************************************************************
;*

               .macro  PUSH_ALL                            ; Save all registers
	            PUSH    R0
				IN      R0, SREG
				PUSH    R0
				PUSH    R1
				CLR     R1
				PUSH    R2
				PUSH    R3
				PUSH    R4
				PUSH    R5
				PUSH    R6
				PUSH    R7
				PUSH    R8
				PUSH    R9
				PUSH    R10
				PUSH    R11
				PUSH    R12
				PUSH    R13
				PUSH    R14
				PUSH    R15
				PUSH    R16
				PUSH    R17
				PUSH    R18
				PUSH    R19
				PUSH    R20
				PUSH    R21
				PUSH    R22
				PUSH    R23
				PUSH    R24
				PUSH    R25
				PUSH    R26
				PUSH    R27
				PUSH    R28
				PUSH    R29
				PUSH    R30
				PUSH    R31
                IN      R16, EIND
                PUSH    R16
				IN      R16, RAMPD
                PUSH    R16
				IN      R16, RAMPX
                PUSH    R16
				IN      R16, RAMPY
                PUSH    R16
				IN      R16, RAMPZ
                PUSH    R16
                .endm
;*
;*********************************************************************************************************
;*                               POP ALL THE REGISTER OUT THE STACK
;*
;* Description : Restore all the register form the stack
;*                +-------+
;*                |  RO   | ---> [RO..RAMPZ]
;*                |  SREG |
;*                |  R1   |
;*                |  R2   |
;*                |   .   |
;*                |   .   |
;*                | RAMPZ |
;*                +-------+
;*********************************************************************************************************
;*
                .macro  POP_ALL                             ; Restore all registers
                POP     R16
                OUT     RAMPZ, R16
				POP     R16
                OUT     RAMPY, R16
                POP     R16
                OUT     RAMPX, R16
                POP     R16
                OUT     RAMPD, R16
                POP     R16
                OUT     EIND,  R16
				POP     R31
				POP     R30
				POP     R29
				POP     R28
				POP     R27
				POP     R26
				POP     R25
				POP     R24
				POP     R23
				POP     R22
				POP     R21
				POP     R20
				POP     R19
				POP     R18
				POP     R17
				POP     R16
				POP     R15
				POP     R14
				POP     R13
				POP     R12
				POP     R11
				POP     R10
				POP     R9
				POP     R8
				POP     R7
				POP     R6
				POP     R5
				POP     R4
				POP     R3
				POP     R2
				POP     R1
				POP     R0
				OUT     SREG, R0
				POP     R0
                .endm
;*
;*********************************************************************************************************
;*                               PUSH THE STACK POINTER INTO THE STACK
;*
;* Description : Save the stack pointer
;*
;* Note(s)     : R28 and R29 are equal to the current stak pointer [R28-R29] = SP
;*               after the macro exit.
;*
;*********************************************************************************************************
;*

                .macro  SAVE_SP                            ; Save stack pointer
                IN      R26,  SPL
				IN      R27,  SPH
			  .endm

;*
;*********************************************************************************************************
;*                               RESTORE THE STACK POINTER
;*
;* Description : Restore the Stack Pointer
;*
;* Note(s)     : X register points to the memory location where the SP is contained.
;*
;*********************************************************************************************************
;*
                .macro  RESTORE_SP                             ; Restore stack pointer
                 LD      R28,X+
                 OUT     SPL,R28
                 LD      R29,X+
                 OUT     SPH,R29
				.endm
