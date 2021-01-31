#include <stdint.h>
#include "bsp.h"

uint32_t stack_blinky1[40];
uint32_t *sp_blinky1 = &stack_blinky1[40]; // points 1 word beyond stack
// remember that the stack grows down, from end of stack to beginning


void main_blinky1() {
    BSP_init();
    while (1) {
        BSP_ledGreenOn();
        BSP_delay(BSP_TICKS_PER_SEC / 4U);
        BSP_ledGreenOff();
        BSP_delay(BSP_TICKS_PER_SEC * 3U / 4U);
    }
}

uint32_t stack_blinky2[40];
uint32_t *sp_blinky2 = &stack_blinky2[40];
void main_blinky2() {
    BSP_init();
    while (1) {
				BSP_ledBlueOn();
        BSP_delay(BSP_TICKS_PER_SEC / 2U);
        BSP_ledBlueOff();
        BSP_delay(BSP_TICKS_PER_SEC * 3U);
    }
}

int main() {
	uint32_t volatile run = 0u;
	
   BSP_init();
    
	/*
		main_blinky1();
		main_blinky2(); // Compiler eliminates this call as unreachable because it knows the first call never returns
	*/
	
	/* fabricate Cortex-M ISR stack frame for blinky1 */
	// full stack, so must decrement stack first
	*(--sp_blinky1) = (1u << 24); /* xPSR, thumb mode */
	*(--sp_blinky1) = (uint32_t) &main_blinky1; /* PC */
	*(--sp_blinky1) = 0x0000000Eu; /* LR, doesn't really matter because threads don't return */
	*(--sp_blinky1) = 0x0000000Cu; /* R12 */
	*(--sp_blinky1) = 0x00000003u; /* R3 */
	*(--sp_blinky1) = 0x00000002u; /* R2 */
	*(--sp_blinky1) = 0x00000001u; /* R1 */
	*(--sp_blinky1) = 0x00000000u; /* R0 */
	/* append R4-R11 to stack frame to preserve them for this thread */
	*(--sp_blinky1) = 0x00000002u; /* R11 */
	*(--sp_blinky1) = 0x00000001u; /* R10 */
	*(--sp_blinky1) = 0x00000000u; /* R9 */
	*(--sp_blinky1) = 0x00000002u; /* R8 */
	*(--sp_blinky1) = 0x00000001u; /* R7 */
	*(--sp_blinky1) = 0x00000000u; /* R6 */
	*(--sp_blinky1) = 0x00000002u; /* R5 */
	*(--sp_blinky1) = 0x00000001u; /* R4 */
	
	
	/* fabricate Cortex-M ISR stack frame for blinky2 */
	*(--sp_blinky2) = (1u << 24); /* xPSR, thumb mode */
	*(--sp_blinky2) = (uint32_t) &main_blinky2; /* PC */
	*(--sp_blinky2) = 0x0000000Eu; /* LR, doesn't really matter because threads don't return */
	*(--sp_blinky2) = 0x0000000Cu; /* R12 */
	*(--sp_blinky2) = 0x00000003u; /* R3 */
	*(--sp_blinky2) = 0x00000002u; /* R2 */
	*(--sp_blinky2) = 0x00000001u; /* R1 */
	*(--sp_blinky2) = 0x00000000u; /* R0 */
	/* append R4-R11 to stack frame to preserve them for this thread */
	*(--sp_blinky2) = 0x00000002u; /* R11 */
	*(--sp_blinky2) = 0x00000001u; /* R10 */
	*(--sp_blinky2) = 0x00000000u; /* R9 */
	*(--sp_blinky2) = 0x00000002u; /* R8 */
	*(--sp_blinky2) = 0x00000001u; /* R7 */
	*(--sp_blinky2) = 0x00000000u; /* R6 */
	*(--sp_blinky2) = 0x00000002u; /* R5 */
	*(--sp_blinky2) = 0x00000001u; /* R4 */
    
	while(1) {
	// so that main does not terminate. Wait for threads to switch around
	} 
}