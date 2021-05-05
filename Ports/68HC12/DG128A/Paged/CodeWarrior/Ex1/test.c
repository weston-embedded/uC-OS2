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
*                                       Paged M68HC12 Sample code
*                                                  IAR
*
* Filename : test.c
* Version  : V2.93.01
*********************************************************************************************************
*/

#include <includes.h>

/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/

#define  APP_TASK_STK_SIZE   256

/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/

OS_STK  AppStartTaskStk[APP_TASK_STK_SIZE];
OS_STK  AppTask1Stk[APP_TASK_STK_SIZE];
OS_STK  AppTask2Stk[APP_TASK_STK_SIZE];

INT16U  AppTask1Ctr;
INT16U  AppTask2Ctr;

/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void  AppStartTask(void *pdata);
static void  AppTask1(void *pdata);
static void  AppTask2(void *pdata);
static void  AppTickInit(void);

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary 68HC12 and C initialization.
* Arguments   : none
*********************************************************************************************************
*/

void main (void)
{
    /*---- Any initialization code prior to calling OSInit() goes HERE --------------------------------*/

    OSInit();                               /* Initialize "uC/OS-II, The Real-Time Kernel"             */

    /*---- Any initialization code before starting multitasking ---------------------------------------*/

    OSTaskCreate(AppStartTask, (void *)0, (void *)&AppStartTaskStk[APP_TASK_STK_SIZE - 1], 0);

    /*---- Create any other task you want before we start multitasking --------------------------------*/

    OSStart();                              /* Start multitasking (i.e. give control to uC/OS-II)      */
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
* Arguments   : pdata   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'pdata' is not
*                  used.  The compiler should not generate any code for this statement.
*               2) Interrupts are enabled once the task start because the I-bit of the CCR register was
*                  set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/

static void  AppStartTask (void *pdata)
{
    pdata = pdata;
    AppTickInit();                          /* Initialize the ticker                                   */
    /*---- Task initialization code goes HERE! --------------------------------------------------------*/
    OSTaskCreate(AppTask1, (void *)0, (void *)&AppTask1Stk[APP_TASK_STK_SIZE - 1], 10);
    OSTaskCreate(AppTask2, (void *)0, (void *)&AppTask2Stk[APP_TASK_STK_SIZE - 1], 20);

    while (TRUE) {                          /* Task body, always written as an infinite loop.          */
        /*---- Task code goes HERE! -------------------------------------------------------------------*/
        OSTimeDly(1);                       /* Delay task execution for one clock tick                 */
    }
}

/*
*********************************************************************************************************
*                                             TASK #1
*
* Description : This is an example of a task.
* Arguments   : pdata   is the argument passed to 'AppTask1()' by 'OSTaskCreate()'.
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'pdata' is not
*                  used.  The compiler should not generate any code for this statement.
*               2) Interrupts are enabled once the task start because the I-bit of the CCR register was
*                  set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/

static void  AppTask1 (void *pdata)
{
    pdata = pdata;
    /*---- Task initialization code goes HERE! --------------------------------------------------------*/

    while (TRUE) {                          /* Task body, always written as an infinite loop.          */
        /*---- Task code goes HERE! -------------------------------------------------------------------*/
        AppTask1Ctr++;
        OSTimeDly(1);                       /* Delay task execution for one clock tick                 */
    }
}

/*
*********************************************************************************************************
*                                             TASK #2
*
* Description : This is an example of a task.
* Arguments   : pdata   is the argument passed to 'AppTask2()' by 'OSTaskCreate()'.
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'pdata' is not
*                  used.  The compiler should not generate any code for this statement.
*               2) Interrupts are enabled once the task start because the I-bit of the CCR register was
*                  set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/

static void  AppTask2 (void *pdata)
{
    pdata = pdata;
    /*---- Task initialization code goes HERE! --------------------------------------------------------*/

    while (TRUE) {                          /* Task body, always written as an infinite loop.          */
        /*---- Task code goes HERE! -------------------------------------------------------------------*/
        AppTask2Ctr++;
        OSTimeDly(1);                       /* Delay task execution for one clock tick                 */
    }
}

