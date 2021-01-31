#include <stdint.h>
#include "miros.h"


/* Pointers to keep track of current thread and next thread to execute */
// Note that they are volatile pointers, because the pointers themselves are used in PendSV.
// If volatile was before the *, you would get a non-volatile pointer to a volatile object. But we want to modify the pointers themselves
// inside of the ISR.
OSThread * volatile OS_curr, * volatile OS_next;

void OS_init(void) {
	/* Sets PendSV priority to lowest possible, by placing 3 bits at the front (FF for the sake of generalization to other processors) */
	// Note: this way of accessing the peripheral register was used instead of core struct to generalize it to other Cortex cores,
	// since all Cortex cores have the same address for PendSV's priority register.
	*(uint32_t volatile *) 0xE000ED20 |= (0xFFu << 16);
}

void OS_sched(void) {
	// OS_next = ... (how OS_sched decides the next thread to execute will be decided later)
	
	if(OS_next != OS_curr) {
		*(uint32_t volatile *)0xE000ED04 = (1u << 28); // Place a 1 into the register that will set a flag for PendSV
	}
	
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
	
	/* this pre-fills the unused part of the stack from right above sp to stk_limit with 0xdeadbeef */
	for(sp = sp - 1u; sp >= stk_limit; --sp) {
		*sp = 0xDEADBEEFu;
	}
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
