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
static uint32_t volatile l_tickCtr;

void SysTick_Handler(void) {
		QXK_ISR_ENTRY(); // Inform QXK about entering an ISR
		QF_TICK_X(0u, (void *)0); // Services the timeouts for specific clock tick rate, specified in constructors
	
		QXK_ISR_EXIT(); // Exit an interrupt, calls QXK's preemptive scheduler for you to switch contexts if necessary
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


void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    NVIC_SystemReset();
}
