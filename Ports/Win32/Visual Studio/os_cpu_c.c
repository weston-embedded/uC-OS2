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
*                                    Microsoft Win32 Specific code
*
* Filename : os_cpu_c.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#define   OS_CPU_GLOBALS


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <lib_def.h>
#include  <ucos_ii.h>

#define  _WIN32_WINNT  0x0600
#define   WIN32_LEAN_AND_MEAN

#include  <windows.h>
#include  <mmsystem.h>
#include  <stdio.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  WIN32_SLEEP                                        1u
#define  WIN32_MM_TMR                                       2u          /* Use the high resolution Multimedia timer.                */

#define  TIMER_METHOD                       WIN32_MM_TMR

#define  WIN_MM_MIN_RES                                     1u          /* Minimum timer resolution.                                */

#define  OS_MSG_TRACE                                       1u          /* Allow print trace messages.                              */

#ifdef  _MSC_VER
#define  MS_VC_EXCEPTION                           0x406D1388
#endif


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef  enum  os_task_state {
    STATE_NONE = 0,
    STATE_CREATED,
    STATE_RUNNING,
    STATE_SUSPENDED,
    STATE_INTERRUPTED,
    STATE_TERMINATING,
    STATE_TERMINATED
} OS_TASK_STATE;


typedef  struct  os_task_stk {
    void                      *TaskArgPtr;
    INT16U                     TaskOpt;
    void                     (*Task)(void*);
    HANDLE                     ThreadHandle;
    DWORD                      ThreadID;
    volatile  OS_TASK_STATE    TaskState;
    HANDLE                     SignalPtr;                               /* Task synchronization signal.                             */
    HANDLE                     InitSignalPtr;                           /* Task created         signal.                             */
    CPU_BOOLEAN                Terminate;                               /* Task terminate flag.                                     */
    OS_TCB                    *OSTCBPtr;
} OS_TASK_STK;


#ifdef _MSC_VER
#pragma pack(push,8)
typedef  struct  threadname_info {
    DWORD   dwType;                                                     /* Must be 0x1000.                                          */
    LPCSTR  szName;                                                     /* Pointer to name (in user addr space).                    */
    DWORD   dwThreadID;                                                 /* Thread ID (-1 = caller thread).                          */
    DWORD   dwFlags;                                                    /* Reserved for future use, must be zero.                   */
} THREADNAME_INFO;
#pragma pack(pop)
#endif


#if (TIMER_METHOD == WIN32_MM_TMR)
#ifdef _MSC_VER
#pragma  comment (lib, "winmm.lib")
#endif
#endif


/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*/

#if (OS_VERSION >= 281u) && (OS_TMR_EN > 0u)
static  INT16U    OSTmrCtr;
#endif                                                                  /* #if (OS_VERSION >= 281) && (OS_TMR_EN > 0)               */

static  HANDLE    OSTerminate_SignalPtr;

static  HANDLE    OSTick_Thread;
static  DWORD     OSTick_ThreadId;
#if (TIMER_METHOD == WIN32_MM_TMR)
static  HANDLE    OSTick_SignalPtr;
static  TIMECAPS  OSTick_TimerCap;
static  MMRESULT  OSTick_TimerId;
#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  DWORD  WINAPI  OSTickW32         (LPVOID        p_arg);
static  DWORD  WINAPI  OSTaskW32         (LPVOID        p_arg);

static  void           OSTaskTerminate   (OS_TASK_STK  *p_stk);

static  BOOL   WINAPI  OSCtrlBreakHandler(DWORD         ctrl);

static  void           OSSetThreadName   (DWORD         thread_id,
                                          INT8U        *p_name);

#if (OS_MSG_TRACE > 0u)
static  int            OS_Printf         (char         *p_str, ...);
#endif


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : None.
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*
*              2) Kernel objects must have unique names. Otherwise, a duplicate handle will be given for
*                 consecutive created objects. A GetLastError() ERROR_ALREADY_EXISTS can be checked when
*                 this case happens.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION > 203u)
void  OSInitHookBegin (void)
{
    HANDLE  hProc;


#if (OS_VERSION >= 281u) && (OS_TMR_EN > 0u)
    OSTmrCtr = 0u;
#endif

#if (TIMER_METHOD     == WIN32_SLEEP) && \
    (OS_TICKS_PER_SEC >  100u) && \
    (OS_MSG_TRACE     >    0u)
    OS_Printf("Warning: Sleep TIMER_METHOD cannot maintain time accuracy with the current setting of OS_TICKS_PER_SEC. Consider using Multimedia TIMER_METHOD.\n\n");
#endif


    OSTerminate_SignalPtr = NULL;
    OSTick_Thread         = NULL;
#if (TIMER_METHOD == WIN32_MM_TMR)
    OSTick_SignalPtr      = NULL;
#endif


    CPU_IntInit();                                                      /* Initialize Critical Section objects.                     */


    hProc = GetCurrentProcess();
    SetPriorityClass(hProc, HIGH_PRIORITY_CLASS);
    SetProcessAffinityMask(hProc, 1);

    OSSetThreadName(GetCurrentThreadId(), (INT8U *)"main()");


    OSTerminate_SignalPtr = CreateEvent(NULL, TRUE, FALSE, NULL);       /* Manual reset enabled to broadcast terminate signal.      */
    if (OSTerminate_SignalPtr == NULL) {
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Error: CreateEvent [OSTerminate] failed.\n");
#endif
        return;
    }
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)OSCtrlBreakHandler, TRUE);

    OSTick_Thread = CreateThread(NULL, 0, OSTickW32, 0, CREATE_SUSPENDED, &OSTick_ThreadId);
    if (OSTick_Thread == NULL) {
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Error: CreateThread [OSTickW32] failed.\n");
#endif
        CloseHandle(OSTerminate_SignalPtr);
        OSTerminate_SignalPtr = NULL;
        return;
    }

