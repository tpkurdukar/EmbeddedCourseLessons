/* Board Support Package (BSP) for the EK-TM4C123GXL board */

#include "qpc.h" // include before bsp so that SW1_sema is defined
#include "bsp.h"
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

/* on-board LEDs */
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)
/* on-board switch */
#define BTN_SW1		(1u << 4)



// SEMA
//static QXSemaphore Morse_sema;

// MUTEX
static QXMutex Morse_mutex;

void SysTick_Handler(void) {
		QXK_ISR_ENTRY(); // Inform QXK about entering an ISR
		QF_TICK_X(0u, (void *)0); // Services the timeouts for specific clock tick rate, specified in constructors
	
		QXK_ISR_EXIT(); // Exit an interrupt, calls QXK's preemptive scheduler for you to switch contexts
}

void GPIOPortF_IRQHandler(void) {
		QXK_ISR_ENTRY(); // Inform QXK about entering an ISR
	// make sure the interrupt was caused by the button press before signaling semaphore
		if((GPIOF_AHB->RIS & BTN_SW1) != 0u) { 
			QXSemaphore_signal(&SW1_sema); // signal the semaphore
		}
		GPIOF_AHB->ICR = 0xFFu; // clear interrupt sources
	
		QXK_ISR_EXIT(); // Exit an interrupt, calls QXK's preemptive scheduler for you to switch contexts if necessary
}

void BSP_init(void) {
    SYSCTL->RCGCGPIO  |= (1U << 5); /* enable Run mode for GPIOF */
    SYSCTL->GPIOHBCTL |= (1U << 5); /* enable AHB for GPIOF */
    GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN);

    /* Configure switch SW1 */
		GPIOF_AHB->DIR &= ~BTN_SW1; // input
		GPIOF_AHB->DEN |= BTN_SW1; // digital enable
		GPIOF_AHB->PUR |= BTN_SW1; // enable internal pull-up: pin is normally high

		/* GPIO interrupt setup for SW1 */
		GPIOF_AHB->IS &= ~BTN_SW1; // edge detect for SW1
		GPIOF_AHB->IBE &= ~BTN_SW1; // only one edge generates the interrupt
		GPIOF_AHB->IEV &= ~BTN_SW1; // trigger interrupt on falling edge
		GPIOF_AHB->IM |= BTN_SW1;	// enable GPIOF interrupt for SW1
	
		// SEMA
		/*
		// Initialized to 1, resource initially available
		QXSemaphore_init(&SW1_sema,  pointer to semaphore to initialize 
                     1U,  // initial semaphore count (singaling semaphore) 
                     1U); // maximum semaphore count (binary semaphore) 
		*/
		
		// MUTEX
		QXMutex_init(&Morse_mutex, 6u); // priority ceiling 6
		// prio ceiling mutex is similar to thread, must be initialized after QF_init()
}
/* Callback functions: designed in QXK but not implemented there */
void QF_onStartup(void) {
		SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
	
		// Cannot set SysTick interrupt's priority to 0, because in cortex m3 and above this would fall under the category of kernel unaware interrupts
	  // according to QPC. PL0 with 3 prio bits is kernel unaware, but SysTick interacts with the kernel so it should not be
	  // set as such.
	
		NVIC_SetPriority(SysTick_IRQn, QF_AWARE_ISR_CMSIS_PRI); // Set the SysTick interrupt to a high priority of kernel aware interrupts
		NVIC_SetPriority(GPIOF_IRQn, QF_AWARE_ISR_CMSIS_PRI + 1); // Greater prio level as SysTick
    // __enable_irq(); redundant
		NVIC_EnableIRQ(GPIOF_IRQn); // enable IRQs in NVIC
}

/* Provided for situation when an application exits (this never exits, so empty) */
void QF_onCleanup(void) {

}

void QXK_onIdle(void) {
	//__WFI(); /* Low power mode: stops CPU clock and waits for interrupt to occur (interrupts are asynchronous) */
}

void BSP_ledRedOn(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED;
}

void BSP_ledRedOff(void) {
    GPIOF_AHB->DATA_Bits[LED_RED] = 0U;
}

void BSP_ledBlueToggle(void) {
	QF_CRIT_STAT_TYPE istat; // Automatic variable which will remember interrupt disable status
	
	QF_CRIT_ENTRY(istat); // Reads current interrupt disable status and saves it into provided variable, then disables interrupts
	
	
	// Recall that the data register holds all 8 bits of the registers, so it becomes a shared resource
	GPIOF_AHB->DATA = GPIOF_AHB->DATA ^ LED_BLUE;
	
	QF_CRIT_EXIT(istat);
}

void BSP_ledGreenToggle(void) {
	// QXK kernel's mechanism for critical sections
	QF_CRIT_STAT_TYPE istat; // Automatic variable which will remember interrupt disable status
	
	QF_CRIT_ENTRY(istat); // Reads current interrupt disable status and saves it into provided variable, then disables interrupts
	
	// Recall that the data register holds all 8 bits of the registers, so it becomes a shared resource
	GPIOF_AHB->DATA = GPIOF_AHB->DATA ^ LED_GREEN;
	QF_CRIT_EXIT(istat); // Restores saved interrupt disable status from provided istatus parameter
	// This convoluted implementation is useful for critical section nesting
}

void BSP_ledBlueOn(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
}

void BSP_ledBlueOff(void) {
    GPIOF_AHB->DATA_Bits[LED_BLUE] = 0U;
}

void BSP_ledGreenOn(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
}

void BSP_ledGreenOff(void) {
    GPIOF_AHB->DATA_Bits[LED_GREEN] = 0U;
}

void BSP_sendMorseCode(uint32_t bitmask) {
	uint32_t volatile delay_ctr;
	enum { DOT_DELAY = 150 };
	
	// LOCK
	//QSchedStatus sstat; // trick that allows scheduler locks to nest if necessary
	
	// SEMA
	// Wait for a semaphore to have a token 
	//QXSemaphore_wait(&Morse_sema,  /* pointer to semaphore to wait on */
			//QXTHREAD_NO_TIMEOUT); /* timeout for waiting: indefinite */
	
	// LOCK
	//sstat = QXK_schedLock(5u); // priority ceiling 5: the level up to which to lock the scheduler
	// All threads below or equal to the ceiling level won't be scheduled, but threads above the ceiling
	// are scheduled as usual. Should be at least as high as the highest priority thread that uses
	// the shared resource. The thread that invokes this now takes ownership of the scheduler lock
	
	// MUTEX
	QXMutex_lock(&Morse_mutex, QXTHREAD_NO_TIMEOUT); // Thread becomes owner of the mutex
	
	for(; bitmask != 0u; bitmask <<= 1) {
		if((bitmask & (1u << 31)) != 0u) {
			BSP_ledGreenOn();
		} else {
			BSP_ledGreenOff();
		}
		for(delay_ctr = DOT_DELAY; delay_ctr != 0u; --delay_ctr);
	}
	BSP_ledGreenOff();
	for(delay_ctr = 7 * DOT_DELAY; delay_ctr != 0u; --delay_ctr);
	
	// SEMA
	//QXSemaphore_signal(&Morse_sema); // Insert token for the next message
	
	// LOCK
	// QXK_schedUnlock(sstat);
	QXMutex_unlock(&Morse_mutex);
}


void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    NVIC_SystemReset();
}
