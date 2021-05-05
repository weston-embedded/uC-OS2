#********************************************************************************************************
#                                              uC/OS-II
#                                        The Real-Time Kernel
#
#                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
#
#                                 SPDX-License-Identifier: APACHE-2.0
#
#               This software is subject to an open source license and is distributed by
#                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
#                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
#
#********************************************************************************************************

#********************************************************************************************************
#
#                                          Freescale  MPC56xx
#                                         CodeWarrior Compiler
#
# Filename : os_cpu_a.s
# Version  : V2.93.01
#********************************************************************************************************

#*********************************************************************************************************
#*                                             ASM HEADER
#*********************************************************************************************************

    .include  "os_cpu_a.inc"

    .section  .text, text

    .extern  BSP_TmrTickISR

    .extern  OSTaskSwHook
    .extern  OSIntEnter
    .extern  OSIntExit

    .extern  OSTCBCur                                           # Pointer to Current Tasks TCB
    .extern  OSTCBHighRdy                                       # Pointer to Highest Priority Ready Task
    .extern  OSPrioHighRdy
    .extern  OSPrioCur
    .extern  OSRunning
    .extern  OSIntNesting


#*********************************************************************************************************
#*                                              DEFINES
#*********************************************************************************************************

    .equ  INTC_IACKR, 0xfff48010                                # Interrupt Acknowledge Register Address
    .equ  INTC_EOIR,  0xfff48018                                # End of Interrupt Register Address


#*********************************************************************************************************
#*                                          PUBLIC DECLARATIONS
#*********************************************************************************************************

    .global  OSCtxSw
    .global  OSStartHighRdy
    .global  OSIntCtxSw

    .global  OSTickISR
    .global  OSExtIntISR

    .global  OS_CPU_SR_Save
    .global  OS_CPU_SR_Restore
    .global  OS_CPU_SR_Rd


#*********************************************************************************************************
#*                                      ISR HANDLER FOR SW VECTOR MODE
#*
#* Description : This function is executed when IVOR4 External input interrupt (SPR 404) exception occurs.
#*********************************************************************************************************
    .align  16
OSExtIntISR:
    prologue                                                    # Save context

    lis     r11,OSIntNesting@ha                                 # Load OSIntNesting
    lbz     r3, OSIntNesting@l(r11)
    addi    r0, r3, 1
    stb     r0, OSIntNesting@l(r11)                             # OSIntNesting++

    cmpwi   r0, 1                                               # if (OSIntNesting != 1) ...
    bne     OSExtIntISR_NotSaveSP                               # Do not save the stackpointer

    lis     r11,OSTCBCur@ha                                     # Get pointer to current TCB
    lwz     r11,OSTCBCur@l(r11)
    stw     r1, 0(r11)                                          # Save stack pointer in current TCB

OSExtIntISR_NotSaveSP:
    lis     r3, INTC_IACKR@ha                                   # Store address of INTC.IACKR in r3
    lwz     r3, INTC_IACKR@l(r3)                                # Load contents of INTC.IACKR in r3
    lwz     r12,0(r3)                                           # Load the base adress of Vector Table

    mtctr   r12                                                 # Load Count register
    bctrl                                                       # Branch to ISR_Handler

    li      r0, 0                                               # Load r0 with 0x0000
    lis     r3, INTC_EOIR@ha                                    # Get the address of the INTC.EOIR Register ...
    stw     r0, INTC_EOIR@l(r3)                                 # and clear the INTC.EOIR register

    bl      OSIntExit                                           # Call to decrement OSIntNesting

    epilogue                                                    # Restore the context

    rfi                                                         # Return from interrupt


#*********************************************************************************************************
#*                                            SYSTEM TICK ISR
#*
#* Description : This function is the ISR to notify uC/OS-II that a system tick has occurred.
#*********************************************************************************************************
    .align  16
OSTickISR:
    prologue

    lis     r11,OSIntNesting@ha                                 # Load OSIntNesting
    lbz     r3, OSIntNesting@l(r11)
    addi    r0, r3, 1
    stb     r0, OSIntNesting@l(r11)                             # OSIntNesting++

    cmpwi   r0, 1                                               # if (OSIntNesting != 1) ...
    bne     OSTickISR_NotSaveSP                                 # do not save the stackpointer

    lis     r11,OSTCBCur@ha                                     # Get pointer to current TCB
    lwz     r11,OSTCBCur@l(r11)
    stw     r1, 0(r11)                                          # Save stack pointer in current TCB

OSTickISR_NotSaveSP:
    lis     r4, 0x0800                                          # Load r4 with TSR[DIS] bit (0x08000000)
    mtspr   TSR,r4                                              # Clear TSR[DIS] bit

    bl      BSP_TmrTickISR                                      # Call TmrTick handler
    bl      OSIntExit                                           # Call to decrement OSIntNesting

    epilogue                                                    # Restore context

    rfi                                                         # Run task