#if (OS_MSG_TRACE > 0u)
    OS_Printf("OSTick    created, Thread ID %5.0d\n", OSTick_ThreadId);
#endif

    SetThreadPriority(OSTick_Thread, THREAD_PRIORITY_HIGHEST);

#if (TIMER_METHOD == WIN32_MM_TMR)
    if (timeGetDevCaps(&OSTick_TimerCap, sizeof(OSTick_TimerCap)) != TIMERR_NOERROR) {
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Error: Cannot retrieve Timer capabilities.\n");
#endif
        CloseHandle(OSTick_Thread);
        CloseHandle(OSTerminate_SignalPtr);

        OSTick_Thread         = NULL;
        OSTerminate_SignalPtr = NULL;
        return;
    }

    if (OSTick_TimerCap.wPeriodMin < WIN_MM_MIN_RES) {
        OSTick_TimerCap.wPeriodMin = WIN_MM_MIN_RES;
    }

    if (timeBeginPeriod(OSTick_TimerCap.wPeriodMin) != TIMERR_NOERROR) {
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Error: Cannot set Timer minimum resolution.\n");
#endif
        CloseHandle(OSTick_Thread);
        CloseHandle(OSTerminate_SignalPtr);

        OSTick_Thread         = NULL;
        OSTerminate_SignalPtr = NULL;
        return;
    }

    OSTick_SignalPtr = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (OSTick_SignalPtr == NULL) {
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Error: CreateEvent [OSTick] failed.\n");
#endif
        timeEndPeriod(OSTick_TimerCap.wPeriodMin);
        CloseHandle(OSTick_Thread);
        CloseHandle(OSTerminate_SignalPtr);

        OSTick_Thread         = NULL;
        OSTerminate_SignalPtr = NULL;
        return;
    }

#ifdef _MSC_VER
#pragma warning (disable : 4055)
#endif
    OSTick_TimerId = timeSetEvent((UINT          )(1000u / OS_TICKS_PER_SEC),
                                  (UINT          ) OSTick_TimerCap.wPeriodMin,
                                  (LPTIMECALLBACK) OSTick_SignalPtr,
                                  (DWORD_PTR     ) NULL,
                                  (UINT          )(TIME_PERIODIC | TIME_CALLBACK_EVENT_SET));
#ifdef _MSC_VER
#pragma warning (default : 4055)
#endif

    if (OSTick_TimerId == 0u) {
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Error: Cannot start Timer.\n");
#endif
        CloseHandle(OSTick_SignalPtr);
        timeEndPeriod(OSTick_TimerCap.wPeriodMin);
        CloseHandle(OSTick_Thread);
        CloseHandle(OSTerminate_SignalPtr);

        OSTick_SignalPtr      = NULL;
        OSTick_Thread         = NULL;
        OSTerminate_SignalPtr = NULL;
        return;
    }
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
* Arguments  : None.
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION > 203u)
void  OSInitHookEnd (void)
{
}
#endif


/*
*********************************************************************************************************
*                                         TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : p_tcb        Pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u)
void  OSTaskCreateHook (OS_TCB  *p_tcb)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskCreateHook(p_tcb);
#else
    (void)p_tcb;                                                        /* Prevent compiler warning                                 */
#endif
}
#endif


