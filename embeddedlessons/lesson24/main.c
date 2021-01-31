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

uint32_t stack_blinky3[40];
OSThread blinky3;

void main_blinky3() {
    BSP_init();
    while (1) {
				BSP_ledRedOn();
        BSP_delay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        BSP_delay(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}

int main() {
	
  BSP_init();
	OS_init();

	/* Send address of thread for OSThread ptr, address of function for threadhandler ptr, address of stack, size of stack */
  OSThread_start(&blinky1, &main_blinky1, stack_blinky1, sizeof(stack_blinky1)); // fabricates interrupt stack frame for blinky1
	OSThread_start(&blinky2, &main_blinky2, stack_blinky2, sizeof(stack_blinky2)); // fabricates interrupt stack frame for blinky2
	OSThread_start(&blinky3, &main_blinky3, stack_blinky3, sizeof(stack_blinky3)); // start blinky3 thread
	
	/* transfer control to the RTOS to run the threads */
	OS_run();
}
