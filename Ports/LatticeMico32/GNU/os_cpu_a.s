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
*                                          LATTICEMICO32 Port
*
* Filename  : os_cpu_a.s
* Version   : V2.93.01
*********************************************************************************************************
* For       : LatticeMico32
* Toolchain : GNU C/C++ Compiler
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           PUBLIC FUNCTIONS
*********************************************************************************************************
*/

    .global OS_CPU_SR_Save
    .global OS_CPU_SR_Restore
    .global OSIntCtxSw
    .global OSCtxSw
    .global OSStartHighRdy

/*
*********************************************************************************************************
*                                      CODE GENERATION DIRECTIVES
*********************************************************************************************************
*/

    .text

/*
*********************************************************************************************************
*                                  CRITICAL SECTION METHOD 3 FUNCTIONS
*
* Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*              into the CPU's status register.
*
* Prototypes : OS_CPU_SR  OS_CPU_SR_Save    (void);
*              void       OS_CPU_SR_Restore (OS_CPU_SR  os_cpu_sr);
*
*
* Note(s)    : 1) These functions are used in general like this:
*
*                 void Task (void  *p_arg)
*                 {
*
*                 #if (OS_CRITICAL_METHOD == 3)
*                      OS_CPU_SR  os_cpu_sr;
*                 #endif
*
*                          :
*                          :
*                      OS_ENTER_CRITICAL();
*                          :
*                          :
*                      OS_EXIT_CRITICAL();
*                          :
*                          :
*                 }
*********************************************************************************************************
*/

OS_CPU_SR_Save:
    rcsr   r1, ie
    wcsr   ie, r0
    ret

OS_CPU_SR_Restore:
    wcsr   ie, r1
    ret

/*
*********************************************************************************************************
*                                PERFORM A CONTEXT SWITCH
*                                           void OSCtxSw(void)    - from task level
*                                           void OSIntCtxSw(void) - from interrupt level
*
* Note(s): 1) Upon entry:
*             OSTCBCur     points to the OS_TCB of the task to suspend,
*             OSTCBHighRdy points to the OS_TCB of the task to resume
*          2) If the task being suspended is being suspended from a task-level, then the caller-
*             saved registers are already saved and in this case we need to save callee-saved registers
*             only.  If the task being suspended is via a interrupt then the caller-saved registers are
*             already saved by the interrupt service routine. Hence, either case, the routines remain
*             the same.
*          3) We must save the state of ie and must not enable it explicitly.  From an interrupt-level
*             invocation, the main ISR is assumed to have set the IE-bit AND EIE-bit to zero.
*          4) Regarding EA/RA:
*             a) For the task to suspend, EA does NOT need to be saved.  The context switch is invoked
*                by a function call, and the saved RA will be loaded into EA next time the task is
*                activated.
*             b) For the task to resume, EA is made the same as RA, because this function returns with
*                ERET instead of RET.
*********************************************************************************************************
*/