/*
*********************************************************************************************************
*                                         TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : p_tcb        Pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u)
void  OSTaskDelHook (OS_TCB  *p_tcb)
{
    OS_TASK_STK  *p_stk;


#if (OS_APP_HOOKS_EN > 0u)
    App_TaskDelHook(p_tcb);
#endif

    p_stk = (OS_TASK_STK *)p_tcb->OSTCBStkPtr;

    switch (p_stk->TaskState) {
        case STATE_RUNNING:
             if (GetCurrentThreadId() == p_stk->ThreadID) {
                p_stk->Terminate = DEF_TRUE;
                p_stk->TaskState = STATE_TERMINATING;

             } else {

                 TerminateThread(p_stk->ThreadHandle, 0xFFFFFFFF);
                 CloseHandle(p_stk->ThreadHandle);

                 OSTaskTerminate(p_stk);
             }
             break;


        case STATE_CREATED:
        case STATE_SUSPENDED:
        case STATE_INTERRUPTED:
             TerminateThread(p_stk->ThreadHandle, 0xFFFFFFFF);
             CloseHandle(p_stk->ThreadHandle);

             OSTaskTerminate(p_stk);
             break;


        default:
             break;
    }
}
#endif


/*
*********************************************************************************************************
*                                           IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : None.
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION >= 251u)
void  OSTaskIdleHook (void)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskIdleHook();
#endif

    Sleep(1u);                                                          /* Reduce CPU utilization.                                  */
}
#endif


/*
*********************************************************************************************************
*                                          TASK RETURN HOOK
*
* Description: This function is called if a task accidentally returns.  In other words, a task should
*              either be an infinite loop or delete itself when done.
*
* Arguments  : p_tcb        Pointer to the task control block of the task that is returning.
*
* Note(s)    : None.
*********************************************************************************************************
*/

#if (OS_CPU_HOOKS_EN > 0u)
void  OSTaskReturnHook (OS_TCB  *p_tcb)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskReturnHook(p_tcb);
#else
    (void)p_tcb;                                                        /* Prevent compiler warning                                 */
#endif
}
#endif


/*
*********************************************************************************************************
*                                         STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : None.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u)
void  OSTaskStatHook (void)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskStatHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                      INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created. This function is highly processor specific.
*
* Arguments  : task         Pointer to the task code.
*
*              p_arg        Pointer to a user supplied data area that will be passed to the task
*                               when the task first executes.
*
*              ptos         Pointer to the top of stack. It is assumed that 'ptos' points to the
*                               highest valid address on the stack.
*
*              opt          Options used to alter the behavior of OSTaskStkInit().
*                               (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*********************************************************************************************************
*/

OS_STK  *OSTaskStkInit (void  (*task)(void  *pd), void  *p_arg, OS_STK  *ptos, INT16U  opt)
{
    OS_TASK_STK  *p_stk;

                                                                        /* Load stack pointer                                       */
    p_stk                = (OS_TASK_STK *)((char *)ptos - sizeof(OS_TASK_STK));
    p_stk->TaskArgPtr    =  p_arg;
    p_stk->TaskOpt       =  opt;
    p_stk->Task          =  task;
    p_stk->ThreadHandle  =  NULL;
    p_stk->ThreadID      =  0u;
    p_stk->TaskState     =  STATE_NONE;
    p_stk->SignalPtr     =  NULL;
    p_stk->InitSignalPtr =  NULL;
    p_stk->Terminate     =  DEF_FALSE;
    p_stk->OSTCBPtr      =  NULL;

    return ((OS_STK *)p_stk);
}


/*
*********************************************************************************************************
*                                          TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : None.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_TASK_SW_HOOK_EN > 0u)
void  OSTaskSwHook (void)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TaskSwHook();
#endif
}
#endif


/*
*********************************************************************************************************
*                                          OS_TCBInit() HOOK
*
* Description: This function is called by OS_TCBInit() after setting up most of the task control block.
*
* Arguments  : p_tcb        Pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*
*              2) Kernel objects must have unique names. Otherwise, a duplicate handle will be given for
*                 consecutive created objects. A GetLastError() ERROR_ALREADY_EXISTS can be checked when
*                 this case happens.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_VERSION > 203u)
void  OSTCBInitHook (OS_TCB  *p_tcb)
{
    OS_TASK_STK  *p_stk;


#if (OS_APP_HOOKS_EN > 0u)
    App_TCBInitHook(p_tcb);
#else
    (void)p_tcb;                                                        /* Prevent compiler warning                                 */
#endif

    p_stk = (OS_TASK_STK *)p_tcb->OSTCBStkPtr;

    p_stk->SignalPtr = CreateEvent(NULL, FALSE, FALSE, NULL);           /* See Note #2.                                             */
    if (p_stk->SignalPtr == NULL) {
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Task[%3.1d] cannot allocate signal event.\n", p_tcb->OSTCBPrio);
#endif
        return;
    }

    p_stk->InitSignalPtr = CreateEvent(NULL, TRUE, FALSE, NULL);        /* See Note #2.                                             */
    if (p_stk->InitSignalPtr == NULL) {
        CloseHandle(p_stk->SignalPtr);
        p_stk->SignalPtr = NULL;

#if (OS_MSG_TRACE > 0u)
        OS_Printf("Task[%3.1d] cannot allocate initialization complete signal event.\n", p_tcb->OSTCBPrio);
#endif
        return;
    }

    p_stk->ThreadHandle = CreateThread(NULL, 0, OSTaskW32, p_tcb, CREATE_SUSPENDED, &p_stk->ThreadID);
    if (p_stk->ThreadHandle == NULL) {
        CloseHandle(p_stk->InitSignalPtr);
        CloseHandle(p_stk->SignalPtr);

        p_stk->InitSignalPtr = NULL;
        p_stk->SignalPtr     = NULL;
#if (OS_MSG_TRACE > 0u)
        OS_Printf("Task[%3.1d] failed to be created.\n", p_tcb->OSTCBPrio);
#endif
        return;
    }

