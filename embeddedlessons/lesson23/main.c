#include <stdint.h>
#include "miros.h"
#include "bsp.h"
#include "TM4C123GH6PM.h" // tm4c cmsis

uint32_t stack_blinky1[40];
OSThread blinky1;
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
OSThread blinky2;

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
	OS_init();

	/* Send address of thread for OSThread ptr, address of function for threadhandler ptr, address of stack, size of stack */
  OSThread_start(&blinky1, &main_blinky1, stack_blinky1, sizeof(stack_blinky1)); // fabricates interrupt stack frame for blinky1
	OSThread_start(&blinky2, &main_blinky2, stack_blinky2, sizeof(stack_blinky1)); // fabricates interrupt stack frame for blinky2

	// this function is declared in cmsis
	NVIC_SetPriority(SysTick_IRQn, 0u); // Set the SysTick interrupt higher
	while(1) {
	// so that main does not terminate. Wait for threads to switch around
	} 
}
