#include <stdint.h>
#include "miros.h"
#include "qassert.h"

Q_DEFINE_THIS_FILE // Defines namestring for the file, necessary to use assertions from qpc

/* Pointers to keep track of current thread and next thread to execute */
// Note that they are volatile pointers, because the pointers themselves are used in PendSV.
// If volatile was before the *, you would get a non-volatile pointer to a volatile object. But we want to modify the pointers themselves
// inside of the ISR.
OSThread * volatile OS_curr = 0, * volatile OS_next = 0;
OSThread idleThread;

/* Array of all thread ptrs, used in scheduler. This RTOS can handle up to 32 threads */
OSThread *OS_threads[32 + 1];
/* Number of threads that have been started so far, used to populate array */
uint8_t OS_threadsAdded = 0; 
/* Current thread index, used in round robin scheduling */
uint8_t OS_currIdx = 0;
/* Bitmask of threads that are ready to run */
uint32_t OS_readyThreads;

void main_idleThread(void) {
    while(1) {
			OS_onIdle();
		}
}

void OS_init(void *stkSto, uint32_t stkSize) {
	/* Sets PendSV priority to lowest possible, by placing 3 bits at the front (FF for the sake of generalization to other processors) */
	// Note: this way of accessing the peripheral register was used instead of core struct to generalize it to other Cortex cores,
	// since all Cortex cores have the same address for PendSV's priority register.
	*(uint32_t volatile *) 0xE000ED20 |= (0xFFu << 16);
	
	/* start idleThread */
	OSThread_start(&idleThread, &main_idleThread, stkSto, sizeof(stkSize));
}

void OS_sched(void) {
	
	/* Check for idle condition */
	if(OS_readyThreads == 0u) {
		OS_currIdx = 0u; // index of idle thread
	} else {
		/* Have some ready to run threads, but cannot just choose the next thread index. Need
				to keep going around in a round robin fashion until we encounter a thread that is
				ready to run */
		do {
			++OS_currIdx; // increment idx of thread array
			if(OS_currIdx == OS_threadsAdded) {
				OS_currIdx = 1u; // Reset index to the first thread (skipping 0) if we have reached the limit of threads added
			}
			
			// Keep checking each bit in readyThreads until you find a non-zero bit
		} while ((OS_readyThreads & (1u << (OS_currIdx - 1u))) == 0u);
	}
	
	// Have found a non-zero bit, set the corresponding index as the next thread to execute
	OS_next = OS_threads[OS_currIdx];
	
	if(OS_next != OS_curr) {
		*(uint32_t volatile *)0xE000ED04 = (1u << 28); // Place a 1 into the register that will set a flag for PendSV
	}
	
}

void OS_delay(uint32_t ticks) {
	__disable_irq();
	
	// Make sure OS_delay is never called on the idleThread 
	Q_REQUIRE(OS_curr != OS_threads[0]); // precondition
	
	// Load thread's timeout counter, set it as not ready to run, and run the scheduler to immediately switch contexts
	OS_curr->timeout = ticks;
	OS_readyThreads &= ~(1u << (OS_currIdx - 1u));
	OS_sched(); // context switch will happen right after interrupts are reenabled
	__enable_irq();
}

void OS_tick(void) {
	uint8_t n;
	for(n = 1u; n < OS_threadsAdded; ++n) {
		if(OS_threads[n]->timeout != 0u) {
			--OS_threads[n]->timeout;
			if(OS_threads[n]->timeout == 0u) {
				OS_readyThreads |= (1u << (n - 1u)); // Unblock thread if necessary
			}
		}
	}
	
	// Note: no need to disable interrupts in this function. No race conditions are possible because
	// this function is called from SysTick_Handler. The only other functions that share data with this
	// are called by threads, which cannot preempt the SysTick ISR. 
	// Also, no need to call the scheduler to context switch since this is done in the SysTick ISR anyway
}

void OS_run(void) {
	/* callback function: not defined in RTOS itself. Need to define this in the application */
	OS_onStartup(); // Will configure interrupts
	
	/* Calls scheduler, which triggers PendSV interrupt that correctly returns to the first thread to run */
	__disable_irq();
	OS_sched();
	__enable_irq(); // PendSV runs as soon as interrupts are enabled
	
	/* Code from here to the rest of OS_run() should never execute, because control has been transfered to multithreading */
	Q_ERROR(); // assertion that will always evaluate to false
}


