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
* Filename : ucos_ii.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#ifndef  OS_uCOS_II_H
#define  OS_uCOS_II_H

#include  <os_cpu.h>
#include  <os_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************************************
*                                             MISCELLANEOUS
*********************************************************************************************************
*/

#define  OS_VERSION              270u                   /* Version of uC/OS-II (Vx.yy mult. by 100)    */

#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif

#ifndef  FALSE
#define  FALSE                     0
#endif

#ifndef  TRUE
#define  TRUE                      1
#endif

#define  OS_ASCII_NUL        (char)0

#define  OS_PRIO_SELF           0xFF                    /* Indicate SELF priority                      */

#if OS_TASK_STAT_EN > 0
#define  OS_N_SYS_TASKS            2                    /* Number of system tasks                      */
#else
#define  OS_N_SYS_TASKS            1
#endif

#define  OS_STAT_PRIO       (OS_LOWEST_PRIO - 1)        /* Statistic task priority                     */
#define  OS_IDLE_PRIO       (OS_LOWEST_PRIO)            /* IDLE      task priority                     */

#define  OS_EVENT_TBL_SIZE ((OS_LOWEST_PRIO) / 8 + 1)   /* Size of event table                         */
#define  OS_RDY_TBL_SIZE   ((OS_LOWEST_PRIO) / 8 + 1)   /* Size of ready table                         */

#define  OS_TASK_IDLE_ID       65535u                   /* I.D. numbers for Idle and Stat tasks        */
#define  OS_TASK_STAT_ID       65534u

#define  OS_EVENT_EN       (((OS_Q_EN > 0) && (OS_MAX_QS > 0)) || (OS_MBOX_EN > 0) || (OS_SEM_EN > 0) || (OS_MUTEX_EN > 0))


/*
*********************************************************************************************************
*                              TASK STATUS (Bit definition for OSTCBStat)
*********************************************************************************************************
*/
#define  OS_STAT_RDY            0x00        /* Ready to run                                            */
#define  OS_STAT_SEM            0x01        /* Pending on semaphore                                    */
#define  OS_STAT_MBOX           0x02        /* Pending on mailbox                                      */
#define  OS_STAT_Q              0x04        /* Pending on queue                                        */
#define  OS_STAT_SUSPEND        0x08        /* Task is suspended                                       */
#define  OS_STAT_MUTEX          0x10        /* Pending on mutual exclusion semaphore                   */
#define  OS_STAT_FLAG           0x20        /* Pending on event flag group                             */

/*
*********************************************************************************************************
*                                        OS_EVENT types
*********************************************************************************************************
*/
#define  OS_EVENT_TYPE_UNUSED      0
#define  OS_EVENT_TYPE_MBOX        1
#define  OS_EVENT_TYPE_Q           2
#define  OS_EVENT_TYPE_SEM         3
#define  OS_EVENT_TYPE_MUTEX       4
#define  OS_EVENT_TYPE_FLAG        5

/*
*********************************************************************************************************
*                                         EVENT FLAGS
*********************************************************************************************************
*/
#define  OS_FLAG_WAIT_CLR_ALL      0        /* Wait for ALL    the bits specified to be CLR (i.e. 0)   */
#define  OS_FLAG_WAIT_CLR_AND      0

#define  OS_FLAG_WAIT_CLR_ANY      1        /* Wait for ANY of the bits specified to be CLR (i.e. 0)   */
#define  OS_FLAG_WAIT_CLR_OR       1

#define  OS_FLAG_WAIT_SET_ALL      2        /* Wait for ALL    the bits specified to be SET (i.e. 1)   */
#define  OS_FLAG_WAIT_SET_AND      2

#define  OS_FLAG_WAIT_SET_ANY      3        /* Wait for ANY of the bits specified to be SET (i.e. 1)   */
#define  OS_FLAG_WAIT_SET_OR       3


#define  OS_FLAG_CONSUME        0x80        /* Consume the flags if condition(s) satisfied             */


#define  OS_FLAG_CLR               0
#define  OS_FLAG_SET               1

/*
*********************************************************************************************************
*                                   Values for OSTickStepState
*
* Note(s): This feature is used by uC/OS-View.
*********************************************************************************************************
*/

#if OS_TICK_STEP_EN > 0
#define  OS_TICK_STEP_DIS          0        /* Stepping is disabled, tick runs as mormal               */
#define  OS_TICK_STEP_WAIT         1        /* Waiting for uC/OS-View to set OSTickStepState to _ONCE  */
#define  OS_TICK_STEP_ONCE         2        /* Process tick once and wait for next cmd from uC/OS-View */
#endif

/*
*********************************************************************************************************
*       Possible values for 'opt' argument of OSSemDel(), OSMboxDel(), OSQDel() and OSMutexDel()
*********************************************************************************************************
*/
#define  OS_DEL_NO_PEND            0
#define  OS_DEL_ALWAYS             1

/*
*********************************************************************************************************
*                                     OS???PostOpt() OPTIONS
*
* These #defines are used to establish the options for OSMboxPostOpt() and OSQPostOpt().
*********************************************************************************************************
*/
#define  OS_POST_OPT_NONE       0x00        /* Post to highest priority task waiting                   */
#define  OS_POST_OPT_BROADCAST  0x01        /* Broadcast message to ALL tasks waiting                  */
#define  OS_POST_OPT_FRONT      0x02        /* Post to highest priority task waiting                   */

/*
*********************************************************************************************************
*                                 TASK OPTIONS (see OSTaskCreateExt())
*********************************************************************************************************
*/
#define  OS_TASK_OPT_STK_CHK  0x0001        /* Enable stack checking for the task                      */
#define  OS_TASK_OPT_STK_CLR  0x0002        /* Clear the stack when the task is create                 */
#define  OS_TASK_OPT_SAVE_FP  0x0004        /* Save the contents of any floating-point registers       */

/*
*********************************************************************************************************
*                                             ERROR CODES
*********************************************************************************************************
*/
#define OS_NO_ERR                     0

