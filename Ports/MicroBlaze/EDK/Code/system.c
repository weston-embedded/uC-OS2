/******************************************************************************
* Function name         :  system.c
* returns               :  
* Created by            :  Nasser Poureh 
* Date Created          :  6/17/03
* Company				:  Insight Electronics
* Description           :  Main Program
* Notes                 :  
******************************************************************************/

#include "xuartlite_l.h"
#include "xtmrctr_l.h"
#include "xgpio_l.h"
#include "xparameters.h"
#include <xintc_l.h>

	

unsigned int count_1 = 0;
unsigned int count_2 = 0;
unsigned int timer_count = 1;
static char display_data[10] =	{0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6};





void timer1_int_handler(void * baseaddr_p) {
   int baseaddr = *(int *)baseaddr_p;
   unsigned int csr;

  /* Read timer 0 CSR to see if it raised the interrupt */
  csr = XTmrCtr_mGetControlStatusReg(XPAR_MYTIMER1_BASEADDR, 0);
  
  if (csr & XTC_CSR_INT_OCCURED_MASK) {
    
    XGpio_mSetDataReg(XPAR_MYGPIO_A_BASEADDR, display_data[count_1]);
    
    
    /* Clear the timer interrupt */
    XTmrCtr_mSetControlStatusReg(XPAR_MYTIMER1_BASEADDR, 0, csr);

  }

  count_1 = count_1 + 1;

   if (count_1 == 10) {
	   count_1 = 0;
   }

}





void timer2_int_handler(void * baseaddr_p) {
   int baseaddr = *(int *)baseaddr_p;
   unsigned int csr;

  /* Read timer 0 CSR to see if it raised the interrupt */
  csr = XTmrCtr_mGetControlStatusReg(XPAR_MYTIMER2_BASEADDR, 0);
  
  if (csr & XTC_CSR_INT_OCCURED_MASK) {
    
    XGpio_mSetDataReg(XPAR_MYGPIO_B_BASEADDR, display_data[count_2]);
    
    
    /* Clear the timer interrupt */
    XTmrCtr_mSetControlStatusReg(XPAR_MYTIMER2_BASEADDR, 0, csr);

  } 
   count_2 = count_2 + 1;

   if (count_2 == 10) {
	   count_2 = 0;
   }

}







main() {


	print("        ####################################################\n\r");
	print("        #                                                  #\n\r");
	print("        #              Memec Design MB1000                 #\n\r");
	print("        #          MicroBlaze Development Board            #\n\r");
	print("        #                                                  #\n\r");
	print("        ####################################################\n\r");
	print("\n\r");
	print("\n\r");



  /* Enable microblaze interrupts */
  microblaze_enable_interrupts();

  /* Start the interrupt controller */
  XIntc_mMasterEnable(XPAR_MYINTC_BASEADDR);

  /* Set the direction of the GPIO ports to outputs to drive the 7-seg LEDs*/
  XGpio_mSetDataDirection(XPAR_MYGPIO_A_BASEADDR, 0x00);
  XGpio_mSetDataDirection(XPAR_MYGPIO_B_BASEADDR, 0x00);

  /* set the number of cycles each timer counts before generating an interrupt */
  XTmrCtr_mSetLoadReg(XPAR_MYTIMER1_BASEADDR, 0,  50000000);
  XTmrCtr_mSetLoadReg(XPAR_MYTIMER2_BASEADDR, 0, 100000000);

  /* reset the timers, and clear interrupts */
  XTmrCtr_mSetControlStatusReg(XPAR_MYTIMER1_BASEADDR, 0, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK );
  XTmrCtr_mSetControlStatusReg(XPAR_MYTIMER2_BASEADDR, 0, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK );
	
  /* Enable timer1 and timer2 interrupts in the interrupt controller */
  XIntc_mEnableIntr(XPAR_MYINTC_BASEADDR, XPAR_MYTIMER1_INTERRUPT_MASK | XPAR_MYTIMER2_INTERRUPT_MASK);
  
  /* start the timers */
  XTmrCtr_mSetControlStatusReg(XPAR_MYTIMER1_BASEADDR, 0, XTC_CSR_ENABLE_TMR_MASK | XTC_CSR_ENABLE_INT_MASK | XTC_CSR_AUTO_RELOAD_MASK | XTC_CSR_DOWN_COUNT_MASK);
  XTmrCtr_mSetControlStatusReg(XPAR_MYTIMER2_BASEADDR, 0, XTC_CSR_ENABLE_TMR_MASK | XTC_CSR_ENABLE_INT_MASK | XTC_CSR_AUTO_RELOAD_MASK | XTC_CSR_DOWN_COUNT_MASK);
  
  /* Wait for interrupts to occur */
  while (1)
      ;

}
 
