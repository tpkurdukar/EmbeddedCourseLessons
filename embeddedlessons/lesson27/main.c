#include <stdint.h>
#include "qpc.h"
#include "bsp.h"
#include "TM4C123GH6PM.h" // tm4c cmsis

Q_DEFINE_THIS_FILE

QXSemaphore SW1_sema; // Semaphore used to toggle blue LED with button press

uint32_t stack_blinky1[40];
QXThread blinky1;
// remember that the stack grows down, from end of stack to beginning

// In QXK, a thread function has a signature that takes allows it to access the associated thread
void main_blinky1(QXThread * const me) {
    while (1) {
        uint32_t volatile i;
			// 1.2 ms execution time for this block, slightly longer than SysTick clock tick (which happens every 1 ms)
				for(i = 1500u; i != 0u; --i) {
					BSP_ledGreenOn();
					BSP_ledGreenOff();
				}
				QXThread_delay(1u); // block for 1 SysTick (1 ms)
    }
}

uint32_t stack_blinky2[40];
QXThread blinky2;

void main_blinky2(QXThread * const me) {
    while (1) {
				uint32_t volatile i;
			
			// Synchronize thread with semaphore. Thread will "block" by stopping here until either semaphore is signalled
			// or until the specified timeout has been elapsed
				QXSemaphore_wait(&SW1_sema,  /* pointer to semaphore to wait on */
			QXTHREAD_NO_TIMEOUT); /* timeout for waiting: indefinite */
			// CPU load runs 3 times longer than blinky1, 3.6 ms
				for(i = 3*1500u; i != 0u; --i) {
					BSP_ledBlueOn();
					BSP_ledBlueOff();
				}
				// Replacing blocking delay with semaphore for delaying
				//QXThread_delay(50u); // block for 50 SysTicks (50 ms delay)
    }
}

uint32_t stack_blinky3[40];
QXThread blinky3;

void main_blinky3(QXThread * const me) {
    while (1) {
				BSP_ledRedOn();
        QXThread_delay(BSP_TICKS_PER_SEC / 3U);
        BSP_ledRedOff();
        QXThread_delay(BSP_TICKS_PER_SEC * 3U / 5U);
    }
}

uint32_t stack_idleThread[40];

int main() {
	
  BSP_init();
	// Don't need to pass in an idle thread stack because this RTOS reuses the main C call stack
	QF_init();
	
	// Initialize binary semaphore
	QXSemaphore_init(&SW1_sema, /* pointer to semaphore to initialize */
                     0U,  /* initial semaphore count (singaling semaphore) */
                     1U); /* maximum semaphore count (binary semaphore) */

	// In QPC API, need to initialize first by calling the constructor, and then start the thread
	QXThread_ctor(&blinky1, &main_blinky1, 0);
	
	// Macro used to start thread
	QXTHREAD_START(&blinky1,
									5u, // Priority
									(void *) 0, 0, // Message queue buffer (not used)
									stack_blinky1, sizeof(stack_blinky1), // Stack buffer and size
									(void *)0); // Extra parameter (ignored at this point)
	
	QXThread_ctor(&blinky2, &main_blinky2, 0);
	QXTHREAD_START(&blinky2,
									2u, // Priority
									(void *) 0, 0, // Message queue buffer (not used)
									stack_blinky2, sizeof(stack_blinky2), // Stack buffer and size
									(void *)0); // Extra parameter (ignored at this point)
	
  /*
	QXThread_ctor(&blinky3, &main_blinky3, 0);
	QXTHREAD_START(&blinky3,
									1u, // Priority
									(void *) 0, 0, // Message queue buffer (not used)
									stack_blinky3, sizeof(stack_blinky3), // Stack buffer and size
									(void *)0); // Extra parameter (ignored at this point)
	*/
									
	//QXThread_start(&blinky3, 3u, &main_blinky3, stack_blinky3, sizeof(stack_blinky3)); // start blinky3 thread
	
	/* transfer control to the RTOS to run the threads */
	QF_run();
}