#define OS_ERR_EVENT_TYPE             1u
#define OS_ERR_PEND_ISR               2u
#define OS_ERR_POST_NULL_PTR          3u
#define OS_ERR_PEVENT_NULL            4u
#define OS_ERR_POST_ISR               5u
#define OS_ERR_QUERY_ISR              6u
#define OS_ERR_INVALID_OPT            7u
#define OS_ERR_TASK_WAITING           8u

#define OS_TIMEOUT                   10u
#define OS_TASK_NOT_EXIST            11u
#define OS_ERR_EVENT_NAME_TOO_LONG   12u
#define OS_ERR_FLAG_NAME_TOO_LONG    13u
#define OS_ERR_TASK_NAME_TOO_LONG    14u
#define OS_ERR_PNAME_NULL            15u

#define OS_MBOX_FULL                 20u

#define OS_Q_FULL                    30u
#define OS_Q_EMPTY                   31u

#define OS_PRIO_EXIST                40u
#define OS_PRIO_ERR                  41u
#define OS_PRIO_INVALID              42u

#define OS_SEM_OVF                   50u

#define OS_TASK_DEL_ERR              60u
#define OS_TASK_DEL_IDLE             61u
#define OS_TASK_DEL_REQ              62u
#define OS_TASK_DEL_ISR              63u

#define OS_NO_MORE_TCB               70u

#define OS_TIME_NOT_DLY              80u
#define OS_TIME_INVALID_MINUTES      81u
#define OS_TIME_INVALID_SECONDS      82u
#define OS_TIME_INVALID_MILLI        83u
#define OS_TIME_ZERO_DLY             84u

#define OS_TASK_SUSPEND_PRIO         90u
#define OS_TASK_SUSPEND_IDLE         91u

#define OS_TASK_RESUME_PRIO         100u
#define OS_TASK_NOT_SUSPENDED       101u

#define OS_MEM_INVALID_PART         110u
#define OS_MEM_INVALID_BLKS         111u
#define OS_MEM_INVALID_SIZE         112u
#define OS_MEM_NO_FREE_BLKS         113u
#define OS_MEM_FULL                 114u
#define OS_MEM_INVALID_PBLK         115u
#define OS_MEM_INVALID_PMEM         116u
#define OS_MEM_INVALID_PDATA        117u
#define OS_MEM_INVALID_ADDR         118u
#define OS_MEM_NAME_TOO_LONG        119u

#define OS_ERR_NOT_MUTEX_OWNER      120u

#define OS_TASK_OPT_ERR             130u

#define OS_ERR_DEL_ISR              140u
#define OS_ERR_CREATE_ISR           141u

#define OS_FLAG_INVALID_PGRP        150u
#define OS_FLAG_ERR_WAIT_TYPE       151u
#define OS_FLAG_ERR_NOT_RDY         152u
#define OS_FLAG_INVALID_OPT         153u
#define OS_FLAG_GRP_DEPLETED        154u


/*
*********************************************************************************************************
*                                          EVENT CONTROL BLOCK
*********************************************************************************************************
*/

