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
*                                   Freescale DSP568xx Specific code
*                        Metrowerks CodeWarrior for Freescale Embedded DSP568xx
*
* Filename : os_cpu.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include "includes.h"
#include "AroPrintf.h"
#include "AS1.h"
#include "memoire.h"
#include "IO.h"
#include "AroCan.h"
#include "MesureBus.h"
#include "Superviseur.h"
#include "Resultat.h"
#include "EntreesSorties.h"
#include "Mesures.h"
#include "Apprentissage.h"
#include "Controleur_Intensite.h"
#include "Defauts.h"

#include "Diagnostique.h"
#include "CalculateurConsNormal.h"
#include "Regulateur.h"
#include "GenerateurAA.h"
#include "Shell.h"
OS_CPU_SR cpu_sr;

#ifdef TRACE_CPU
	UINT8 OSIdleCtrRunMax;
#endif

#ifdef DEBUG_REGU
	INT16 	g_wRegDebugEm;
	UINT16	g_uwRegDebugT_Soudage;
	UINT16	g_wRegDebugCommande;
	UINT16	g_uwRegDebugConsigne;
	UINT16	g_uwRegDebugGS;
#endif

#ifdef TRACE_TENSION_BUS
extern UINT16  g_uwDebugCBTension_Maxi;
#endif


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
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookBegin (void)
{

#ifdef TRACE_CPU
	OSIdleCtrRunMax=0;
#endif

#ifdef TRACE_TENSION_BUS
	g_uwDebugCBTension_Maxi=0;
#endif

#ifdef DEBUG_REGU
	g_wRegDebugCommande=30000;
#endif

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
* Returns    : none
*
*********************************************************************************************************
*/

#if OS_VERSION > 203
void OSInitHookEnd (void)
{
	Shell_vInit();
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
*              2) I decided to change the options on the statistic task to allow for floating-point in
*                 case you decide to do math. in OSTaskStatHook().
*********************************************************************************************************
*/
void OSTaskCreateHook (OS_TCB *ptcb)
{
	ptcb=ptcb;

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
	ptcb=ptcb;
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
	AS1_TComData ucTouche;
	 switch(AS1_RecvChar(&ucTouche))
	 {
	 	case ERR_OK :   Shell_vExecute(ucTouche);

	 	break;

	 	case ERR_SPEED : Aro_uwPrintf("ERR_SPEED\r");
	 	break;


	 	case ERR_BREAK : Aro_uwPrintf("ERR_BREAK\r");
	 	break;

	 	case ERR_COMMON : Aro_uwPrintf("ERR_COMMON\r");
	 	break;

	 	case ERR_RXEMPTY :
	 	break;

		default : Aro_uwPrintf("AUTRE\r");
	 }

	if(OSCPUUsage>OSIdleCtrRunMax)
	{
		OSIdleCtrRunMax=OSCPUUsage;
	}
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
	ptcb=ptcb;
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


/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION >= 205
void OSTaskIdleHook (void)
{

}

#endif


/*
*********************************************************************************************************
*  Helper routines to interface C to ASM
*********************************************************************************************************
*/
int  ValueOSPrioHighRdy (void);
void SetOSRunning (void);
void SetOSPrioCur (int Priority);


int ValueOSPrioHighRdy (void)
{
        return (int)OSPrioHighRdy;
}

void SetOSRunning (void)
{
        OSRunning = TRUE;
}

void SetOSPrioCur (int Priority)
{
        OSPrioCur = Priority;
}


/*
;*********************************************************************************************************
;                                            HANDLE TICK ISR
;
; Description: This function is called by the timer ISR periodically (every tick).
;              The exact amount of time for each tick depends on how the timer was configured.
;
; Arguments  : none
;
; Returns    : none
;
; Note(s)    : The following C-like pseudo-code describe the operation being performed in the code below.
;
;              if (!OSRunning) Return;
;
;              OSIntNesting++;
;
;              OSTimeTick();              Notify uC/OS-II that a tick has occured
;
;              OSIntExit();               Notify uC/OS-II about end of ISR
;
;              Return
;*********************************************************************************************************
*/

void OSTickISR (void)
{

	    if (OSRunning)
        {
              OSIntNesting++;     /* Increment interrupt nesting */

        OSTimeTick();       /* Process system tick */
        OSIntExit();        /* Notify uC/OS-II of end of ISR */
        }
}


#endif
