#ifndef MIROS_H
#define MIROS_H

#include <stdint.h>

/* Thread Control Block (TCB) */
typedef struct {
	void *sp; /* stack pointer, declared as a void pointer */
	/* other attributes associated with a thread would go here */
} OSThread;

/* typedef OSThreadHandler as a pointer to a function containing no args, no returns */
typedef void (*OSThreadHandler)(); // weird syntax ik

/* Function used to set priority of PendSV to have the lowest possible priority of all ISRs */
// Note: this function has to be called with interrupts disabled. Otherwise, race conditions may occur
void OS_init(void);

/* Function used to trigger PendSV based on the decision of which thread to switch to next */
void OS_sched(void);

/* Function called to initialize call stack of thread by fabricating the exception frame*/
void OSThread_start(
	OSThread *me,
	OSThreadHandler threadHandler,
	void *stkSto, uint32_t stkSize);

#endif