#if (OS_EVENT_EN > 0) && (OS_MAX_EVENTS > 0)
typedef struct os_event {
    INT8U    OSEventType;                    /* Type of event control block (see OS_EVENT_TYPE_???)     */
    INT8U    OSEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
    INT16U   OSEventCnt;                     /* Semaphore Count (not used if other EVENT type)          */
    void    *OSEventPtr;                     /* Pointer to message or queue structure                   */
    INT8U    OSEventTbl[OS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
#if OS_EVENT_NAME_SIZE > 1
    char     OSEventName[OS_EVENT_NAME_SIZE];
#endif
} OS_EVENT;
#endif


/*
*********************************************************************************************************
*                                       EVENT FLAGS CONTROL BLOCK
*********************************************************************************************************
*/

#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)
typedef struct os_flag_grp {                /* Event Flag Group                                        */
    INT8U         OSFlagType;               /* Should be set to OS_EVENT_TYPE_FLAG                     */
    void         *OSFlagWaitList;           /* Pointer to first NODE of task waiting on event flag     */
    OS_FLAGS      OSFlagFlags;              /* 8, 16 or 32 bit flags                                   */
#if OS_FLAG_NAME_SIZE > 1
    char          OSFlagName[OS_FLAG_NAME_SIZE];
#endif
} OS_FLAG_GRP;



typedef struct os_flag_node {               /* Event Flag Wait List Node                               */
    void         *OSFlagNodeNext;           /* Pointer to next     NODE in wait list                   */
    void         *OSFlagNodePrev;           /* Pointer to previous NODE in wait list                   */
    void         *OSFlagNodeTCB;            /* Pointer to TCB of waiting task                          */
    void         *OSFlagNodeFlagGrp;        /* Pointer to Event Flag Group                             */
    OS_FLAGS      OSFlagNodeFlags;          /* Event flag to wait on                                   */
    INT8U         OSFlagNodeWaitType;       /* Type of wait:                                           */
                                            /*      OS_FLAG_WAIT_AND                                   */
                                            /*      OS_FLAG_WAIT_ALL                                   */
                                            /*      OS_FLAG_WAIT_OR                                    */
                                            /*      OS_FLAG_WAIT_ANY                                   */
} OS_FLAG_NODE;
#endif


/*
*********************************************************************************************************
*                                          MESSAGE MAILBOX DATA
*********************************************************************************************************
*/

#if OS_MBOX_EN > 0
typedef struct os_mbox_data {
    void   *OSMsg;                         /* Pointer to message in mailbox                            */
    INT8U   OSEventTbl[OS_EVENT_TBL_SIZE]; /* List of tasks waiting for event to occur                 */
    INT8U   OSEventGrp;                    /* Group corresponding to tasks waiting for event to occur  */
} OS_MBOX_DATA;
#endif

/*
*********************************************************************************************************
*                                     MEMORY PARTITION DATA STRUCTURES
*********************************************************************************************************
*/

#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)
typedef struct os_mem {                   /* MEMORY CONTROL BLOCK                                      */
    void   *OSMemAddr;                    /* Pointer to beginning of memory partition                  */
    void   *OSMemFreeList;                /* Pointer to list of free memory blocks                     */
    INT32U  OSMemBlkSize;                 /* Size (in bytes) of each block of memory                   */
    INT32U  OSMemNBlks;                   /* Total number of blocks in this partition                  */
    INT32U  OSMemNFree;                   /* Number of memory blocks remaining in this partition       */
#if OS_MEM_NAME_SIZE > 1
    char    OSMemName[OS_MEM_NAME_SIZE];  /* Memory partition name                                     */
#endif
} OS_MEM;


typedef struct os_mem_data {
    void   *OSAddr;                    /* Pointer to the beginning address of the memory partition     */
    void   *OSFreeList;                /* Pointer to the beginning of the free list of memory blocks   */
    INT32U  OSBlkSize;                 /* Size (in bytes) of each memory block                         */
    INT32U  OSNBlks;                   /* Total number of blocks in the partition                      */
    INT32U  OSNFree;                   /* Number of memory blocks free                                 */
    INT32U  OSNUsed;                   /* Number of memory blocks used                                 */
} OS_MEM_DATA;
#endif


/*
*********************************************************************************************************
*                                    MUTUAL EXCLUSION SEMAPHORE DATA
*********************************************************************************************************
*/

#if OS_MUTEX_EN > 0
typedef struct os_mutex_data {
    INT8U   OSEventTbl[OS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    INT8U   OSEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
    INT8U   OSValue;                        /* Mutex value (0 = used, 1 = available)                   */
    INT8U   OSOwnerPrio;                    /* Mutex owner's task priority or 0xFF if no owner         */
    INT8U   OSMutexPIP;                     /* Priority Inheritance Priority or 0xFF if no owner       */
} OS_MUTEX_DATA;
#endif

/*
*********************************************************************************************************
*                                          MESSAGE QUEUE DATA
*********************************************************************************************************
*/

#if OS_Q_EN > 0
typedef struct os_q {                   /* QUEUE CONTROL BLOCK                                         */
    struct os_q   *OSQPtr;              /* Link to next queue control block in list of free blocks     */
    void         **OSQStart;            /* Pointer to start of queue data                              */
    void         **OSQEnd;              /* Pointer to end   of queue data                              */
    void         **OSQIn;               /* Pointer to where next message will be inserted  in   the Q  */
    void         **OSQOut;              /* Pointer to where next message will be extracted from the Q  */
    INT16U         OSQSize;             /* Size of queue (maximum number of entries)                   */
    INT16U         OSQEntries;          /* Current number of entries in the queue                      */
} OS_Q;


typedef struct os_q_data {
    void          *OSMsg;               /* Pointer to next message to be extracted from queue          */
    INT16U         OSNMsgs;             /* Number of messages in message queue                         */
    INT16U         OSQSize;             /* Size of message queue                                       */
    INT8U          OSEventTbl[OS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur         */
    INT8U          OSEventGrp;          /* Group corresponding to tasks waiting for event to occur     */
} OS_Q_DATA;
#endif

/*
*********************************************************************************************************
*                                           SEMAPHORE DATA
*********************************************************************************************************
*/

#if OS_SEM_EN > 0
typedef struct os_sem_data {
    INT16U  OSCnt;                          /* Semaphore count                                         */
    INT8U   OSEventTbl[OS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    INT8U   OSEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
} OS_SEM_DATA;
#endif

/*
*********************************************************************************************************
*                                            TASK STACK DATA
*********************************************************************************************************
*/

#if OS_TASK_CREATE_EXT_EN > 0
typedef struct os_stk_data {
    INT32U  OSFree;                    /* Number of free bytes on the stack                            */
    INT32U  OSUsed;                    /* Number of bytes used on the stack                            */
} OS_STK_DATA;
#endif


/*
*********************************************************************************************************
*                                          TASK CONTROL BLOCK
*********************************************************************************************************
*/

typedef struct os_tcb {
    OS_STK          *OSTCBStkPtr;      /* Pointer to current top of stack                              */

#if OS_TASK_CREATE_EXT_EN > 0
    void            *OSTCBExtPtr;      /* Pointer to user definable data for TCB extension             */
    OS_STK          *OSTCBStkBottom;   /* Pointer to bottom of stack                                   */
    INT32U           OSTCBStkSize;     /* Size of task stack (in number of stack elements)             */
    INT16U           OSTCBOpt;         /* Task options as passed by OSTaskCreateExt()                  */
    INT16U           OSTCBId;          /* Task ID (0..65535)                                           */
#endif

    struct os_tcb   *OSTCBNext;        /* Pointer to next     TCB in the TCB list                      */
    struct os_tcb   *OSTCBPrev;        /* Pointer to previous TCB in the TCB list                      */

#if OS_EVENT_EN > 0
    OS_EVENT        *OSTCBEventPtr;    /* Pointer to event control block                               */
#endif

#if ((OS_Q_EN > 0) && (OS_MAX_QS > 0)) || (OS_MBOX_EN > 0)
    void            *OSTCBMsg;         /* Message received from OSMboxPost() or OSQPost()              */
#endif

#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)
#if OS_TASK_DEL_EN > 0
    OS_FLAG_NODE    *OSTCBFlagNode;    /* Pointer to event flag node                                   */
#endif
    OS_FLAGS         OSTCBFlagsRdy;    /* Event flags that made task ready to run                      */
#endif

    INT16U           OSTCBDly;         /* Nbr ticks to delay task or, timeout waiting for event        */
    INT8U            OSTCBStat;        /* Task status                                                  */
    INT8U            OSTCBPrio;        /* Task priority (0 == highest, 63 == lowest)                   */

    INT8U            OSTCBX;           /* Bit position in group  corresponding to task priority (0..7) */
    INT8U            OSTCBY;           /* Index into ready table corresponding to task priority        */
    INT8U            OSTCBBitX;        /* Bit mask to access bit position in ready table               */
    INT8U            OSTCBBitY;        /* Bit mask to access bit position in ready group               */

#if OS_TASK_DEL_EN > 0
    BOOLEAN          OSTCBDelReq;      /* Indicates whether a task needs to delete itself              */
#endif

#if OS_TASK_PROFILE_EN > 0
    INT32U           OSTCBCtxSwCtr;    /* Number of time the task was switched in                      */
    INT32U           OSTCBCyclesTot;   /* Total number of clock cycles the task has been running       */
    INT32U           OSTCBCyclesStart; /* Snapshot of cycle counter at start of task resumption        */
    OS_STK          *OSTCBStkBase;     /* Pointer to the beginning of the task stack                   */
    INT32U           OSTCBStkUsed;     /* Number of bytes used from the stack                          */
#endif

#if OS_TASK_NAME_SIZE > 1
    char             OSTCBTaskName[OS_TASK_NAME_SIZE];
#endif
} OS_TCB;


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

OS_EXT  INT32U            OSCtxSwCtr;               /* Counter of number of context switches           */

#if (OS_EVENT_EN > 0) && (OS_MAX_EVENTS > 0)
OS_EXT  OS_EVENT         *OSEventFreeList;          /* Pointer to list of free EVENT control blocks    */
OS_EXT  OS_EVENT          OSEventTbl[OS_MAX_EVENTS];/* Table of EVENT control blocks                   */
#endif

#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)
OS_EXT  OS_FLAG_GRP       OSFlagTbl[OS_MAX_FLAGS];  /* Table containing event flag groups              */
OS_EXT  OS_FLAG_GRP      *OSFlagFreeList;           /* Pointer to free list of event flag groups       */
#endif

#if OS_TASK_STAT_EN > 0
OS_EXT  INT8S             OSCPUUsage;               /* Percentage of CPU used                          */
OS_EXT  INT32U            OSIdleCtrMax;             /* Max. value that idle ctr can take in 1 sec.     */
OS_EXT  INT32U            OSIdleCtrRun;             /* Val. reached by idle ctr at run time in 1 sec.  */
OS_EXT  BOOLEAN           OSStatRdy;                /* Flag indicating that the statistic task is rdy  */
OS_EXT  OS_STK            OSTaskStatStk[OS_TASK_STAT_STK_SIZE];      /* Statistics task stack          */
#endif

OS_EXT  INT8U             OSIntNesting;             /* Interrupt nesting level                         */
OS_EXT  INT8U             OSIntExitY;

OS_EXT  INT8U             OSLockNesting;            /* Multitasking lock nesting level                 */

OS_EXT  INT8U             OSPrioCur;                /* Priority of current task                        */
OS_EXT  INT8U             OSPrioHighRdy;            /* Priority of highest priority task               */

OS_EXT  INT8U             OSRdyGrp;                        /* Ready list group                         */
OS_EXT  INT8U             OSRdyTbl[OS_RDY_TBL_SIZE];       /* Table of tasks which are ready to run    */

OS_EXT  BOOLEAN           OSRunning;                       /* Flag indicating that kernel is running   */

OS_EXT  INT8U             OSTaskCtr;                       /* Number of tasks created                  */

OS_EXT  volatile  INT32U  OSIdleCtr;                                 /* Idle counter                   */

OS_EXT  OS_STK            OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];      /* Idle task stack                */


OS_EXT  OS_TCB           *OSTCBCur;                        /* Pointer to currently running TCB         */
OS_EXT  OS_TCB           *OSTCBFreeList;                   /* Pointer to list of free TCBs             */
OS_EXT  OS_TCB           *OSTCBHighRdy;                    /* Pointer to highest priority TCB R-to-R   */
OS_EXT  OS_TCB           *OSTCBList;                       /* Pointer to doubly linked list of TCBs    */
OS_EXT  OS_TCB           *OSTCBPrioTbl[OS_LOWEST_PRIO + 1];/* Table of pointers to created TCBs        */
OS_EXT  OS_TCB            OSTCBTbl[OS_MAX_TASKS + OS_N_SYS_TASKS];   /* Table of TCBs                  */

#if OS_TICK_STEP_EN > 0
OS_EXT  INT8U             OSTickStepState;          /* Indicates the state of the tick step feature    */
#endif

#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)
OS_EXT  OS_MEM           *OSMemFreeList;            /* Pointer to free list of memory partitions       */
OS_EXT  OS_MEM            OSMemTbl[OS_MAX_MEM_PART];/* Storage for memory partition manager            */
#endif

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0)
OS_EXT  OS_Q             *OSQFreeList;              /* Pointer to list of free QUEUE control blocks    */
OS_EXT  OS_Q              OSQTbl[OS_MAX_QS];        /* Table of QUEUE control blocks                   */
#endif

#if OS_TIME_GET_SET_EN > 0
OS_EXT  volatile  INT32U  OSTime;                   /* Current value of system time (in ticks)         */
#endif


extern  INT8U   const     OSMapTbl[8];              /* Priority->Bit Mask lookup table                 */
extern  INT8U   const     OSUnMapTbl[256];          /* Priority->Index    lookup table                 */


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                     (Target Independent Functions)
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            MISCELLANEOUS
*********************************************************************************************************
*/

#if (OS_EVENT_EN > 0) && (OS_EVENT_NAME_SIZE > 1)
INT8U         OSEventNameGet(OS_EVENT *pevent, char *pname, INT8U *err);
void          OSEventNameSet(OS_EVENT *pevent, char *pname, INT8U *err);
#endif

/*
*********************************************************************************************************
*                                         EVENT FLAGS MANAGEMENT
*********************************************************************************************************
*/

#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)

#if OS_FLAG_ACCEPT_EN > 0
OS_FLAGS      OSFlagAccept(OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U wait_type, INT8U *err);
#endif

OS_FLAG_GRP  *OSFlagCreate(OS_FLAGS flags, INT8U *err);

#if OS_FLAG_DEL_EN > 0
OS_FLAG_GRP  *OSFlagDel(OS_FLAG_GRP *pgrp, INT8U opt, INT8U *err);
#endif

#if (OS_FLAG_EN > 0) && (OS_FLAG_NAME_SIZE > 1)
INT8U         OSFlagNameGet(OS_FLAG_GRP *pgrp, char *pname, INT8U *err);
void          OSFlagNameSet(OS_FLAG_GRP *pgrp, char *pname, INT8U *err);
#endif

OS_FLAGS      OSFlagPend(OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U wait_type, INT16U timeout, INT8U *err);
OS_FLAGS      OSFlagPendGetFlagsRdy(void);
OS_FLAGS      OSFlagPost(OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U operation, INT8U *err);

#if OS_FLAG_QUERY_EN > 0
OS_FLAGS      OSFlagQuery(OS_FLAG_GRP *pgrp, INT8U *err);
#endif
#endif

/*
*********************************************************************************************************
*                                        MESSAGE MAILBOX MANAGEMENT
*********************************************************************************************************
*/

#if OS_MBOX_EN > 0

#if OS_MBOX_ACCEPT_EN > 0
void         *OSMboxAccept(OS_EVENT *pevent);
#endif

OS_EVENT     *OSMboxCreate(void *msg);

#if OS_MBOX_DEL_EN > 0
OS_EVENT     *OSMboxDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
#endif

void         *OSMboxPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);

#if OS_MBOX_POST_EN > 0
INT8U         OSMboxPost(OS_EVENT *pevent, void *msg);
#endif

#if OS_MBOX_POST_OPT_EN > 0
INT8U         OSMboxPostOpt(OS_EVENT *pevent, void *msg, INT8U opt);
#endif

#if OS_MBOX_QUERY_EN > 0
INT8U         OSMboxQuery(OS_EVENT *pevent, OS_MBOX_DATA *p_mbox_data);
#endif
#endif

/*
*********************************************************************************************************
*                                           MEMORY MANAGEMENT
*********************************************************************************************************
*/

#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)

OS_MEM       *OSMemCreate(void *addr, INT32U nblks, INT32U blksize, INT8U *err);
void         *OSMemGet(OS_MEM *pmem, INT8U *err);
#if OS_MEM_NAME_SIZE > 1
INT8U         OSMemNameGet(OS_MEM *pmem, char *pname, INT8U *err);
void          OSMemNameSet(OS_MEM *pmem, char *pname, INT8U *err);
#endif
INT8U         OSMemPut(OS_MEM *pmem, void *pblk);

#if OS_MEM_QUERY_EN > 0
INT8U         OSMemQuery(OS_MEM *pmem, OS_MEM_DATA *p_mem_data);
#endif

#endif

/*
*********************************************************************************************************
*                                MUTUAL EXCLUSION SEMAPHORE MANAGEMENT
*********************************************************************************************************
*/

#if OS_MUTEX_EN > 0

#if OS_MUTEX_ACCEPT_EN > 0
INT8U         OSMutexAccept(OS_EVENT *pevent, INT8U *err);
#endif

OS_EVENT     *OSMutexCreate(INT8U prio, INT8U *err);

#if OS_MUTEX_DEL_EN > 0
OS_EVENT     *OSMutexDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
#endif

void          OSMutexPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);
INT8U         OSMutexPost(OS_EVENT *pevent);

