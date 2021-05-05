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
*                                          (TASKING Compiler)
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include <ucos_ii.h>

/*
*********************************************************************************************************
*                              SAVE 'DS:USP' IN TCB OF TASK BEING SWITCHED OUT
*********************************************************************************************************
*/

_inline void CPU_USPSave (void)
{
#pragma asm
                                            ;     OSTCBCur->OSTCBStkPtr = DS:USP
    SETB     0218H                          ; 4~, SSEL.0 = 1 (R0 uses ES register)
    MOV.B    ES, #SEG(_OSTCBCur)            ; 3~, ES:R0  = &OSTCBCur
    MOV.W    R0, #SOF(_OSTCBCur)            ; 3~
    MOV.W    R2, [R0+]                      ; 4~, R3:R2  = OSTCBCur
    MOV.W    R3, [R0]                       ; 5~
    MOV.W    R0, R2                         ; 3~, ES:R0  = R3:R2
    MOV.B    ES, R3L                        ; 3~
    MOV.B    R3H, #0                        ; 3~, R3:R2  = DS:USP
    MOV.B    R3L, DS                        ; 3~
    MOV      R2, USP                        ; 3~
    MOV.W    [R0+], R2                      ; 3~, OSTCBCur->OSTCBStkPtr = DS:USP
    MOV.W    [R0], R3                       ; 5~
#pragma endasm
}

/*
*********************************************************************************************************
*                             RESTORE 'DS:USP' FROM TCB OF TASK BEING SWITCHED IN
*********************************************************************************************************
*/

_inline void CPU_USPLoad (void)
{
#pragma asm
                                            ;     DS:USP = OSTCBHighRdy->OSTCBStkPtr
    SETB     0218H                          ; 4~, SSEL.0 = 1 (R0 uses ES register)
    MOV.B    ES, #SEG(_OSTCBHighRdy)        ; 3~, ES:R0  = &OSTCBHighRdy
    MOV.W    R0, #SOF(_OSTCBHighRdy)        ; 3~
    MOV.W    R2, [R0+]                      ; 3~, R3:R2  = OSTCBHighRdy
    MOV.W    R3, [R0]                       ; 5~
    MOV.B    ES, R3L                        ; 3~, ES:R0  = R3:R2
    MOV.W    R0, R2                         ; 3~
    MOV.W    R2, [R0+]                      ; 5~, R3:R2  = OSTCBHighRdy->OSTCBStkPtr
    MOV.W    R3, [R0]                       ; 4~
    MOV      USP, R2                        ; 3~, DS:USP = R3:R2
    MOV.B    DS, R3L                        ; 4~
#pragma endasm
}

/*
*********************************************************************************************************
*                                         RETURN FROM ISR
*********************************************************************************************************
*/

_inline void CPU_RetFromISR (void)
{
    EA = 1;                                 /* Re-enable interrupts because RETI does not              */
#pragma asm
    RETI
#pragma endasm
}


/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              pdata         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.  For the TASKING compiler, 'pdata' is passed
*                            in R1:R0.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when your task starts executing.
*
*              2) Two bytes are placed on the stack for the ES register because the 'POPU.B ES'
*                 instruction actually pops 2 bytes off the stack.  The upper byte of the popped value
*                 is discarded by the XA.
*
*              3) Two bytes are placed on the stack for the SSEL register because the 'POPU.B SSEL'
*                 instruction actually pops 2 bytes off the stack.  The upper byte of the popped value
*                 is discarded by the XA.
*
*              4) The last stacking operation (before the return) places the task address and the PSW
*                 in the reverse order of an interrupt.  This is done because the task address and PSW
*                 are actually placed on the USER stack and moved to the SYSTEM stack when a task is
*                 'switched-in'.
*
*              5) 'pdata' is assumed to be in R1:R0 for the TASKING compiler.
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{
    INT16U  *stk;
    INT32U   data;
    INT32U   temp;


    opt    = opt;                              /* 'opt' is not used, prevent warning                   */
    stk    = (INT16U *)ptos;                   /* Load stack pointer                                   */

                                               /* Simulate FCALL with argument ...                     */
    temp   = (INT32U)task;                     /*   ... Convert 'task' pointer to 32-bit value         */
    *--stk = (INT16U)( temp & 0xFFFFL);        /*   ... Place lower 16 bits of 'task' on stack         */
    *--stk = (INT16U)((temp >> 16) & 0x00FFL); /*   ... Place upper  8 bits of 'task' on stack         */

                                               /* Simulate INTERRUPT stacking ...                      */
    *--stk = (INT16U)0x6666;                   /*   R6   = 0x6666                                      */
    *--stk = (INT16U)0x5555;                   /*   R5   = 0x5555                                      */
    *--stk = (INT16U)0x4444;                   /*   R4   = 0x4444                                      */
    *--stk = (INT16U)0x3333;                   /*   R3   = 0x3333                                      */
    *--stk = (INT16U)0x2222;                   /*   R2   = 0x2222                                      */
    data   = (INT32U)pdata;                    /*   ... Convert 'pdata' pointer to 32-bit value        */
    *--stk = (INT16U)((data >> 16) & 0x00FFL); /*   R1:R0 = pdata (passed in registers R1:R0)          */
    *--stk = (INT16U)( data & 0xFFFFL);        /*                                                      */
    *--stk = (INT16U)0x0000;                   /*   ES   = 0x00     ES used by CEIBO emulator!         */
    *--stk = (INT16U)0xFFFF;                   /*   SSEL = 0xFF     Allow writes to ES register        */
                                               /*   NOTE: Push in reverse order because these will     */
                                               /*         be placed on the System Stack during a       */
                                               /*         context switch to this task.                 */
    *--stk = (INT16U)0x0000;                   /*   PSW  = User Mode, Register bank #0, Ints. En.      */
    *--stk = (INT16U)((temp >> 16) & 0x00FFL); /*   PC(H), i.e. in reverse order.                      */
    *--stk = (INT16U)( temp & 0xFFFFL);        /*   PC(L)                                              */
    return ((OS_STK *)stk);                    /* Return pointer to task's top-of-stack                */
}


