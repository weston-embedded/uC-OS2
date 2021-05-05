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
*                                       Philips XA Specific code
*                                          LARGE MEMORY MODEL
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned int   INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   int   INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 16-bit wide                    */

#define BYTE           INT8S                     /* Define data types for backward compatibility ...   */
#define UBYTE          INT8U                     /* ... to uC/OS V1.xx.  Not actually needed for ...   */
#define WORD           INT16S                    /* ... uC/OS-II.                                      */
#define UWORD          INT16U
#define LONG           INT32S
#define ULONG          INT32U

/*
*********************************************************************************************************
*                                        CRITICAL SECTION CONTROL
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.
*             The XA port doesn't currently support this method.
*********************************************************************************************************
*/
#define  OS_CRITICAL_METHOD    1


#if      OS_CRITICAL_METHOD == 1
#define  OS_ENTER_CRITICAL()  EA = 0             /* Disable interrupts                                 */
#define  OS_EXIT_CRITICAL()   EA = 1             /* Enable  interrupts                                 */
#endif

/*
*********************************************************************************************************
*                                           MISCELLANEOUS
*********************************************************************************************************
*/

#define  OS_STK_GROWTH         1               /* Stack grows from HIGH to LOW memory on XA            */

#define  OS_TASK_SW()        _intxa(31)        /* Trap #15, (Interrupt 31) invokes OSCtxSw()           */

/*
*********************************************************************************************************
*                                  SAVE PROCESSOR CONTEXT ONTO USER STACK
*
* Note(s) : 1) The final OR.B instruction, forces all register indirect addressing to be via the ES
*              segment register. In the large memory model this is assumed by the compiler to be the
*              default value and is required for correct operation of code generated by the C & C++
*              compilers. If an assembly routine (e.g. a Tasking library function) fiddles with the
*              SSEL value and we just interrupted one of those, then we may have "inherited" a non-0xFFH
*              value. If we had specified the SSEL register in the _frame() directive for this ISR, the
*              compiler would have generated the instruction automatically. Since an empty _frame()
*              is used we will have to set SSEL ourselve. The value will be restored on RETI.
*              Thanks to Scott and Peter for this finding:
*                  Scott Finneran  sfinneran@lucent.com
*                  Peter Miller    millerp@canb.auug.org.au
*********************************************************************************************************
*/

_inline void CPU_PushAllToUserStk (void)
{
#pragma asm
                                            ;     SAVE PROCESSOR CONTEXT ONTO USER STACK
    PUSHU    R0,R1,R2,R3,R4,R5,R6           ;24~, Save interrupted task's context
    PUSHU.B  ES                             ; 5~, Save ES
    PUSHU.B  SSEL                           ; 5~, Save SSEL
    POP      R2                             ; 5~, Move PSW from SS to US
    PUSHU    R2                             ; 5~
    POP      R2                             ; 5~, Move PCH from SS to US
    PUSHU    R2                             ; 5~
    POP      R2                             ; 5~, Move PCL from SS to US
    PUSHU    R2                             ; 5~

    OR.B     SSEL,#0FFH                     ;     All addressing via ES
#pragma endasm
}

/*
*********************************************************************************************************
*                                RESTORE PROCESSOR CONTEXT FROM USER STACK
*********************************************************************************************************
*/

_inline void CPU_PopAllFromUserStk (void)
{
#pragma asm
                                            ;     RESTORE PROCESSOR CONTEXT FROM USER STACK
    POPU     R2                             ; 5~, Move PCL from US to SS
    PUSH     R2                             ; 5~
    POPU     R2                             ; 5~, Move PCH from US to SS
    PUSH     R2                             ; 5~
    POPU     R2                             ; 5~, Move PSW from US to SS
    PUSH     R2                             ; 5~
    POPU.B   SSEL                           ; 5~, Get SSEL
    POPU.B   ES                             ; 5~, Get ES
    POPU     R0,R1,R2,R3,R4,R5,R6           ;18~, Load interrupted task's context
#pragma endasm
}

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                          TASKING  ATTRIBUTES
*                                      (Target Specific Functions)
*********************************************************************************************************
*/

