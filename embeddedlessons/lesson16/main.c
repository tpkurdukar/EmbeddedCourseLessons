#include <stdint.h>

#include "tm4c_cmsis.h"
#include "bsp.h"

#define SYS_CLOCK_HZ 16000000u // there are 16000000 cycles in one second

#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

int main() {
    
    SYSCTL->RCGC2 |= (1U << 5);  /* enable clock for GPIOF */
    SYSCTL->GPIOHSCTL |= (1U << 5); /* enable AHB for GPIOF */
    GPIOF_HS->DIR |= (LED_RED | LED_BLUE | LED_GREEN);
    GPIOF_HS->DEN |= (LED_RED | LED_BLUE | LED_GREEN);
    
    // configuring registers for system timer SysTick
    // want half a second between interrupts
    // Make sure this doesn't overflow the 24 bit STCURRENT reg
    // 8000000 cycles = 0x7A11FF, 3 bytes, so we're good
    SysTick->LOAD = SYS_CLOCK_HZ/2u - 1u; // minus one b/c it counts through 0
    SysTick->VAL  = 0u; // clear on write, so it will clear regardless of value
    SysTick->CTRL = (1u << 2) | (1u << 1) | 1u; // from sheet: clk source, interrupt enable, counter enable

    __enable_irq(); // there is a PREMASK bit that must be low for interrupts to work
    //GPIOF_HS->DATA_Bits[LED_BLUE] = LED_BLUE;
    while (1) {
    }
    //return 0;
}