#if OS_CPU_HOOKS_EN
/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookBegin (void)
{
}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookEnd (void)
{
}
#endif


/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskCreateHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
void OSTaskSwHook (void)
{
}

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/
void OSTaskStatHook (void)
{
}

/*
*********************************************************************************************************
*                                           OSTCBInit() HOOK
*
* Description: This function is called by OSTCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                                           /* Prevent Compiler warning                 */
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
void OSTimeTickHook (void)
{
}
#endif


/*
*********************************************************************************************************
*                                       START HIGHEST PRIORITY TASK
*
* Description : This function is called by OSStart() to start the highest priority task that was created
*               by your application before calling OSStart().
*
* Arguments   : none
*
* Note(s)     : 1) The stack frame (16-bit wide stack) is assumed to look as follows:
*
*                                                                                          LOW MEMORY
*                  OSTCBHighRdy->OSTCBStkPtr +  0  ---->  Pointer to task (Lower 16 bits)
*                                            +  2         Pointer to task (Upper  8 bits)
*                                            +  4         PSW  (initialized to 0x0000)
*                                            +  6         SSEL (initialized to 0x8080)
*                                            +  8         ES   (initialized to 0xEEEE)
*                                            + 10         R0
*                                            + 12         R1
*                                            + 14         R2
*                                            + 16         R3
*                                            + 18         R4
*                                            + 20         R5
*                                            + 22         R6
*                                            + 24         Pointer to task (Upper  8 bits)
*                                            + 26         Pointer to task (Lower 16 bits)
*                                            + 28         pdata (Lower 16 bits)
*                                            + 30         pdata (Upper  8 bits)
*                                                                                          HIGH MEMORY
*
*               2) OSStartHighRdy() MUST:
*                      a) Call OSTaskSwHook()
*                      b) Set OSRunning to TRUE
*                      c) Switch to the highest priority task.
*
*               3) It is assumed that this function is called with the XA in SYSTEM mode.
*********************************************************************************************************
*/

void OSStartHighRdy (void)
{
    OSTaskSwHook();

    OSRunning = TRUE;

    CPU_USPLoad();                          /* Get DS:USP from High Priority Task's OS_TCB             */

    CPU_PopAllFromUserStk();                /* Restore processor registers from USER stack             */

    CPU_RetFromISR();
}


/*
*********************************************************************************************************
*                                       TASK LEVEL CONTEXT SWITCH
*
* Description : This function is called when a task makes a higher priority task ready-to-run.
*
* Arguments   : none
*
* Note(s)     : 1) Upon entry,
*                  OSTCBCur     points to the OS_TCB of the task to suspend
*                  OSTCBHighRdy points to the OS_TCB of the task to resume
*
*               2) The stack frame of the task to suspend looks as follows.
*
*                                                                                          LOW MEMORY
*                                        SSP +  0  ---->  PSW
*                                            +  2         Return PC (Upper  8 bits)
*                                            +  4         Return PC (Lower 16 bits)
*                                                                                          HIGH MEMORY
*
*               3) The stack frame of the task to resume looks as follows:
*
*                                                                                          LOW MEMORY
*                  OSTCBHighRdy->OSTCBStkPtr +  0  ---->  Pointer to task (Lower 16 bits)
*                                            +  2         Pointer to task (Upper  8 bits)
*                                            +  4         PSW
*                                            +  6         SSEL
*                                            +  8         ES
*                                            + 10         R0
*                                            + 12         R1
*                                            + 14         R2
*                                            + 16         R3
*                                            + 18         R4
*                                            + 20         R5
*                                            + 22         R6
*                                                                                          HIGH MEMORY
*********************************************************************************************************
*/

