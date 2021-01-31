#ifndef MIROS_H
#define MIROS_H

#include <stdint.h>

/* Thread Control Block (TCB) */
typedef struct {
	void *sp; /* stack pointer, declared as a void pointer */
	uint32_t timeout; /* each thread needs a timeout counter */
	uint8_t priority; /* small integer declaring priority of thread */
} OSThread;

/* typedef OSThreadHandler as a pointer to a function containing no args, no returns */
typedef void (*OSThreadHandler)(); // weird syntax ik

/* Function used to set priority of PendSV to have the lowest possible priority of all ISRs */
// Note: this function has to be called with interrupts disabled. Otherwise, race conditions may occur
void OS_init(void *stkSto, uint32_t stkSize);

/* Function used to trigger PendSV based on the decision of which thread to switch to next */
void OS_sched(void);

/* Transfers control to the RTOS to run the threads, leaves main */
void OS_run(void);

/* Callback function that configures and starts interrupts */
void OS_onStartup(void);

/* main for idle thread, necessary to use blocking */
void main_idleThread(void);

/* callback to handle idle thread */
void OS_onIdle(void);

/* Blocks a thread by loading its timeout counter with the number of ticks */
void OS_delay(uint32_t ticks);

/* Called by SysTick interrupt: will decrement the timeout counters of all blocked threads and unblock those that reach 0 */
void OS_tick(void);

/* Function called to initialize call stack of thread by fabricating the exception frame*/
void OSThread_start(
	OSThread *me,
	uint8_t priority,
	OSThreadHandler threadHandler,
	void *stkSto, uint32_t stkSize);

#endif