#if (OS_MSG_TRACE > 0u)
    OS_Printf("Task[%3.1d] created, Thread ID %5.0d\n", p_tcb->OSTCBPrio, p_stk->ThreadID);
#endif

    p_stk->TaskState = STATE_CREATED;
    p_stk->OSTCBPtr  = p_tcb;
}
#endif


/*
*********************************************************************************************************
*                                              TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : None.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (OS_CPU_HOOKS_EN > 0u) && (OS_TIME_TICK_HOOK_EN > 0u)
void  OSTimeTickHook (void)
{
#if (OS_APP_HOOKS_EN > 0u)
    App_TimeTickHook();
#endif

#if (OS_VERSION >= 281u) && (OS_TMR_EN > 0u)
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0u;
        OSTmrSignal();
    }
#endif
}
#endif


/*
*********************************************************************************************************
*                              START HIGHEST PRIORITY TASK READY-TO-RUN
*
* Description: This function is called by OSStart() to start the highest priority task that was created
*              by your application before calling OSStart().
*
* Arguments  : None.
*
* Note(s)    : 1) OSStartHighRdy() MUST:
*                      a) Call OSTaskSwHook() then,
*                      b) Set OSRunning to TRUE,
*                      c) Switch to the highest priority task.
*********************************************************************************************************
*/

void  OSStartHighRdy (void)
{
    OS_TASK_STK  *p_stk;
    OS_TCB       *p_tcb;
    INT8U         prio;
    CPU_SR_ALLOC();


    OSTaskSwHook();
    OSRunning = 1;

    p_stk = (OS_TASK_STK *)OSTCBHighRdy->OSTCBStkPtr;                   /* OSTCBCur  = OSTCBHighRdy;                                */
                                                                        /* OSPrioCur = OSPrioHighRdy;                               */
    ResumeThread(p_stk->ThreadHandle);
                                                                        /* Wait while task is created and until it is ready to run. */
    SignalObjectAndWait(p_stk->SignalPtr, p_stk->InitSignalPtr, INFINITE, FALSE);
    ResumeThread(OSTick_Thread);                                        /* Start OSTick Thread.                                     */
    WaitForSingleObject(OSTick_Thread, INFINITE);                       /* Wait until OSTick Thread has terminated.                 */


#if (TIMER_METHOD == WIN32_MM_TMR)
    timeKillEvent(OSTick_TimerId);
    timeEndPeriod(OSTick_TimerCap.wPeriodMin);
    CloseHandle(OSTick_SignalPtr);
#endif

    CloseHandle(OSTick_Thread);
    CloseHandle(OSTerminate_SignalPtr);


#if (OS_MSG_TRACE > 0u)
    OS_Printf("\nDeleting uC/OS-II tasks...\n");
#endif
                                                                        /* Delete all created tasks/threads.                        */
    CPU_CRITICAL_ENTER();
    p_tcb = OSTCBList;
    while (p_tcb != (OS_TCB *)0) {
        if (p_tcb != OS_TCB_RESERVED) {
            prio = p_tcb->OSTCBPrio;
            if (prio == OS_TASK_IDLE_PRIO) {
                OSTaskDelHook(p_tcb);
                p_tcb = p_tcb->OSTCBNext;
            } else {
                p_tcb = p_tcb->OSTCBNext;
               (void)OSTaskDel(prio);
            }
        } else {
            p_tcb = p_tcb->OSTCBNext;
        }
    }
    CPU_CRITICAL_EXIT();

    CPU_IntEnd();                                                       /* Delete Critical Section objects.                         */
}


/*
*********************************************************************************************************
*                                      TASK LEVEL CONTEXT SWITCH
*
* Description: This function is called when a task makes a higher priority task ready-to-run.
*
* Arguments  : None.
*
* Note(s)    : 1) Upon entry,
*                 OSTCBCur     points to the OS_TCB of the task to suspend
*                 OSTCBHighRdy points to the OS_TCB of the task to resume
*
*              2) OSCtxSw() MUST:
*                      a) Save processor registers then,
*                      b) Save current task's stack pointer into the current task's OS_TCB,
*                      c) Call OSTaskSwHook(),
*                      d) Set OSTCBCur = OSTCBHighRdy,
*                      e) Set OSPrioCur = OSPrioHighRdy,
*                      f) Switch to the highest priority task.
*
*                      pseudo-code:
*                           void  OSCtxSw (void)
*                           {
*                               Save processor registers;
*
*                               OSTCBCur->OSTCBStkPtr =  SP;
*
*                               OSTaskSwHook();
*
*                               OSTCBCur              =  OSTCBHighRdy;
*                               OSPrioCur             =  OSPrioHighRdy;
*
*                               Restore processor registers from (OSTCBHighRdy->OSTCBStkPtr);
*                           }
*********************************************************************************************************
*/

