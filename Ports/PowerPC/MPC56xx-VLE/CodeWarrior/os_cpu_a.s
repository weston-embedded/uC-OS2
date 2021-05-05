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
#                                        Freescale  MPC56xx-VLE
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

    e_lis   r11,OSIntNesting@ha                                 # Load OSIntNesting
    e_lbz   r3, OSIntNesting@l(r11)
    e_addi  r0, r3, 1
    e_stb   r0, OSIntNesting@l(r11)                             # OSIntNesting++

    e_cmpl16i r0, 1                                             # if (OSIntNesting != 1) ...
    se_bne  OSExtIntISR_NotSaveSP                               # Do not save the stackpointer

    e_lis   r11,OSTCBCur@ha                                     # Get pointer to current TCB
    e_lwz   r11,OSTCBCur@l(r11)
    e_stw   r1, 0(r11)                                          # Save stack pointer in current TCB

OSExtIntISR_NotSaveSP:
    e_lis   r3, INTC_IACKR@h                                    # Store address of INTC.IACKR in r3
    e_or2i  r3, INTC_IACKR@l
    e_lwz   r3, 0(r3)                                           # Load contents of INTC.IACKR in r3
    e_lwz   r12,0(r3)                                           # Load the base adress of Vector Table

    mtctr   r12                                                 # Load Count register
    se_bctrl                                                    # Branch to ISR_Handler

    se_li   r0, 0                                               # Load r0 with 0x0000
    e_lis   r3, INTC_EOIR@h                                     # Get the address of the INTC.EOIR Register ...
    e_or2i  r3, INTC_EOIR@l
    e_stw   r0, 0(r3)                                           # And clear the INTC.EOIR register

    e_bl    OSIntExit                                           # Call to decrement OSIntNesting

    epilogue                                                    # Restore the context

    se_rfi                                                      # Return from interrupt


#*********************************************************************************************************
#*                                            SYSTEM TICK ISR
#*
#* Description : This function is the ISR to notify uC/OS-II that a system tick has occurred.
#*********************************************************************************************************
    .align  16
OSTickISR:
    prologue

    e_lis   r11,OSIntNesting@ha                                 # load OSIntNesting
    e_lbz   r3, OSIntNesting@l(r11)
    e_addi  r0, r3, 1
    e_stb   r0, OSIntNesting@l(r11)                             # OSIntNesting++

    e_cmpl16i r0, 1                                             # if (OSIntNesting != 1) ...
    se_bne  OSTickISR_NotSaveSP                                 # Do not save the stackpointer

    e_lis   r11,OSTCBCur@ha                                     # Get pointer to current TCB
    e_lwz   r11,OSTCBCur@l(r11)
    e_stw   r1, 0(r11)                                          # Save stack pointer in current TCB

OSTickISR_NotSaveSP:
    e_lis   r4, 0x0800                                          # Load r4 with TSR[DIS] bit (0x08000000)
    mtspr   TSR,r4                                              # Clear TSR[DIS] bit

    e_bl    BSP_TmrTickISR                                      # Call TmrTick handler
    e_bl    OSIntExit                                           # Call to decrement OSIntNesting

    epilogue                                                    # Restore context

    se_rfi                                                      # Run task


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
    se_blr

OS_CPU_SR_Restore:
    mtmsr   r3
    se_blr


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
    se_blr


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
    e_bl    OSTaskSwHook                                        # Call OSTaskSwHook

    e_li    r0, 1                                               # Set OSRunning to 1
    e_lis   r11,OSRunning@ha
    e_stb   r0, OSRunning@l(r11)

    e_lis   r11,OSTCBHighRdy@ha                                 # Get pointer to ready task TCB
    e_lwz   r11,OSTCBHighRdy@l(r11)

    e_lis   r12,OSTCBCur@ha                                     # Save as current task TCB ptr.
    e_stw   r11,OSTCBCur@l(r12)

    e_lwz   r1, 0(r11)                                          # Get new stack pointer

    epilogue                                                    # Restore context

    se_rfi                                                      # Run task


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

    e_lis   r11,OSTCBCur@ha                                     # Get pointer to current TCB
    e_lwz   r11,OSTCBCur@l(r11)
    e_stw   r1, 0(r11)                                          # Save stack pointer in current TCB

    e_bl    OSTaskSwHook                                        # Call OSTaskSwHook

    e_lis   r11,OSTCBHighRdy@ha                                 # Get pointer to ready task TCB
    e_lwz   r11,OSTCBHighRdy@l(r11)

    e_lis   r12,OSTCBCur@ha                                     # Save as current task TCB ptr.
    e_stw   r11,OSTCBCur@l(r12)

    e_lis   r12,OSPrioHighRdy@ha                                # Get High Ready Priority
    e_lbz   r10,OSPrioHighRdy@l(r12)

    e_lis   r12,OSPrioCur@ha                                    # Save as Current Priority
    e_stb   r10,OSPrioCur@l(r12)

    e_lwz   r1, 0(r11)                                          # Get new stack pointer from TCB

    epilogue                                                    # Restore context

    se_rfi                                                      # Run task


#*********************************************************************************************************
#*                                      INTERRUPT LEVEL CONTEXT SWITCH
#*
#* Description : This function is called by OSIntExit() to perform a context switch to a task that has
#*               been made ready-to-run by an ISR.
#*********************************************************************************************************
    .align  16
OSIntCtxSw:
    mbar    1

    e_bl    OSTaskSwHook                                        # Call OSTaskSwHook

    e_lis   r11,OSTCBHighRdy@ha                                 # Get pointer to ready task TCB
    e_lwz   r11,OSTCBHighRdy@l(r11)

    e_lis   r12,OSTCBCur@ha                                     # Save as current task TCB ptr.
    e_stw   r11,OSTCBCur@l(r12)

    e_lis   r12,OSPrioHighRdy@ha                                # Get High Ready Priority
    e_lbz   r10,OSPrioHighRdy@l(r12)

    e_lis   r12,OSPrioCur@ha                                    # Save as Current Priority
    e_stb   r10,OSPrioCur@l(r12)

    e_lwz   r1, 0(r11)                                          # Get new stack pointer

    epilogue                                                    # Restore context

    mbar    1

    se_rfi                                                      # Run task


#*********************************************************************************************************
#*                                     RETURN FROM INTERRUPT TRAP
#*********************************************************************************************************
    .align  16
RFI_trap:
    se_rfi


#*********************************************************************************************************
#*                                     CPU ASSEMBLY PORT FILE END
#*********************************************************************************************************
    .end