#if OS_MUTEX_QUERY_EN > 0
INT8U         OSMutexQuery(OS_EVENT *pevent, OS_MUTEX_DATA *p_mutex_data);
#endif

#endif


/*
*********************************************************************************************************
*                                         MESSAGE QUEUE MANAGEMENT
*********************************************************************************************************
*/

#if (OS_Q_EN > 0) && (OS_MAX_QS > 0)

#if OS_Q_ACCEPT_EN > 0
void         *OSQAccept(OS_EVENT *pevent, INT8U *err);
#endif

OS_EVENT     *OSQCreate(void **start, INT16U size);

#if OS_Q_DEL_EN > 0
OS_EVENT     *OSQDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
#endif

#if OS_Q_FLUSH_EN > 0
INT8U         OSQFlush(OS_EVENT *pevent);
#endif

void         *OSQPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);

#if OS_Q_POST_EN > 0
INT8U         OSQPost(OS_EVENT *pevent, void *msg);
#endif

#if OS_Q_POST_FRONT_EN > 0
INT8U         OSQPostFront(OS_EVENT *pevent, void *msg);
#endif

#if OS_Q_POST_OPT_EN > 0
INT8U         OSQPostOpt(OS_EVENT *pevent, void *msg, INT8U opt);
#endif

#if OS_Q_QUERY_EN > 0
INT8U         OSQQuery(OS_EVENT *pevent, OS_Q_DATA *p_q_data);
#endif