void  OSCtxSw (void)
{
    OS_TASK_STK  *p_stk;
    OS_TASK_STK  *p_stk_new;
#if (OS_MSG_TRACE > 0u)
    OS_TCB       *p_tcb_cur;
    OS_TCB       *p_tcb_new;
#endif
    CPU_SR_ALLOC();


#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    cpu_sr = 0u;
#endif

#if (OS_MSG_TRACE > 0u)
    p_tcb_cur = OSTCBCur;
    p_tcb_new = OSTCBHighRdy;
#endif


    p_stk = (OS_TASK_STK *)OSTCBCur->OSTCBStkPtr;

    OSTaskSwHook();

    OSTCBCur  = OSTCBHighRdy;
    OSPrioCur = OSPrioHighRdy;

    if (p_stk->TaskState == STATE_RUNNING) {
        p_stk->TaskState  = STATE_SUSPENDED;
    }
    p_stk_new = (OS_TASK_STK *)OSTCBHighRdy->OSTCBStkPtr;
    switch (p_stk_new->TaskState) {
        case STATE_CREATED:                                             /* TaskState updated to STATE_RUNNING once thread runs.     */
             ResumeThread(p_stk_new->ThreadHandle);
                                                                        /* Wait while task is created and until it is ready to run. */
             SignalObjectAndWait(p_stk_new->SignalPtr, p_stk_new->InitSignalPtr, INFINITE, FALSE);
             break;


        case STATE_SUSPENDED:
             p_stk_new->TaskState = STATE_RUNNING;
             SetEvent(p_stk_new->SignalPtr);
             break;


        case STATE_INTERRUPTED:
             p_stk_new->TaskState = STATE_RUNNING;
             ResumeThread(p_stk_new->ThreadHandle);
             break;


#if (OS_MSG_TRACE > 0u)
        case STATE_NONE:
             OS_Printf("[OSCtxSw] Error: Invalid state STATE_NONE\nCur    Task[%3.1d] Thread ID %5.0d: '%s'\nNew    Task[%3.1d] Thread ID %5.0d: '%s'\n\n",
                       p_tcb_cur->OSTCBPrio,
                       p_stk->ThreadID,
                       p_tcb_cur->OSTCBTaskName,
                       p_tcb_new->OSTCBPrio,
                       p_stk_new->ThreadID,
                       p_tcb_new->OSTCBTaskName);
             return;


        case STATE_RUNNING:
             OS_Printf("[OSCtxSw] Error: Invalid state STATE_RUNNING\nCur    Task[%3.1d] Thread ID %5.0d: '%s'\nNew    Task[%3.1d] Thread ID %5.0d: '%s'\n\n",
                       p_tcb_cur->OSTCBPrio,
                       p_stk->ThreadID,
                       p_tcb_cur->OSTCBTaskName,
                       p_tcb_new->OSTCBPrio,
                       p_stk_new->ThreadID,
                       p_tcb_new->OSTCBTaskName);
             return;


        case STATE_TERMINATING:
             OS_Printf("[OSCtxSw] Error: Invalid state STATE_TERMINATING\nCur    Task[%3.1d] Thread ID %5.0d: '%s'\nNew    Task[%3.1d] Thread ID %5.0d: '%s'\n\n",
                       p_tcb_cur->OSTCBPrio,
                       p_stk->ThreadID,
                       p_tcb_cur->OSTCBTaskName,
                       p_tcb_new->OSTCBPrio,
                       p_stk_new->ThreadID,
                       p_tcb_new->OSTCBTaskName);
             return;


        case STATE_TERMINATED:
             OS_Printf("[OSCtxSw] Error: Invalid state STATE_TERMINATED\nCur    Task[%3.1d] Thread ID %5.0d: '%s'\nNew    Task[%3.1d] Thread ID %5.0d: '%s'\n\n",
                       p_tcb_cur->OSTCBPrio,
                       p_stk->ThreadID,
                       p_tcb_cur->OSTCBTaskName,
                       p_tcb_new->OSTCBPrio,
                       p_stk_new->ThreadID,
                       p_tcb_new->OSTCBTaskName);
             return;


#endif
        default:
             return;
    }


    if (p_stk->Terminate == DEF_TRUE) {
        OSTaskTerminate(p_stk);

        CPU_CRITICAL_EXIT();

        ExitThread(0u);                                                 /* ExitThread() never returns.                              */
        return;
    }
    CPU_CRITICAL_EXIT();
    WaitForSingleObject(p_stk->SignalPtr, INFINITE);
    CPU_CRITICAL_ENTER();
}


