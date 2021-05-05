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
;                                           Generic ARM Port
;                                              VFP SUPPORT
;
; Filename  : os_cpu_fpu_a.asm
; Version   : V2.93.01
;********************************************************************************************************
; For       : ARM7 or ARM9
; Mode      : ARM  or Thumb
; Toolchain : RealView Development Suite
;             RealView Microcontroller Development Kit (MDK)
;             ARM Developer Suite (ADS)
;             Keil uVision
;********************************************************************************************************


;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

    EXPORT  OS_CPU_FP_Restore
    EXPORT  OS_CPU_FP_Save

;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

    AREA CODE, CODE, READONLY
    CODE32

;*********************************************************************************************************
;                                           RESTORE VFP REGISTERS
;                                       void OS_CPU_FP_Restore(void *pblk)
;
; Description : This function is called to restore the contents of the VFP registers during a context
;               switch.  It is assumed that a pointer to a storage area for the VFP registers is placed
;               in the task's TCB (i.e. .OSTCBExtPtr).

; Arguments   : pblk    is passed to this function in R0 when called.
;
; Notes       : Floating point math should NEVER be performed within an ISR as this will corrupt
;               the state of the VFP registers for the last VFP task that ran. Instead, all floating
;               point math should be performed within floating point enabled tasks ONLY.
;*********************************************************************************************************

OS_CPU_FP_Restore
        FLDMIAS R0!, {S0-S31}           ; Restore the VFP registers from pblk
        BX      LR                      ; Return to calling function

;*********************************************************************************************************
;                                           SAVE VFP REGISTERS
;                                        void OS_CPU_FP_Save(void *pblk)
;
; Description : This function is called to save the contents of the VFP registers during a context
;               switch.  It is assumed that a pointer to a storage area for the VFP registers is placed
;               in the task's TCB (i.e. .OSTCBExtPtr).
;
; Arguments   : pblk    is passed to this function in R0 when called.
;
; Notes       : Floating point math should NEVER be performed within an ISR as this will corrupt
;               the state of the VFP registers for the last VFP task that ran. Instead, all floating
;               point math should be performed within floating point enabled tasks ONLY.
;*********************************************************************************************************

OS_CPU_FP_Save
        FSTMIAS R0!, {S0-S31}           ; Save the VFP registers in pblk
        BX      LR                      ; Return to calling function


        END