#endif


/*
*********************************************************************************************************
*                                          SEMAPHORE MANAGEMENT
*********************************************************************************************************
*/
#if OS_SEM_EN > 0

#if OS_SEM_ACCEPT_EN > 0
INT16U        OSSemAccept(OS_EVENT *pevent);
#endif

OS_EVENT     *OSSemCreate(INT16U cnt);

#if OS_SEM_DEL_EN > 0
OS_EVENT     *OSSemDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
#endif

void          OSSemPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);
INT8U         OSSemPost(OS_EVENT *pevent);

#if OS_SEM_QUERY_EN > 0
INT8U         OSSemQuery(OS_EVENT *pevent, OS_SEM_DATA *p_sem_data);
#endif

#endif


/*
*********************************************************************************************************
*                                            TASK MANAGEMENT
*********************************************************************************************************
*/
#if OS_TASK_CHANGE_PRIO_EN > 0
INT8U         OSTaskChangePrio(INT8U oldprio, INT8U newprio);
#endif

#if OS_TASK_CREATE_EN > 0
INT8U         OSTaskCreate(void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT8U prio);
#endif

#if OS_TASK_CREATE_EXT_EN > 0
INT8U         OSTaskCreateExt(void  (*task)(void *pd),
                              void   *p_arg,
                              OS_STK *ptos,
                              INT8U   prio,
                              INT16U  id,
                              OS_STK *pbos,
                              INT32U  stk_size,
                              void   *pext,
                              INT16U  opt);
#endif

#if OS_TASK_DEL_EN > 0
INT8U         OSTaskDel(INT8U prio);
INT8U         OSTaskDelReq(INT8U prio);
#endif

