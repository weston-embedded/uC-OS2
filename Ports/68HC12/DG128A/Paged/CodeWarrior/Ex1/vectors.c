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
*                                 PAGED 68HC812 INTERRUPT VECTOR TABLE
*                                                  IAR
*
* Filename : vectors.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#include <includes.h>

/*
*********************************************************************************************************
*                                              MC68HC812
*                                         INTERRUPT VECTOR TABLE
*
* Description: This is the interrupt vector table used by the MC68HC812 CPU.  Note that unused
*              interrupts are vectored to NULL.
* Notes      : 1) You MUST define the vector that defines the address of your startup code.
*********************************************************************************************************
*/

void (* const OSVectorTbl[])(void) = {      /* This table begins at 0xFF80                             */
    (void (*)())NULL,                       /* 0xFF80 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF82 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF84 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF86 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF88 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF8A Reserved                                         */
    (void (*)())NULL,                       /* 0xFF8C Reserved                                         */
    (void (*)())NULL,                       /* 0xFF8E Reserved                                         */

    (void (*)())NULL,                       /* 0xFF90 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF92 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF94 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF96 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF98 Reserved                                         */
    (void (*)())NULL,                       /* 0xFF9A Reserved                                         */
    (void (*)())NULL,                       /* 0xFF9C Reserved                                         */
    (void (*)())NULL,                       /* 0xFF9E Reserved                                         */

    (void (*)())NULL,                       /* 0xFFA0 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFA2 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFA4 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFA6 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFA8 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFAA Reserved                                         */
    (void (*)())NULL,                       /* 0xFFAC Reserved                                         */
    (void (*)())NULL,                       /* 0xFFAE Reserved                                         */

    (void (*)())NULL,                       /* 0xFFB0 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFB2 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFB4 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFB6 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFB8 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFBA Reserved                                         */
    (void (*)())NULL,                       /* 0xFFBC Reserved                                         */
    (void (*)())NULL,                       /* 0xFFBE Reserved                                         */

    (void (*)())NULL,                       /* 0xFFC0 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFC2 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFC4 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFC6 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFC8 Reserved                                         */
    (void (*)())NULL,                       /* 0xFFCA Reserved                                         */
    (void (*)())NULL,                       /* 0xFFCC Reserved                                         */
    (void (*)())NULL,                       /* 0xFFCE Key Wakeup H                                     */

    (void (*)())NULL,                       /* 0xFFD0 Key Wakeup J                                     */
    (void (*)())NULL,                       /* 0xFFD2 ATD                                              */
    (void (*)())NULL,                       /* 0xFFD4 SCI 1                                            */
    (void (*)())NULL,                       /* 0xFFD6 SCI 0                                            */
    (void (*)())NULL,                       /* 0xFFD8 SPI Serial Transfer Complete                     */
    (void (*)())NULL,                       /* 0xFFDA Pulse Accumulator Input Edge                     */
    (void (*)())NULL,                       /* 0xFFDC Pulse Accumulator Overflow                       */
    (void (*)())NULL,                       /* 0xFFDE Timer Overflow                                   */

#if OS_TICK_OC == 7
    (void (*)())OSTickISR,                  /* 0xFFE0 Timer Channel 7                                  */
#else
    (void (*)())NULL,                       /* 0xFFE0 Timer Channel 7                                  */
#endif

#if OS_TICK_OC == 6
    (void (*)())OSTickISR,                  /* 0xFFE2 Timer Channel 6                                  */
#else
    (void (*)())NULL,                       /* 0xFFE2 Timer Channel 6                                  */
#endif

#if OS_TICK_OC == 5
    (void (*)())OSTickISR,                  /* 0xFFE4 Timer Channel 5                                  */
#else
    (void (*)())NULL,                       /* 0xFFE4 Timer Channel 5                                  */
#endif

#if OS_TICK_OC == 4
    (void (*)())OSTickISR,                  /* 0xFFE6 Timer Channel 4                                  */
#else
    (void (*)())NULL,                       /* 0xFFE6 Timer Channel 4                                  */
#endif

#if OS_TICK_OC == 3
    (void (*)())OSTickISR,                  /* 0xFFE8 Timer Channel 3                                  */
#else
    (void (*)())NULL,                       /* 0xFFE8 Timer Channel 3                                  */
#endif

#if OS_TICK_OC == 2
    (void (*)())OSTickISR,                  /* 0xFFEA Timer Channel 2                                  */
#else
    (void (*)())NULL,                       /* 0xFFEA Timer Channel 2                                  */
#endif

#if OS_TICK_OC == 1
    (void (*)())OSTickISR,                  /* 0xFFEA Timer Channel 1                                  */
#else
    (void (*)())NULL,                       /* 0xFFEA Timer Channel 1                                  */
#endif

#if OS_TICK_OC == 0
    (void (*)())OSTickISR,                  /* 0xFFEA Timer Channel 0                                  */
#else
    (void (*)())NULL,                       /* 0xFFEA Timer Channel 0                                  */
#endif

    (void (*)())NULL,                       /* 0xFFF0 Real Time Interrupt (RTI)                        */

    (void (*)())NULL,                       /* 0xFFF2 IRQ (External Pin) or Key Wakeup D               */

    (void (*)())NULL,                       /* 0xFFF4 XIRQ Pin (Pseudo Nonmaskable Interrupt)          */

    (void (*)())NULL,                       /* 0xFFF6 SWI                                              */

    (void (*)())NULL,                       /* 0xFFF8 Illegal Opcode Trap                              */

    (void (*)())NULL,                       /* 0xFFFA COP Failure (Reset)                              */

    (void (*)())NULL,                       /* 0xFFFC COP Clock Monitor Fail (Reset)                   */

    (void (*)())NULL                        /* 0xFFFE RESET                                            */
};