/*
*********************************************************************************************************
*                                      TICKER INITIALIZATION
*
* Description : This function is used to initialize one of the eight output compares to generate an
*               interrupt at the desired tick rate.  You must decide which output compare you will be
*               using by setting the configuration variable OS_TICK_OC (see OS_CFG.H and also OS_CPU_A.S)
*               to 0..7 depending on which output compare to use.
*                   OS_TICK_OC set to 0 chooses output compare #0 as the ticker source
*                   OS_TICK_OC set to 1 chooses output compare #1 as the ticker source
*                   OS_TICK_OC set to 2 chooses output compare #2 as the ticker source
*                   OS_TICK_OC set to 3 chooses output compare #3 as the ticker source
*                   OS_TICK_OC set to 4 chooses output compare #4 as the ticker source
*                   OS_TICK_OC set to 5 chooses output compare #5 as the ticker source
*                   OS_TICK_OC set to 6 chooses output compare #6 as the ticker source
*                   OS_TICK_OC set to 7 chooses output compare #7 as the ticker source
* Arguments   : none
* Notes       : 1) It is assumed that you have set the prescaler rate of the free running timer within
*                  the first 64 E clock cycles of the 68HC12.
*               2) CPU registers are define in IO.H (see COSMIC compiler) and in OS_CPU_A.S.
*********************************************************************************************************
*/

static void AppTickInit (void)
{
    TSCR   = 0x80;                          /* Enable timer                                            */

#if OS_TICK_OC == 0
    TIOS  |= 0x01;                          /* Make channel an output compare                          */
    TC0    = TCNT + OS_TICK_OC_CNTS;        /* Set TC0 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x01;                          /* Enable OC0 interrupt.                                   */
#endif

#if OS_TICK_OC == 1
    TIOS  |= 0x02;                          /* Make channel an output compare                          */
    TC1    = TCNT + OS_TICK_OC_CNTS;        /* Set TC1 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x02;                          /* Enable OC1 interrupt.                                   */
#endif

#if OS_TICK_OC == 2
    TIOS  |= 0x04;                          /* Make channel an output compare                          */
    TC2    = TCNT + OS_TICK_OC_CNTS;        /* Set TC2 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x04;                          /* Enable OC2 interrupt.                                   */
#endif

#if OS_TICK_OC == 3
    TIOS  |= 0x08;                          /* Make channel an output compare                          */
    TC3    = TCNT + OS_TICK_OC_CNTS;        /* Set TC3 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x08;                          /* Enable OC3 interrupt.                                   */
#endif

#if OS_TICK_OC == 4
    TIOS  |= 0x10;                          /* Make channel an output compare                          */
    TC4    = TCNT + OS_TICK_OC_CNTS;        /* Set TC4 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x10;                          /* Enable OC4 interrupt.                                   */
#endif

#if OS_TICK_OC == 5
    TIOS  |= 0x20;                          /* Make channel an output compare                          */
    TC5    = TCNT + OS_TICK_OC_CNTS;        /* Set TC5 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x20;                          /* Enable OC5 interrupt.                                   */
#endif

#if OS_TICK_OC == 6
    TIOS  |= 0x40;                          /* Make channel an output compare                          */
    TC6    = TCNT + OS_TICK_OC_CNTS;        /* Set TC6 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x40;                          /* Enable OC6 interrupt.                                   */
#endif

#if OS_TICK_OC == 7
    TIOS  |= 0x80;                          /* Make channel an output compare                          */
    TC7    = TCNT + OS_TICK_OC_CNTS;        /* Set TC7 to present time + OS_TICK_OC_CNTS               */
    TMSK1 |= 0x80;                          /* Enable OC7 interrupt.                                   */
#endif
}