#if OS_TASK_NAME_SIZE > 1
INT8U         OSTaskNameGet(INT8U prio, char *pname, INT8U *err);
void          OSTaskNameSet(INT8U prio, char *pname, INT8U *err);
#endif

#if OS_TASK_SUSPEND_EN > 0
INT8U         OSTaskResume(INT8U prio);
INT8U         OSTaskSuspend(INT8U prio);
#endif

#if OS_TASK_CREATE_EXT_EN > 0
INT8U         OSTaskStkChk(INT8U prio, OS_STK_DATA *p_stk_data);
#endif

#if OS_TASK_QUERY_EN > 0
INT8U         OSTaskQuery(INT8U prio, OS_TCB *p_task_data);
#endif


/*
*********************************************************************************************************
*                                            TIME MANAGEMENT
*********************************************************************************************************
*/

void          OSTimeDly(INT16U ticks);

#if OS_TIME_DLY_HMSM_EN > 0
INT8U         OSTimeDlyHMSM(INT8U hours, INT8U minutes, INT8U seconds, INT16U milli);
#endif

#if OS_TIME_DLY_RESUME_EN > 0
INT8U         OSTimeDlyResume(INT8U prio);
#endif

#if OS_TIME_GET_SET_EN > 0
INT32U        OSTimeGet(void);
void          OSTimeSet(INT32U ticks);
#endif

void          OSTimeTick(void);

/*
*********************************************************************************************************
*                                             MISCELLANEOUS
*********************************************************************************************************
*/

void          OSInit(void);

void          OSIntEnter(void);
void          OSIntExit(void);

#if OS_SCHED_LOCK_EN > 0
void          OSSchedLock(void);
void          OSSchedUnlock(void);
#endif

void          OSStart(void);

void          OSStatInit(void);

INT16U        OSVersion(void);


/*
*********************************************************************************************************
*                                      INTERNAL FUNCTION PROTOTYPES
*                            (Your application MUST NOT call these functions)
*********************************************************************************************************
*/

#if OS_TASK_DEL_EN > 0
void          OS_Dummy(void);
#endif

#if OS_EVENT_EN > 0
INT8U         OS_EventTaskRdy(OS_EVENT *pevent, void *msg, INT8U msk);
void          OS_EventTaskWait(OS_EVENT *pevent);
void          OS_EventTO(OS_EVENT *pevent);
void          OS_EventWaitListInit(OS_EVENT *pevent);
#endif

#if (OS_VERSION >= 251) && (OS_FLAG_EN > 0) && (OS_MAX_FLAGS > 0)
void          OS_FlagInit(void);
void          OS_FlagUnlink(OS_FLAG_NODE *pnode);
#endif

void          OS_MemClr(INT8U *pdest, INT16U size);
void          OS_MemCopy(INT8U *pdest, INT8U *psrc, INT16U size);

#if (OS_MEM_EN > 0) && (OS_MAX_MEM_PART > 0)
void          OS_MemInit(void);
#endif

#if OS_Q_EN > 0
void          OS_QInit(void);
#endif

void          OS_Sched(void);

#if (OS_EVENT_NAME_SIZE > 1) || (OS_FLAG_NAME_SIZE > 1) || (OS_MEM_NAME_SIZE > 1) || (OS_TASK_NAME_SIZE > 1)
INT8U         OS_StrCopy(char *pdest, char *psrc);
INT8U         OS_StrLen(char *psrc);
#endif

void          OS_TaskIdle(void *p_arg);

#if OS_TASK_STAT_EN > 0
void          OS_TaskStat(void *p_arg);
#endif

#if OS_TASK_CREATE_EXT_EN > 0
void          OS_TaskStkClr(OS_STK *ptos, INT32U size, INT16U opt);
#endif

#if (OS_TASK_STAT_STK_CHK_EN > 0) && (OS_TASK_CREATE_EXT_EN > 0)
void          OS_TaskStatStkChk(void);
#endif

INT8U         OS_TCBInit(INT8U prio, OS_STK *ptos, OS_STK *pbos, INT16U id, INT32U stk_size, void *pext, INT16U opt);


/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                      (Target Specific Functions)
*********************************************************************************************************
*/

#if OS_VERSION >= 270 && OS_DEBUG_EN > 0
void          OSDebugInit(void);
#endif

#if OS_VERSION >= 204
void          OSInitHookBegin(void);
void          OSInitHookEnd(void);
#endif

void          OSIntCtxSw(void);

void          OSStartHighRdy(void);

void          OSTaskCreateHook(OS_TCB *ptcb);
void          OSTaskDelHook(OS_TCB *ptcb);

#if OS_VERSION >= 251
void          OSTaskIdleHook(void);
#endif

void          OSTaskStatHook(void);
OS_STK       *OSTaskStkInit(void (*task)(void *pd), void *p_arg, OS_STK *ptos, INT16U opt);

#if OS_TASK_SW_HOOK_EN > 0
void          OSTaskSwHook(void);
#endif

#if OS_VERSION >= 204
void          OSTCBInitHook(OS_TCB *ptcb);
#endif

#if OS_TIME_TICK_HOOK_EN > 0
void          OSTimeTickHook(void);
#endif

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                  (Compiler Specific ISR prototypes)
*********************************************************************************************************
*/

#ifndef OS_ISR_PROTO_EXT
void          OSCtxSw(void);
void          OSTickISR(void);
#endif


/*
*********************************************************************************************************
*                                   LOOK FOR MISSING #define CONSTANTS
*
* This section is used to generate ERROR messages at compile time if certain #define constants are
* MISSING in OS_CFG.H.  This allows you to quickly determine the source of the error.
*
* You SHOULD NOT change this section UNLESS you would like to add more comments as to the source of the
* compile time error.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            EVENT FLAGS
*********************************************************************************************************
*/

