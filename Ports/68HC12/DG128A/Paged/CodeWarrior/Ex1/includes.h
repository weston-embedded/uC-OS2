/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          Master Include File
*
* Filename : includes.h
* Version  : V2.93.01
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         PROCESSOR SPECIFICS
*********************************************************************************************************
*/

#define  CPU_FRT_FREQ         500000L       /* Free Running Timer rate (Hz)                            */

#define  OS_TICK_OC                 7       /* Output compare # used to generate a tick int.           */

                                            /* Number of FRT counts to produce an interrupt @tick rate */
#define  OS_TICK_OC_CNTS  (CPU_FRT_FREQ / OS_TICKS_PER_SEC)

/*
*********************************************************************************************************
*                                           FILES TO INCLUDE
*********************************************************************************************************
*/

#include  <iob32.h>
#include  <stddef.h>

#include  <ucos_ii.h>