_trap(15)
_using(0x8F00)
void  OSCtxSw (void)
{
    CPU_PushAllToUserStk();                 /* Save the processor context on the USER stack            */

    CPU_USPSave();                          /* Save DS:USP to Current Task's OS_TCB                    */

    OSTaskSwHook();                         /* 20~, Invoke user defined context switch hook            */

    OSTCBCur  = OSTCBHighRdy;
    OSPrioCur = OSPrioHighRdy;

    CPU_USPLoad();                          /* Get DS:USP from High Priority Task's OS_TCB             */

    CPU_PopAllFromUserStk();                /* Restore processor registers from USER stack             */

    CPU_RetFromISR();
}


/*
*********************************************************************************************************
*                               PERFORM A CONTEXT SWITCH (From an ISR)
*
* Description : This function is called when an ISR makes a higher priority task ready-to-run.
*
* Arguments   : none
*
* Note(s)     : 1) Upon entry,
*                  OSTCBCur     points to the OS_TCB of the task to suspend
*                  OSTCBHighRdy points to the OS_TCB of the task to resume
*
*               2) The stack frame of the task to suspend looks as follows.
*
*                                                                                          LOW MEMORY
*                                ------- SSP +  0         Return PC to OSIntExit(), lower 16 bits
*                                |           +  2         Return PC to OSIntExit(), upper  8 bits
*                         + 8    |           +  4         Return PC to ISR,         lower 16 bits
*                                |           +  6         Return PC to ISR,         upper  8 bits
*                                ------>     +  8         PSW
*                                            + 10         PC, upper  8 bits
*                                            + 12         PC, lower 16 bits
*                                                                                          HIGH MEMORY
*
*               3) The stack frame of the task to resume looks as follows:
*
*                                                                                          LOW MEMORY
*                  OSTCBHighRdy->OSTCBStkPtr +  0  ---->  Pointer to task (Lower 16 bits)
*                                            +  2         Pointer to task (Upper  8 bits)
*                                            +  4         PSW
*                                            +  6         SSEL
*                                            +  8         ES
*                                            + 10         R0
*                                            + 12         R1
*                                            + 14         R2
*                                            + 16         R3
*                                            + 18         R4
*                                            + 20         R5
*                                            + 22         R6
*                                                                                          HIGH MEMORY
*********************************************************************************************************
*/

void OSIntCtxSw (void)
{
    OSTaskSwHook();                         /* 20~, Invoke user defined context switch hook            */

    OSTCBCur  = OSTCBHighRdy;
    OSPrioCur = OSPrioHighRdy;

    CPU_USPLoad();                          /* Get DS:USP from High Priority Task's OS_TCB             */

    CPU_PopAllFromUserStk();                /* Restore processor registers from USER stack             */

    CPU_RetFromISR();
}


/*
*********************************************************************************************************
*                                             TICK ISR
*
* Notes: 1) Your ISR MUST be coded as shown below:
*           a) Declare your function with the '_interrupt()' attribute passing it the vector number.
*           b) Add the '_using()' attribute and pass it '0x8F00' to set the PSW
*           c) Add the _'frame()' attribute and specify NO argument.
*           d) declare the function as a function returning 'void' with 'void' argument
*           e) Save the processor context as shown in the code below.
*           f) Increment 'OSIntNesting'
*           g) Invoke YOUR C ISR handler
*           h) Call OSIntExit() at the end of your ISR
*           i) Restore the processor registers as shown in the code below.  DO NOT restore R0..R6.
*        2) You DON'T need to add an RETI instruction because it is also inserted by the Tasking
*           compiler.
*********************************************************************************************************
*/

_interrupt(33)  _using(0x8F00)  _frame()  void  OSTickISR (void)
{
    CPU_PushAllToUserStk();                 /* Save the processor context on the USER stack            */

    OSIntNesting++;                         /* Notify uC/OS-II of ISR entry                            */
    if (OSintNesting == 1) {
        CPU_USPSave();                      /* Save DS:USP to Current Task's OS_TCB                    */
    }

    OSTimeTick();                           /* Process tick                                            */
    OSIntExit();                            /* Notify uC/OS-II of ISR exit                             */

    CPU_PopAllFromUserStk();                /* Restore processor registers from USER stack             */
}