#ifndef OS_FLAG_EN
#error  "OS_CFG.H, Missing OS_FLAG_EN: Enable (1) or Disable (0) code generation for Event Flags"
#else
    #ifndef OS_MAX_FLAGS
    #error  "OS_CFG.H, Missing OS_MAX_FLAGS: Max. number of Event Flag Groups in your application"
    #else
        #if     OS_MAX_FLAGS > 255
        #error  "OS_CFG.H, OS_MAX_FLAGS must be <= 255"
        #endif
    #endif

    #ifndef OS_FLAG_WAIT_CLR_EN
    #error  "OS_CFG.H, Missing OS_FLAG_WAIT_CLR_EN: Include code for Wait on Clear EVENT FLAGS"
    #endif

    #ifndef OS_FLAG_ACCEPT_EN
    #error  "OS_CFG.H, Missing OS_FLAG_ACCEPT_EN: Include code for OSFlagAccept()"
    #endif

    #ifndef OS_FLAG_DEL_EN
    #error  "OS_CFG.H, Missing OS_FLAG_DEL_EN: Include code for OSFlagDel()"
    #endif

    #ifndef OS_FLAG_NAME_SIZE
    #error  "OS_CFG.H, Missing OS_FLAG_NAME_SIZE: Determines the size of flag group names"
    #endif

    #ifndef OS_FLAG_QUERY_EN
    #error  "OS_CFG.H, Missing OS_FLAG_QUERY_EN: Include code for OSFlagQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                           MESSAGE MAILBOXES
*********************************************************************************************************
*/

#ifndef OS_MBOX_EN
#error  "OS_CFG.H, Missing OS_MBOX_EN: Enable (1) or Disable (0) code generation for MAILBOXES"
#else
    #ifndef OS_MBOX_ACCEPT_EN
    #error  "OS_CFG.H, Missing OS_MBOX_ACCEPT_EN: Include code for OSMboxAccept()"
    #endif

    #ifndef OS_MBOX_DEL_EN
    #error  "OS_CFG.H, Missing OS_MBOX_DEL_EN: Include code for OSMboxDel()"
    #endif

    #ifndef OS_MBOX_POST_EN
    #error  "OS_CFG.H, Missing OS_MBOX_POST_EN: Include code for OSMboxPost()"
    #endif

    #ifndef OS_MBOX_POST_OPT_EN
    #error  "OS_CFG.H, Missing OS_MBOX_POST_OPT_EN: Include code for OSMboxPostOpt()"
    #endif

    #ifndef OS_MBOX_QUERY_EN
    #error  "OS_CFG.H, Missing OS_MBOX_QUERY_EN: Include code for OSMboxQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                           MEMORY MANAGEMENT
*********************************************************************************************************
*/

#ifndef OS_MEM_EN
#error  "OS_CFG.H, Missing OS_MEM_EN: Enable (1) or Disable (0) code generation for MEMORY MANAGER"
#else
    #ifndef OS_MAX_MEM_PART
    #error  "OS_CFG.H, Missing OS_MAX_MEM_PART: Max. number of memory partitions"
    #else
        #if     OS_MAX_MEM_PART > 255
        #error  "OS_CFG.H, OS_MAX_MEM_PART must be <= 255"
        #endif
    #endif

    #ifndef OS_MEM_NAME_SIZE
    #error  "OS_CFG.H, Missing OS_MEM_NAME_SIZE: Determines the size of memory partition names"
    #endif

    #ifndef OS_MEM_QUERY_EN
    #error  "OS_CFG.H, Missing OS_MEM_QUERY_EN: Include code for OSMemQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                       MUTUAL EXCLUSION SEMAPHORES
*********************************************************************************************************
*/

#ifndef OS_MUTEX_EN
#error  "OS_CFG.H, Missing OS_MUTEX_EN: Enable (1) or Disable (0) code generation for MUTEX"
#else
    #ifndef OS_MUTEX_ACCEPT_EN
    #error  "OS_CFG.H, Missing OS_MUTEX_ACCEPT_EN: Include code for OSMutexAccept()"
    #endif

    #ifndef OS_MUTEX_DEL_EN
    #error  "OS_CFG.H, Missing OS_MUTEX_DEL_EN: Include code for OSMutexDel()"
    #endif

    #ifndef OS_MUTEX_QUERY_EN
    #error  "OS_CFG.H, Missing OS_MUTEX_QUERY_EN: Include code for OSMutexQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                              MESSAGE QUEUES
*********************************************************************************************************
*/

#ifndef OS_Q_EN
#error  "OS_CFG.H, Missing OS_Q_EN: Enable (1) or Disable (0) code generation for QUEUES"
#else
    #ifndef OS_MAX_QS
    #error  "OS_CFG.H, Missing OS_MAX_QS: Max. number of queue control blocks"
    #else
        #if     OS_MAX_QS > 255
        #error  "OS_CFG.H, OS_MAX_QS must be <= 255"
        #endif
    #endif

    #ifndef OS_Q_ACCEPT_EN
    #error  "OS_CFG.H, Missing OS_Q_ACCEPT_EN: Include code for OSQAccept()"
    #endif

    #ifndef OS_Q_DEL_EN
    #error  "OS_CFG.H, Missing OS_Q_DEL_EN: Include code for OSQDel()"
    #endif

    #ifndef OS_Q_FLUSH_EN
    #error  "OS_CFG.H, Missing OS_Q_FLUSH_EN: Include code for OSQFlush()"
    #endif

    #ifndef OS_Q_POST_EN
    #error  "OS_CFG.H, Missing OS_Q_POST_EN: Include code for OSQPost()"
    #endif

    #ifndef OS_Q_POST_FRONT_EN
    #error  "OS_CFG.H, Missing OS_Q_POST_FRONT_EN: Include code for OSQPostFront()"
    #endif

    #ifndef OS_Q_POST_OPT_EN
    #error  "OS_CFG.H, Missing OS_Q_POST_OPT_EN: Include code for OSQPostOpt()"
    #endif

    #ifndef OS_Q_QUERY_EN
    #error  "OS_CFG.H, Missing OS_Q_QUERY_EN: Include code for OSQQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                              SEMAPHORES
*********************************************************************************************************
*/

