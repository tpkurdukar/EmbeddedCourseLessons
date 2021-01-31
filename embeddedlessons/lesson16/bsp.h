#ifndef __BSP_H__
#define __BSP_H__

#define SYS_CLOCK_HZ 16000000u // there are 16000000 cycles in one second

// redefine LEDS
#define LED_RED   (1U << 1)
#define LED_BLUE  (1U << 2)
#define LED_GREEN (1U << 3)

void delay(int volatile iter);

#endif // __BSP_H__