/*
*********************************************************************************************************
*                                            INTERRUPT STUBS
*********************************************************************************************************
*/

_interrupt(1) _using(0x8F00)  void  OS_XAResetISR (void)
{
}

_interrupt(2) _using(0x8F00)  void  OS_XABreakptISR (void)
{
}

_interrupt(3) _using(0x8F00)  void  OS_XATraceISR (void)
{
}

_interrupt(4) _using(0x8F00)  void  OS_XAStkOvfISR (void)
{
}

_interrupt(5) _using(0x8F00)  void  OS_XADivide0ISR (void)
{
}

_interrupt(6) _using(0x8F00)  void  OS_XAUserRETIISR (void)
{
}

_trap(0)      _using(0x8F00)  void  OS_XATrap00ISR (void)
{
}

_trap(1)      _using(0x8F00)  void  OS_XATrap01ISR (void)
{
}

_trap(2)      _using(0x8F00)  void  OS_XATrap02ISR (void)
{
}

_trap(3)      _using(0x8F00)  void  OS_XATrap03ISR (void)
{
}

_trap(4)      _using(0x8F00)  void  OS_XATrap04ISR (void)
{
}

_trap(5)      _using(0x8F00)  void  OS_XATrap05ISR (void)
{
}

_trap(6)      _using(0x8F00)  void  OS_XATrap06ISR (void)
{
}

_trap(7)      _using(0x8F00)  void  OS_XATrap07ISR (void)
{
}

_trap(8)      _using(0x8F00)  void  OS_XATrap08ISR (void)
{
}

_trap(9)      _using(0x8F00)  void  OS_XATrap09ISR (void)
{
}

_trap(10)     _using(0x8F00)  void  OS_XATrap10ISR (void)
{
}

_trap(11)     _using(0x8F00)  void  OS_XATrap11ISR (void)
{
}

_trap(12)     _using(0x8F00)  void  OS_XATrap12ISR (void)
{
}

_trap(13)     _using(0x8F00)  void  OS_XATrap13ISR (void)
{
}

_trap(14)     _using(0x8F00)  void  OS_XATrap14ISR (void)
{
}

#if 0
_trap(15)     _using(0x8F00)  void  OS_XATrap15ISR (void)
{
}
#endif


/*
*********************************************************************************************************
*                                             EVENT INTERRUPTS
*********************************************************************************************************
*/

_interrupt(32) _using(0x8F00)  _frame()  void  OS_XAExt0ISR (void)
{
}

#if 0
_interrupt(33) _using(0x8F00)  _frame()  void  OS_XATmr0ISR (void)
{
}
#endif

_interrupt(34) _using(0x8F00)  _frame()  void  OS_XAExt1ISR (void)
{
}

_interrupt(35) _using(0x8F00)  _frame()  void  OS_XATmr1ISR (void)
{
}

_interrupt(36) _using(0x8F00)  _frame()  void  OS_XATmr2ISR (void)
{
}

_interrupt(37) _using(0x8F00)  _frame()  void  OS_XAComm0RxISR (void)
{
}

_interrupt(38) _using(0x8F00)  _frame()  void  OS_XAComm0TxISR (void)
{
}

_interrupt(39) _using(0x8F00)  _frame()  void  OS_XAComm1RxISR (void)
{
}

_interrupt(40) _using(0x8F00)  _frame()  void  OS_XAComm1TxISR (void)
{
}


/*
*********************************************************************************************************
*                                             SOFTWARE INTERRUPTS
*********************************************************************************************************
*/

_interrupt(64) _using(0x8F00)  _frame()  void  OS_XASwi1ISR (void)
{
}

_interrupt(65) _using(0x8F00)  _frame()  void  OS_XASwi2ISR (void)
{
}

_interrupt(66) _using(0x8F00)  _frame()  void  OS_XASwi3ISR (void)
{
}

_interrupt(67) _using(0x8F00)  _frame()  void  OS_XASwi4ISR (void)
{
}

_interrupt(68) _using(0x8F00)  _frame()  void  OS_XASwi5ISR (void)
{
}

_interrupt(69) _using(0x8F00)  _frame()  void  OS_XASwi6ISR (void)
{
}

_interrupt(70) _using(0x8F00)  _frame()  void  OS_XASwi7ISR (void)
{
}