OSIntCtxSw:
OSCtxSw:
    xor     r0,  r0, r0
    addi    sp, sp, -92                     	/* Create space on the stack                           */

                                           		/* SAVE CURRENT TASK'S CONTEXT:                        */
    sw      (sp+ 4), r11                        /*     Save R11-R27                                    */
    sw      (sp+ 8), r12
    sw      (sp+12), r13
    sw      (sp+16), r14
    sw      (sp+20), r15
    sw      (sp+24), r16
    sw      (sp+28), r17
    sw      (sp+32), r18
    sw      (sp+36), r19
    sw      (sp+40), r20
    sw      (sp+44), r21
    sw      (sp+48), r22
    sw      (sp+52), r23
    sw      (sp+56), r24
    sw      (sp+60), r25
    sw      (sp+64), r26
    sw      (sp+68), r27               		    /*    Save frame pointer                               */


    sw      (sp+84), r1                         /*    Save r1, which may contain the task argument     */
    rcsr    r1,      ie                         /*    Save IE                                          */
    sw      (sp+72), r1
    sw      (sp+80), ra                         /*    Save RA value (see also Note #4)                 */

    orhi    r1, r0, hi(_impure_ptr)             /*    Save impure_ptr                                  */
    ori     r1, r1, lo(_impure_ptr)
    lw      r2, (r1+0)
    sw      (sp+88), r2

    orhi    r1, r0, hi(OSTCBCur)                /* OSTCBCur->OSTCBStkPtr = SP;                         */
    ori     r1, r1, lo(OSTCBCur)
    lw      r2, (r1+0)
    sw      (r2+0), sp

    calli   OSTaskSwHook                        /* OSTaskSwHook();                                     */

    orhi    r1, r0, hi(OSPrioCur)               /* OSPrioCur = OSPrioHighRdy;                          */
    ori     r1, r1, lo(OSPrioCur)
    orhi    r2, r0, hi(OSPrioHighRdy)
    ori     r2, r2, lo(OSPrioHighRdy)
    lb      r3, (r2+0)
    sb      (r1+0), r3

    orhi    r1, r0, hi(OSTCBCur)                /* OSTCBCur = OSTCBHighRdy;                            */
    ori     r1, r1, lo(OSTCBCur)
    orhi    r2, r0, hi(OSTCBHighRdy)
    ori     r2, r2, lo(OSTCBHighRdy)
    lw      r3, (r2+0)
    sw      (r1+0), r3

    lw      sp, (r3+0)                          /* SP = OSTCBHighRdy->OSTCBStkPtr;                     */

                                                /* RESTORE NEW TASK'S CONTEXT:                         */
    lw      r1,  (sp+72)                        /* Restore state of IE                                 */
    wcsr    ie,   r1
    lw      ea,  (sp+80)                        /*    Restore EA                                       */
    lw      ra,  (sp+80)                        /*    Restore RA                                       */


    lw      r11, (sp+ 4)                        /*    Restore R11-R27                                  */
    lw      r12, (sp+ 8)
    lw      r13, (sp+12)
    lw      r14, (sp+16)
    lw      r15, (sp+20)
    lw      r16, (sp+24)
    lw      r17, (sp+28)
    lw      r18, (sp+32)
    lw      r19, (sp+36)
    lw      r20, (sp+40)
    lw      r21, (sp+44)
    lw      r22, (sp+48)
    lw      r23, (sp+52)
    lw      r24, (sp+56)
    lw      r25, (sp+60)
    lw      r26, (sp+64)
    lw      r27, (sp+68)                        /*    Restore frame pointer                            */
    orhi    r1,  r0, hi(_impure_ptr)            /*    Restore impure_ptr                               */
    ori     r1,  r1, lo(_impure_ptr)
    lw      r2,  (sp+88)
    sw      (r1+0), r2
    lw      r1,  (sp+84)                        /*    Restore r1, which may contain the task argument  */

    addi    sp,  sp, 92                         /*    Restore stack                                    */
    eret                                        /* Return (from exception) and copy EIE to IE          */
    nop
    nop
    nop
    nop

/*
*********************************************************************************************************
*                                           START MULTITASKING
*                                       void OSStartHighRdy(void)
*
* Note(s) : 1) OSStartHighRdy() MUST:
*              a) Call OSTaskSwHook() then,
*              b) Set OSRunning to TRUE,
*              c) Switch to the highest priority task.
*********************************************************************************************************
*/

OSStartHighRdy:

    xor     r0, r0, r0                          /* Disable interrupts                                  */
    wcsr    ie, r0

    calli   OSTaskSwHook                        /* OSTaskSwHook();                                     */

    orhi    r2, r0, hi(OSRunning)               /* OSRunning = TRUE;                                   */
    ori     r2, r2, lo(OSRunning)
    ori     r3, r0, 0x1
    sw      (r2+0), r3

                                                /* SWITCH TO HIGHEST PRIORITY TASK:                    */
    orhi    r1, r0, hi(OSTCBHighRdy)            /*    Get highest priority task TCB address            */
    ori     r1, r1, lo(OSTCBHighRdy)
    lw      r4, (r1+0)
    lw      sp, (r4+0)

    lw      fp, (sp+68)                         /*    Restore frame pointer                            */
    lw      ea, (sp+80)                         /*    Restore exception address                        */

    lw      r1, (sp+72)                         /*    Restore state of IE                              */
    wcsr    ie, r1

    orhi    r1, r0, hi(_impure_ptr)             /*    Restore value of impute_ptr                      */
    ori     r1, r1, lo(_impure_ptr)
    lw      r2, (sp+88)
    sw      (r1+0), r2

    lw      r1, (sp+84)                         /*    Restore R1; this contains the task argument      */

    addi    sp, sp, 92                          /* Free stack space used by register context           */

    eret                                        /* Perform ERET to set IE bit from saved value         */
    nop
    nop
    nop
    nop
