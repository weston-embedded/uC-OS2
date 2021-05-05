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
; Filename : os_cpu_i.asm
; Version  : V2.93.01
;********************************************************************************************************
*/

/*
;**************************************************************************************************
;                                             MACROS
;**************************************************************************************************
*/

       .macro    OS_EMAC_SAVE
                                      /* non-EMAC port does not save any EMAC registers     */

      .endm



       .macro    OS_EMAC_RESTORE
                                      /* non-EMAC port does not save any EMAC registers     */

      .endm
