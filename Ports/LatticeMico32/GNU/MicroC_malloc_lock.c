/* 
 * This file contains implementation for multithread protection
 * when calling malloc/free from MicroC/OS tasks.
 */
#include <reent.h>
#include "ucos_ii.h"
#include "MicroC_malloc_lock.h"


static OS_EVENT* __microC_lock;
static int __microC_lock_owner_id;
static int __microC_lock_count;


/*
 * This function must be called after initializing the
 * OS (OSInit) and prior to any task calling malloc
 * or free routines from within the tasks.  This function uses
 * semaphore in lieu of a mutex as the MicroC mutex requires
 * specifying task-priority.
 *
 * This function returns 0 if successful else returns a non-zero
 * value.
 */
int InitMallocLock(void)
{
	/* initialize lock properties */
	__microC_lock_owner_id = -1;
	__microC_lock_count = 0;


	/* 
	 * Create a semaphore with an initial count of 1 i.e.
	 * only one task can grab it.
	 */
	__microC_lock = OSSemCreate(1);
	if(__microC_lock == (OS_EVENT *)0)
		return(1);


	/* all done */
	return(0);
}


/*
 * malloc/free calls this function and passes a pointer to
 * the task's reent structure.  This structure however does
 * not contain any protection information for MicroC.  So
 * this function simply ignores it.
 */
void __malloc_lock(struct _reent *ptr)
{
	INT8U err;

    if (OSRunning == OS_TRUE) {
		/* wait for lock to be available */
		OSSemPend(__microC_lock, 0, &err);
    }

	return;
}


/*
 * malloc/free calls this function and passes a pointer to
 * the task's reent structure.  This structure however does
 * not contain any protection information for MicroC.  So
 * this function simply ignores it.
 */
void __malloc_unlock(struct _reent *ptr)
{
    if(OSRunning == OS_TRUE){
		OSSemPost( __microC_lock );
	}

	return;
}