/*
*********************************************************************************************************
*                                   INTERRUPT LEVEL CONTEXT SWITCH
*
* Description: This function is called by OSIntExit() to perform a context switch from an ISR.
*
* Arguments  : None.
*
* Note(s)    : 1) OSIntCtxSw() MUST:
*                      a) Call OSTaskSwHook() then,
*                      b) Set OSTCBCur = OSTCBHighRdy,
*                      c) Set OSPrioCur = OSPrioHighRdy,
*                      d) Switch to the highest priority task.
*
*              2) OSIntCurTaskSuspend() MUST be called prior to OSIntEnter().
*
*              3) OSIntCurTaskResume()  MUST be called after    OSIntExit() to switch to the highest
*                 priority task.
*********************************************************************************************************
*/

void  OSIntCtxSw (void)
{
    OSTaskSwHook();

    OSTCBCur  = OSTCBHighRdy;
    OSPrioCur = OSPrioHighRdy;
}


/*
*********************************************************************************************************
*                                        OSIntCurTaskSuspend()
*
* Description: This function suspends current task for context switch.
*
* Arguments  : None.
*
* Returns    : DEF_TRUE,  current task     suspended successfully.
*              DEF_FALSE, current task NOT suspended.
*
* Notes      : 1) Current task MUST be suspended before OSIntEnter().
*
*              2) Suspending current task before OSIntEnter() and resuming it after OSIntExit() prevents
*                 task-level code to run concurrently with ISR-level code
*********************************************************************************************************
*/