#ifndef OS_SEM_EN
#error  "OS_CFG.H, Missing OS_SEM_EN: Enable (1) or Disable (0) code generation for SEMAPHORES"
#else
    #ifndef OS_SEM_ACCEPT_EN
    #error  "OS_CFG.H, Missing OS_SEM_ACCEPT_EN: Include code for OSSemAccept()"
    #endif

    #ifndef OS_SEM_DEL_EN
    #error  "OS_CFG.H, Missing OS_SEM_DEL_EN: Include code for OSSemDel()"
    #endif

    #ifndef OS_SEM_QUERY_EN
    #error  "OS_CFG.H, Missing OS_SEM_QUERY_EN: Include code for OSSemQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                             TASK MANAGEMENT
*********************************************************************************************************
*/

#ifndef OS_MAX_TASKS
#error  "OS_CFG.H, Missing OS_MAX_TASKS: Max. number of tasks in your application"
#else
    #if     OS_MAX_TASKS < 2
    #error  "OS_CFG.H,         OS_MAX_TASKS must be >= 2"
    #endif
    #if     OS_MAX_TASKS > 63
    #error  "OS_CFG.H,         OS_MAX_TASKS must be <= 63"
    #endif
#endif

#ifndef OS_TASK_IDLE_STK_SIZE
#error  "OS_CFG.H, Missing OS_TASK_IDLE_STK_SIZE: Idle task stack size"
#endif

#ifndef OS_TASK_STAT_EN
#error  "OS_CFG.H, Missing OS_TASK_STAT_EN: Enable (1) or Disable(0) the statistics task"
#endif

#ifndef OS_TASK_STAT_STK_SIZE
#error  "OS_CFG.H, Missing OS_TASK_STAT_STK_SIZE: Statistics task stack size"
#endif

#ifndef OS_TASK_STAT_STK_CHK_EN
#error  "OS_CFG.H, Missing OS_TASK_STAT_STK_CHK_EN: Check task stacks from statistics task"
#endif

#ifndef OS_TASK_CHANGE_PRIO_EN
#error  "OS_CFG.H, Missing OS_TASK_CHANGE_PRIO_EN: Include code for OSTaskChangePrio()"
#endif

#ifndef OS_TASK_CREATE_EN
#error  "OS_CFG.H, Missing OS_TASK_CREATE_EN: Include code for OSTaskCreate()"
#endif

#ifndef OS_TASK_CREATE_EXT_EN
#error  "OS_CFG.H, Missing OS_TASK_CREATE_EXT_EN: Include code for OSTaskCreateExt()"
#endif

#ifndef OS_TASK_DEL_EN
#error  "OS_CFG.H, Missing OS_TASK_DEL_EN: Include code for OSTaskDel()"
#endif

#ifndef OS_TASK_NAME_SIZE
#error  "OS_CFG.H, Missing OS_TASK_NAME_SIZE: Determine the size of task names"
#endif

#ifndef OS_TASK_SUSPEND_EN
#error  "OS_CFG.H, Missing OS_TASK_SUSPEND_EN: Include code for OSTaskSuspend() and OSTaskResume()"
#endif

#ifndef OS_TASK_QUERY_EN
#error  "OS_CFG.H, Missing OS_TASK_QUERY_EN: Include code for OSTaskQuery()"
#endif

/*
*********************************************************************************************************
*                                             TIME MANAGEMENT
*********************************************************************************************************
*/

#ifndef OS_TICKS_PER_SEC
#error  "OS_CFG.H, Missing OS_TICKS_PER_SEC: Sets the number of ticks in one second"
#endif

#ifndef OS_TIME_DLY_HMSM_EN
#error  "OS_CFG.H, Missing OS_TIME_DLY_HMSM_EN: Include code for OSTimeDlyHMSM()"
#endif

#ifndef OS_TIME_DLY_RESUME_EN
#error  "OS_CFG.H, Missing OS_TIME_DLY_RESUME_EN: Include code for OSTimeDlyResume()"
#endif

#ifndef OS_TIME_GET_SET_EN
#error  "OS_CFG.H, Missing OS_TIME_GET_SET_EN: Include code for OSTimeGet() and OSTimeSet()"
#endif

/*
*********************************************************************************************************
*                                            MISCELLANEOUS
*********************************************************************************************************
*/

#ifndef OS_ARG_CHK_EN
#error  "OS_CFG.H, Missing OS_ARG_CHK_EN: Enable (1) or Disable (0) argument checking"
#endif

#ifndef OS_CPU_HOOKS_EN
#error  "OS_CFG.H, Missing OS_CPU_HOOKS_EN: uC/OS-II hooks are found in the processor port files when 1"
#endif

#ifndef OS_LOWEST_PRIO
#error  "OS_CFG.H, Missing OS_LOWEST_PRIO: Defines the lowest priority that can be assigned"
#endif

#ifndef OS_MAX_EVENTS
#error  "OS_CFG.H, Missing OS_MAX_EVENTS: Max. number of event control blocks in your application"
#else
    #if     OS_MAX_EVENTS > 255
    #error  "OS_CFG.H, OS_MAX_EVENTS must be <= 255"
    #endif
#endif

#ifndef OS_SCHED_LOCK_EN
#error  "OS_CFG.H, Missing OS_SCHED_LOCK_EN: Include code for OSSchedLock() and OSSchedUnlock()"
#endif

#ifndef OS_TASK_PROFILE_EN
#error  "OS_CFG.H, Missing OS_TASK_PROFILE_EN: Include data structure for run-time task profiling"
#endif

#ifndef OS_TASK_SW_HOOK_EN
#error  "OS_CFG.H, Missing OS_TASK_SW_HOOK_EN: Allows you to include the code for OSTaskSwHook() or not"
#endif

#ifndef OS_TICK_STEP_EN
#error  "OS_CFG.H, Missing OS_TICK_STEP_EN: Allows to 'step' one tick at a time with uC/OS-View"
#endif

#ifndef OS_TIME_TICK_HOOK_EN
#error  "OS_CFG.H, Missing OS_TIME_TICK_HOOK_EN: Allows you to include the code for OSTimeTickHook() or not"
#endif

#ifndef OS_DEBUG_EN
#error  "OS_CFG.H, Missing OS_DEBUG_EN: Allows you to include variables for debugging or not"
#endif

#ifdef __cplusplus
}
#endif
#endif