#*********************************************************************************************************
#*                                      CRITICAL SECTION FUNCTIONS
#*
#* Description : These functions are used to enter and exit critical sections using Critical Method #3.
#*
#*                OS_CPU_SR  OS_CPU_SR_Save (void)
#*                          Get current global interrupt mask bit value from MSR
#*                          Disable interrupts
#*                          Return global interrupt mask bit
#*
#*                   void    OS_CPU_SR_Restore (OS_CPU_SR  cpu_sr)
#*                          Set global interrupt mask bit on MSR according to parameter cpu_sr
#*                          Return
#*
#* Argument(s) : cpu_sr      global interrupt mask status.
#*********************************************************************************************************

OS_CPU_SR_Save:
    mfmsr   r3
    wrteei  0
    blr

OS_CPU_SR_Restore:
    mtmsr   r3
    blr


#*********************************************************************************************************
#*                                      READ STATUS REGISTER FUNCTION
#*
#* Description : This function is used to retrieve the status register value.
#*
#*                OS_CPU_SR  OS_CPU_SR_Rd (void)
#*                          Get current MSR value
#*                          Return
#*********************************************************************************************************

OS_CPU_SR_Rd:
    mfmsr   r3
    blr


#*********************************************************************************************************
#*                                 START HIGHEST PRIORITY TASK READY-TO-RUN
#*
#* Description : This function is called by OSStart() to start the highest priority task that was created
#*               by the application before calling OSStart().
#*
#* Arguments   : none
#*
#* Note(s)     : 1) OSStartHighRdy() MUST:
#*                   a) Call OSTaskSwHook() then,
#*                   b) Set OSRunning to TRUE,
#*                   c) Switch to the highest priority task by loading the stack pointer of the highest
#*                      priority task into the SP register and execute an rfi instruction.
#*********************************************************************************************************
    .align  16
OSStartHighRdy:
    bl      OSTaskSwHook                                        # Call OSTaskSwHook

    li      r0, 1                                               # Set OSRunning to 1
    lis     r11,OSRunning@ha
    stb     r0, OSRunning@l(r11)

    lis     r11,OSTCBHighRdy@ha                                 # Get pointer to ready task TCB
    lwz     r11,OSTCBHighRdy@l(r11)

    lis     r12,OSTCBCur@ha                                     # Save as current task TCB ptr.
    stw     r11,OSTCBCur@l(r12)

    lwz     r1, 0(r11)                                          # Get new stack pointer

    epilogue                                                    # Restore context

    rfi                                                         # Run task


#*********************************************************************************************************
#*                                         TASK LEVEL CONTEXT SWITCH
#*
#* Description : This function is called when a task makes a higher priority task ready-to-run.
#*
#* Arguments   : none
#*
#* Note(s)     : 1) Upon entry,
#*                  OSTCBCur     points to the OS_TCB of the task to suspend
#*                  OSTCBHighRdy points to the OS_TCB of the task to resume
#*********************************************************************************************************
    .align  16
OSCtxSw:
    prologue                                                    # Save context

    lis     r11,OSTCBCur@ha                                     # Get pointer to current TCB
    lwz     r11,OSTCBCur@l(r11)
    stw     r1, 0(r11)                                          # Save stack pointer in current TCB

    bl      OSTaskSwHook                                        # Call OSTaskSwHook

    lis     r11,OSTCBHighRdy@ha                                 # Get pointer to ready task TCB
    lwz     r11,OSTCBHighRdy@l(r11)

    lis     r12,OSTCBCur@ha                                     # Save as current task TCB ptr.
    stw     r11,OSTCBCur@l(r12)

    lis     r12,OSPrioHighRdy@ha                                # Get High Ready Priority
    lbz     r10,OSPrioHighRdy@l(r12)

    lis     r12,OSPrioCur@ha                                    # Save as Current Priority
    stb     r10,OSPrioCur@l(r12)

    lwz     r1, 0(r11)                                          # Get new stack pointer from TCB

    epilogue                                                    # Restore context

    rfi                                                         # Run task


#*********************************************************************************************************
#*                                      INTERRUPT LEVEL CONTEXT SWITCH
#*
#* Description : This function is called by OSIntExit() to perform a context switch to a task that has
#*               been made ready-to-run by an ISR.
#*********************************************************************************************************
    .align  16
OSIntCtxSw:
    mbar    1

    bl      OSTaskSwHook                                        # Call OSTaskSwHook

    lis     r11,OSTCBHighRdy@ha                                 # Get pointer to ready task TCB
    lwz     r11,OSTCBHighRdy@l(r11)

    lis     r12,OSTCBCur@ha                                     # Save as current task TCB ptr.
    stw     r11,OSTCBCur@l(r12)

    lis     r12,OSPrioHighRdy@ha                                # Get High Ready Priority
    lbz     r10,OSPrioHighRdy@l(r12)

    lis     r12,OSPrioCur@ha                                    # Save as Current Priority
    stb     r10,OSPrioCur@l(r12)

    lwz     r1, 0(r11)                                          # Get new stack pointer

    epilogue                                                    # Restore context

    mbar    1

    rfi                                                         # Run task


#*********************************************************************************************************
#*                                     RETURN FROM INTERRUPT TRAP
#*********************************************************************************************************
    .align  16
RFI_trap:
    rfi


#*********************************************************************************************************
#*                                     CPU ASSEMBLY PORT FILE END
#*********************************************************************************************************
    .end
