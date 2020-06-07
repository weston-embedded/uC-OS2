# uC/OS-II

μC/OS-II is a portable, ROMable, scalable, preemptive, real-time deterministic multitasking kernel for microprocessors, microcontrollers and DSPs.
Offering unprecedented ease-of-use, μC/OS-II is delivered with complete 100% ANSI C source code and in-depth documentation. μC/OS-II runs on the largest number of processor architectures, with ports available for download from the Micrium Web site.

## For the complete documentation, visit https://doc.micrium.com/display/ucos/

---------------
# Modifications
The ARM Cortex-M ports (Ports/ARM-Cortex-M directory) have been modified to comply with CMSIS. Specifically, the PendSV and SysTick exception handlers have been renamed to the CMSIS-compliant names PendSV_Handler (originally was OS_CPU_PendSVHandler) and SysTick_Handler (originally was OS_CPU_SysTickHandler). This change is necessary for the uC/OS-II applications to be able to use the CMSIS-compliant startup code.
     