CPU_BOOLEAN  OSIntCurTaskSuspend (void)
{
    OS_TCB       *p_tcb;
    OS_TASK_STK  *p_stk;
    CPU_BOOLEAN   ret;


    p_tcb =  OSTCBCur;
    p_stk = (OS_TASK_STK *)p_tcb->OSTCBStkPtr;
    switch (p_stk->TaskState) {
        case STATE_RUNNING:
             SuspendThread(p_stk->ThreadHandle);
             SwitchToThread();

             p_stk->TaskState = STATE_INTERRUPTED;

             ret = DEF_TRUE;
             break;


        case STATE_TERMINATING:                                         /* Task has terminated (run-to-completion/deleted itself).  */
             TerminateThread(p_stk->ThreadHandle, 0xFFFFFFFF);
             CloseHandle(p_stk->ThreadHandle);

             OSTaskTerminate(p_stk);

             ret = DEF_TRUE;
             break;


#if (OS_MSG_TRACE > 0u)
        case STATE_NONE:
             OS_Printf("[OSIntCtxSw Suspend] Error: Invalid state STATE_NONE\nCur    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk->ThreadID);

             ret = DEF_FALSE;
             break;


        case STATE_CREATED:
             OS_Printf("[OSIntCtxSw Suspend] Error: Invalid state STATE_CREATED\nCur    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk->ThreadID);

             ret = DEF_FALSE;
             break;


        case STATE_INTERRUPTED:
             OS_Printf("[OSIntCtxSw Suspend] Error: Invalid state STATE_INTERRUPTED\nCur    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk->ThreadID);

             ret = DEF_FALSE;
             break;


        case STATE_SUSPENDED:
             OS_Printf("[OSIntCtxSw Suspend] Error: Invalid state STATE_SUSPENDED\nCur    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk->ThreadID);

             ret = DEF_FALSE;
             break;


        case STATE_TERMINATED:
             OS_Printf("[OSIntCtxSw Suspend] Error: Invalid state STATE_TERMINATED\nCur    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk->ThreadID);

             ret = DEF_FALSE;
             break;


#endif
        default:
             ret = DEF_FALSE;
             break;
    }

    return (ret);
}


/*
*********************************************************************************************************
*                                        OSIntCurTaskResume()
*
* Description: This function resumes current task for context switch.
*
* Arguments  : None.
*
* Returns    : DEF_TRUE,  current task     resumed successfully.
*              DEF_FALSE, current task NOT resumed.
*
* Notes      : 1) Current task MUST be resumed after OSIntExit().
*
*              2) Suspending current task before OSIntEnter() and resuming it after OSIntExit() prevents
*                 task-level code to run concurrently with ISR-level code
*********************************************************************************************************
*/

CPU_BOOLEAN  OSIntCurTaskResume (void)
{
    OS_TCB       *p_tcb;
    OS_TASK_STK  *p_stk_new;
    CPU_BOOLEAN   ret;


    p_tcb     =  OSTCBHighRdy;
    p_stk_new = (OS_TASK_STK *)p_tcb->OSTCBStkPtr;
    switch (p_stk_new->TaskState) {
        case STATE_CREATED:
             ResumeThread(p_stk_new->ThreadHandle);
                                                                        /* Wait while task is created and until it is ready to run. */
             SignalObjectAndWait(p_stk_new->SignalPtr, p_stk_new->InitSignalPtr, INFINITE, FALSE);
             ret = DEF_TRUE;
             break;


        case STATE_INTERRUPTED:
             p_stk_new->TaskState = STATE_RUNNING;
             ResumeThread(p_stk_new->ThreadHandle);
             ret = DEF_TRUE;
             break;


        case STATE_SUSPENDED:
             p_stk_new->TaskState = STATE_RUNNING;
             SetEvent(p_stk_new->SignalPtr);
             ret = DEF_TRUE;
             break;


#if (OS_MSG_TRACE > 0u)
        case STATE_NONE:
             OS_Printf("[OSIntCtxSw Resume] Error: Invalid state STATE_NONE\nNew    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk_new->ThreadID);
             ret = DEF_FALSE;
             break;


        case STATE_RUNNING:
             OS_Printf("[OSIntCtxSw Resume] Error: Invalid state STATE_RUNNING\nNew    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk_new->ThreadID);
             ret = DEF_FALSE;
             break;


        case STATE_TERMINATING:
             OS_Printf("[OSIntCtxSw Resume] Error: Invalid state STATE_TERMINATING\nNew    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk_new->ThreadID);
             ret = DEF_FALSE;
             break;


        case STATE_TERMINATED:
             OS_Printf("[OSIntCtxSw Resume] Error: Invalid state STATE_TERMINATED\nNew    Task[%3.1d] '%s' Thread ID %5.0d\n",
                       p_tcb->OSTCBPrio,
                       p_tcb->OSTCBTaskName,
                       p_stk_new->ThreadID);
             ret = DEF_FALSE;
             break;


#endif
        default:
             ret = DEF_FALSE;
             break;
    }

    return (ret);
}


/*
*********************************************************************************************************
*                                      WIN32 TASK - OSTickW32()
*
* Description: This functions is the Win32 task that generates the tick interrupts for uC/OS-II.
*
* Arguments  : p_arg        Pointer to argument of the task.
*
* Note(s)    : 1) Priorities of these tasks are very important.
*********************************************************************************************************
*/

static  DWORD  WINAPI  OSTickW32 (LPVOID  p_arg)
{
    CPU_BOOLEAN  terminate;
    CPU_BOOLEAN  suspended;
#if (TIMER_METHOD == WIN32_MM_TMR)
    HANDLE       wait_signal[2];
#endif
    CPU_SR_ALLOC();


#if (TIMER_METHOD == WIN32_MM_TMR)
    wait_signal[0] = OSTerminate_SignalPtr;
    wait_signal[1] = OSTick_SignalPtr;
#endif


    (void)p_arg;                                                        /* Prevent compiler warning                                 */

    terminate = DEF_FALSE;
    while (!terminate) {
#if   (TIMER_METHOD == WIN32_MM_TMR)
        switch (WaitForMultipleObjects(2, wait_signal, FALSE, INFINITE)) {
            case WAIT_OBJECT_0 + 1u:
                 ResetEvent(OSTick_SignalPtr);
#elif (TIMER_METHOD == WIN32_SLEEP)
        switch (WaitForSingleObject(OSTerminate_SignalPtr, 1000u / OS_TICKS_PER_SEC)) {
            case WAIT_TIMEOUT:
#endif
                 CPU_CRITICAL_ENTER();

                 suspended = OSIntCurTaskSuspend();
                 if (suspended == DEF_TRUE) {
                     OSIntEnter();
                     OSTimeTick();
                     OSIntExit();
                     OSIntCurTaskResume();
                 }

                 CPU_CRITICAL_EXIT();
                 break;


            case WAIT_OBJECT_0 + 0u:
                 terminate = DEF_TRUE;
                 break;


            default:
#if (OS_MSG_TRACE > 0u)
                 OS_Printf("[OSTickW32] Error: Invalid signal.\n");
#endif
                 terminate = DEF_TRUE;
                 break;
        }
    }

#if (OS_MSG_TRACE > 0u)
    OS_Printf("[OSTickW32] Terminated.\n");
#endif

    return (0u);
}


/*
*********************************************************************************************************
*                                      WIN32 TASK - OSTaskW32()
*
* Description: This function is a generic Win32 task wrapper for uC/OS-II tasks.
*
* Arguments  : p_arg        Pointer to argument of the task.
*
* Note(s)    : 1) Priorities of these tasks are very important.
*********************************************************************************************************
*/

static  DWORD  WINAPI  OSTaskW32 (LPVOID  p_arg)
{
    OS_TASK_STK  *p_stk;
    OS_TCB       *p_tcb;


    p_tcb = (OS_TCB      *)p_arg;
    p_stk = (OS_TASK_STK *)p_tcb->OSTCBStkPtr;

    p_stk->TaskState = STATE_SUSPENDED;
    WaitForSingleObject(p_stk->SignalPtr, INFINITE);

    OSSetThreadName(p_stk->ThreadID, p_tcb->OSTCBTaskName);

#if (OS_MSG_TRACE > 0u)
    OS_Printf("Task[%3.1d] '%s' Running\n", p_tcb->OSTCBPrio, p_tcb->OSTCBTaskName);
#endif

    p_stk->TaskState = STATE_RUNNING;
    SetEvent(p_stk->InitSignalPtr);                                     /* Indicate task has initialized successfully.              */

    p_stk->Task(p_stk->TaskArgPtr);

    OSTaskDel(p_tcb->OSTCBPrio);                                        /* Thread may exit at OSCtxSw().                            */

    return (0u);
}


/*
*********************************************************************************************************
*                                          OSTaskTerminate()
*
* Description: This function handles task termination control signals.
*
* Arguments  : p_stk        Pointer to the stack of the task to clear its control signals.
*********************************************************************************************************
*/

static  void  OSTaskTerminate (OS_TASK_STK  *p_stk)
{
#if (OS_MSG_TRACE > 0u)
    OS_TCB       *p_tcb;
#endif


#if (OS_MSG_TRACE > 0u)
    p_tcb = p_stk->OSTCBPtr;
    OS_Printf("Task[%3.1d] '%s' Deleted\n", p_tcb->OSTCBPrio, p_tcb->OSTCBTaskName);
#endif
    CloseHandle(p_stk->InitSignalPtr);
    CloseHandle(p_stk->SignalPtr);

    p_stk->ThreadID      = 0u;
    p_stk->ThreadHandle  = NULL;
    p_stk->InitSignalPtr = NULL;
    p_stk->SignalPtr     = NULL;
    p_stk->TaskState     = STATE_TERMINATED;
    p_stk->OSTCBPtr      = NULL;
}


/*
*********************************************************************************************************
*                                        OSCtrlBreakHandler()
*
* Description: This function handles control signals sent to the console window.
*
* Arguments  : ctrl         Control signal type.
*
* Returns    : TRUE,  control signal was     handled.
*              FALSE, control signal was NOT handled.
*********************************************************************************************************
*/

static  BOOL  WINAPI  OSCtrlBreakHandler (DWORD  ctrl)
{
    BOOL  ret;


    ret = FALSE;

    switch (ctrl) {
        case CTRL_C_EVENT:                                              /* CTRL-C pressed.                                          */
        case CTRL_BREAK_EVENT:                                          /* CTRL-BREAK pressed.                                      */
        case CTRL_CLOSE_EVENT:                                          /* Console window is closing.                               */
        case CTRL_LOGOFF_EVENT:                                         /* Logoff has started.                                      */
        case CTRL_SHUTDOWN_EVENT:                                       /* System shutdown in process.                              */
#if (OS_MSG_TRACE > 0u)
             OS_Printf("\nTerminating Scheduler...\n");
#endif
             SetEvent(OSTerminate_SignalPtr);

             if (ctrl == CTRL_CLOSE_EVENT) {
                 Sleep(500);                                            /* Give a chance to OSTickW32 to terminate.                 */
             } else {
                 ret = TRUE;
             }
             break;


        default:
             break;
    }

    return (ret);
}


/*
*********************************************************************************************************
*                                             OS_Printf()
*
* Description: This function is analog of printf.
*
* Arguments  : p_str        Pointer to format string output.
*
* Returns    : Number of characters written.
*********************************************************************************************************
*/
#if (OS_MSG_TRACE > 0u)
static  int  OS_Printf (char  *p_str, ...)
{
    va_list  param;
    int      ret;


    va_start(param, p_str);
#ifdef _MSC_VER
    ret = vprintf_s(p_str, param);
#else
    ret = vprintf(p_str, param);
#endif
    va_end(param);

    return (ret);
}
#endif


/*
*********************************************************************************************************
*                                          OSDebuggerBreak()
*
* Description: This function throws a breakpoint exception when a debugger is present.
*
* Arguments  : None.
*********************************************************************************************************
*/

void  OSDebuggerBreak (void)
{
#ifdef _MSC_VER
    __try {
        DebugBreak();
    }
    __except(GetExceptionCode() == EXCEPTION_BREAKPOINT ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        return;
    }
#else
#ifdef _DEBUG
    DebugBreak();
#endif
#endif
}


/*
*********************************************************************************************************
*                                          OSSetThreadName()
*
* Description: This function sets thread names.
*
* Arguments  : thread_id    Thread ID.
*
*              p_name       Pointer to name of the thread string.
*
* Note(s)    : Visual Studio allows threads to be named into debug session.
*********************************************************************************************************
*/

static  void  OSSetThreadName (DWORD  thread_id, INT8U  *p_name)
{
#ifdef _MSC_VER
    THREADNAME_INFO  info;


    info.dwType     = (DWORD )0x1000u;
    info.szName     = (LPCSTR)p_name;
    info.dwThreadID = (DWORD )thread_id;
    info.dwFlags    = (DWORD )0u;

    __try {
        RaiseException(MS_VC_EXCEPTION, 0u, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
#endif
}