void OSThread_start(
	OSThread *me,
	OSThreadHandler threadHandler,
	void *stkSto, uint32_t stkSize) 
{
	/* Declares stack pointer to be at the end of the stack memory.
	 * Necessary because in ARM Cortex-M, the stack grows from high to low
	 */
	uint32_t * sp = (uint32_t *) (((uint32_t)stkSto + stkSize) / 8 * 8); // Stack is a 32 bit number. Cast as such, add it to size, cast back to ptr
	uint32_t * stk_limit;
	// / 8 * 8 is there to force the address to be aligned at the 8 byte boundary, i.e the address % 8 = 0. This will always round down, so it will be within allocated space
	
	*(--sp) = (1u << 24); /* xPSR, thumb mode */
	*(--sp) = (uint32_t) threadHandler; /* PC, where we want code to jump to for this thread */
	*(--sp) = 0x0000000Eu; /* LR, doesn't really matter because threads don't return */
	*(--sp) = 0x0000000Cu; /* R12 */
	*(--sp) = 0x00000003u; /* R3 */
	*(--sp) = 0x00000002u; /* R2 */
	*(--sp) = 0x00000001u; /* R1 */
	*(--sp) = 0x00000000u; /* R0 */
	/* append R4-R11 to stack frame to preserve them for this thread */
	*(--sp) = 0x0000000Bu; /* R11 */
	*(--sp) = 0x0000000Au; /* R10 */
	*(--sp) = 0x00000009u; /* R9 */
	*(--sp) = 0x00000008u; /* R8 */
	*(--sp) = 0x00000007u; /* R7 */
	*(--sp) = 0x00000006u; /* R6 */
	*(--sp) = 0x00000005u; /* R5 */
	*(--sp) = 0x00000004u; /* R4 */
	
	/* save top of stack frame into TCB */
	me->sp = sp; 
	/* this is used so that the top-most limit of the stack is at an address that is divisible by 8. */
	stk_limit = (uint32_t *) (((((uint32_t) stkSto - 1u) / 8) + 1u) * 8); // e.g 100 would go to 104, which is divisible by 8
	
	/* this pre-fills the unused part of the stack from right above sp to stk_limit with 0xdeadbeef for visual purposes */
	for(sp = sp - 1u; sp >= stk_limit; --sp) {
		*sp = 0xDEADBEEFu;
	}
	
	/* Register the thread with the OS, with check to make sure there is no overflow */
	// Q_DIM just gives you the max number of elements in an array
	Q_ASSERT(OS_threadsAdded < Q_DIM(OS_threads));
	// Q_ASSERT will call Q_onAssert() error handler if the expression inside evaluates to false
	OS_threads[OS_threadsAdded] = me; // adds current thread to array
	
	/* Configure thread as ready to run if it is not the idle thread. When first started, thread goes to ready state */
	if(OS_threadsAdded > 0u) {
		OS_readyThreads |= (1u << (OS_threadsAdded - 1u));
	}
	++OS_threadsAdded;
	
}

__asm
void PendSV_Handler(void) {
	
	/***** C Code for PendSV_Handler
	
	void * sp; // Will be used to represent SP register in CPU
	
	__disable_irq();
	// Step 1: save stack content of current thread (if there is a current thread)
	// If this is the first time PendSV is called for example, it won't be from a current thread, so check for NULL
	if(OS_curr != (OSThread *)0) {
		// Push registers R4-R11 on the stack, which won't be in C
		// Then, the stack ptr in the TCB must be updated to current value of SP register
		OS_curr->sp = sp; // sp will contain SP register's value, which has already changed from pushing of R4-R11
	}  
	
	// Step 2: Restore context of next scheduled thread by loading sp member into SP register of CPU and popping R4-R11
	sp = OS_next->sp;
	OS_curr = OS_next;
	// pop registers r4-r11
	__enable_irq();
	*/
	IMPORT	OS_curr
	IMPORT	OS_next
	/* __disable_irq(); */
	
	CPSID				I
	
	/* if (OS_curr != (OSThread *) 0) */
	LDR					r1, =OS_curr ; =OS_curr means address of OS_curr variable (recall that OS_curr is a ptr)
	LDR					r1, [r1, #0x00] ; obtain address stored in OS_curr variable (OS_curr is a ptr)
	CBZ					r1, PendSV_Restore
	
	/* push r4-r11 onto call stack of current thread */
	PUSH				{r4-r11}
	
	/* update stack pointer member of current thread */
	LDR					r1, =OS_curr
	LDR					r1, [r1, #0x00]
	STR 				sp, [r1, #0x00] ; Store SP reg from CPU into sp member of current thread

PendSV_Restore
	/* sp = OS_next->sp; */
	LDR					r1, =OS_next
	LDR					r1, [r1, #0x00]
	LDR					sp, [r1, #0x00] ; Place sp member from next thread into SP register
	
	/* OS_curr = OS_next; */
	LDR					r1, =OS_next
	LDR					r1, [r1, #0x00]
	LDR					r2, =OS_curr
	STR 				r1, [r2, #0x00]
	
	/* pop registers r4-r11 */
	POP					{r4-r11}
	
	/* __enable_irq(); */
	CPSIE				I
	BX					lr
	
}
