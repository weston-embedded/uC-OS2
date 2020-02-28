#ifndef MICROC_MALLOC_LOCK_H_
#define MICROC_MALLOC_LOCK_H_

/*
 * This function must be called after initializing the
 * OS (OSInit) and prior to any task calling malloc
 * or free routines from within the tasks.
 *
 * This function need not be called if not using standard
 * libc functions (especially malloc, free, printf, file-ops)
 * within tasks i.e. anything that needs malloc/free.
 *
 * This function returns 0 if successful else returns a non-zero
 * value.
 */
int InitMallocLock(void);

#endif /*MICROC_MALLOC_LOCK_H_*/