_trap(15)      _using(0x8F00)            void  OSCtxSw(void);
_interrupt(33) _using(0x8F00)  _frame()  void  OSTickISR(void);


/*
*********************************************************************************************************
*                                           EXCEPTIONS
*********************************************************************************************************
*/

_interrupt(1) _using(0x8F00)  void  OS_XAResetISR(void);
_interrupt(2) _using(0x8F00)  void  OS_XABreakptISR(void);
_interrupt(3) _using(0x8F00)  void  OS_XATraceISR(void);
_interrupt(4) _using(0x8F00)  void  OS_XAStkOvfISR(void);
_interrupt(5) _using(0x8F00)  void  OS_XADivide0ISR(void);
_interrupt(6) _using(0x8F00)  void  OS_XAUserRETIISR(void);

/*
*********************************************************************************************************
*                                             TRAPS
*********************************************************************************************************
*/

_trap(0)      _using(0x8F00)  void  OS_XATrap00ISR(void);
_trap(1)      _using(0x8F00)  void  OS_XATrap01ISR(void);
_trap(2)      _using(0x8F00)  void  OS_XATrap02ISR(void);
_trap(3)      _using(0x8F00)  void  OS_XATrap03ISR(void);
_trap(4)      _using(0x8F00)  void  OS_XATrap04ISR(void);
_trap(5)      _using(0x8F00)  void  OS_XATrap05ISR(void);
_trap(6)      _using(0x8F00)  void  OS_XATrap06ISR(void);
_trap(7)      _using(0x8F00)  void  OS_XATrap07ISR(void);
_trap(8)      _using(0x8F00)  void  OS_XATrap08ISR(void);
_trap(9)      _using(0x8F00)  void  OS_XATrap09ISR(void);
_trap(10)     _using(0x8F00)  void  OS_XATrap10ISR(void);
_trap(11)     _using(0x8F00)  void  OS_XATrap11ISR(void);
_trap(12)     _using(0x8F00)  void  OS_XATrap12ISR(void);
_trap(13)     _using(0x8F00)  void  OS_XATrap13ISR(void);
_trap(14)     _using(0x8F00)  void  OS_XATrap14ISR(void);

#if 0
_trap(15)     _using(0x8F00)  void  OS_XATrap15ISR(void);            /* Used by OSCtxSw()              */
#endif

/*
*********************************************************************************************************
*                                             EVENT INTERRUPTS
*********************************************************************************************************
*/

_interrupt(32) _using(0x8F00)  _frame()  void  OS_XAExt0ISR(void);

#if 0
_interrupt(33) _using(0x8F00)  _frame()  void  OS_XATmr0ISR(void);   /* Used by OSTickISR()            */
#endif

_interrupt(34) _using(0x8F00)  _frame()  void  OS_XAExt1ISR(void);
_interrupt(35) _using(0x8F00)  _frame()  void  OS_XATmr1ISR(void);
_interrupt(36) _using(0x8F00)  _frame()  void  OS_XATmr2ISR(void);
_interrupt(37) _using(0x8F00)  _frame()  void  OS_XAComm0RxISR(void);
_interrupt(38) _using(0x8F00)  _frame()  void  OS_XAComm0TxISR(void);
_interrupt(39) _using(0x8F00)  _frame()  void  OS_XAComm1RxISR(void);
_interrupt(40) _using(0x8F00)  _frame()  void  OS_XAComm1TxISR(void);

/*
*********************************************************************************************************
*                                             SOFTWARE INTERRUPTS
*********************************************************************************************************
*/

_interrupt(64) _using(0x8F00)  _frame()  void  OS_XASwi1ISR(void);
_interrupt(65) _using(0x8F00)  _frame()  void  OS_XASwi2ISR(void);
_interrupt(66) _using(0x8F00)  _frame()  void  OS_XASwi3ISR(void);
_interrupt(67) _using(0x8F00)  _frame()  void  OS_XASwi4ISR(void);
_interrupt(68) _using(0x8F00)  _frame()  void  OS_XASwi5ISR(void);
_interrupt(69) _using(0x8F00)  _frame()  void  OS_XASwi6ISR(void);
_interrupt(70) _using(0x8F00)  _frame()  void  OS_XASwi7ISR(void);
