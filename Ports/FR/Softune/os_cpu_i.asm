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
; Filename : os_cpu_i.asm
; Version  : V2.93.01
;********************************************************************************************************

;*********************************************************************************************************
;                                                MACROS
;*********************************************************************************************************

#macro  PUSH_ALL
        ST          RP, @-R15
        STM1        (R8,R9,R10,R11,R12,R13,R14)
        STM0        (R0,R1,R2,R3,R4,R5,R6,R7)
        ST          MDH, @-R15
        ST          MDL, @-R15
#endm


#macro  POP_ALL
        LD          @R15+, MDL
        LD          @R15+, MDH
        LDM0        (R7,R6,R5,R4,R3,R2,R1,R0)
        LDM1        (R14,R13,R12,R11,R10,R9,R8)
        LD          @R15+, RP
#endm
