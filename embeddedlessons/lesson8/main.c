#include "lm4f120h5qr.h"

#define LED_RED (1u << 1)
#define LED_BLUE (1u << 2)
#define LED_GREEN (1u << 3)

void delay(void);

void delay(void) {
    // delay loop
   int volatile counter = 0;
   while(counter < 1000) counter++;
}

int main()
{
  // when using bit set idiom (OR operation), make sure datasheet permits R/W on the bits 
  SYSCTL_RCGCGPIO_R |= 1u << 5; // Set bit 5 (the sixth bit) to 1. Manages clock gating, powers on block for GPIO-F
  SYSCTL_GPIOHBCTL_R |= (1u << 5); // enable high performance bus
  
  GPIO_PORTF_AHB_DIR_R |= (LED_RED | LED_BLUE | LED_GREEN); // Set bits 1, 2, and 3 to 1. Sets LED pin as output (like DDR)
  GPIO_PORTF_AHB_DEN_R |= (LED_RED | LED_BLUE | LED_GREEN); // Set bits 1, 2, and 3 to 1 in digital function register (?)
  
  GPIO_PORTF_AHB_DATA_BITS_R[LED_BLUE] = LED_BLUE; // Set bit 1, turns on blue LED.

  while(1) {
   //GPIO_PORTF_DATA_R |= LED_RED; // This is writing to every GPIO bit (1111111100), which we don't want. We just want to change the one for red
    // Had to left shift by 2 to take into account the size of the element
    // There are 256 separate data registers for each combination of GPIO bits in GPIOF.
   //*((unsigned long volatile *) (0x40025000 + (LED_RED << 2))) = LED_RED; // Find address to change just the red LED's gpio data register
   
    // interrupt safe way of changing GPIO bits
   GPIO_PORTF_AHB_DATA_BITS_R[LED_RED] = LED_RED;
   //*(GPIO_PORTF_DATA_BITS_R + LED_RED) = LED_RED;
   delay();
   
   //GPIO_PORTF_DATA_R &= ~LED_RED; // Set bit 1, turns off red LED and preserves other bits
   GPIO_PORTF_AHB_DATA_BITS_R[LED_RED] = 0;
   delay();
   
  }
 
  return 0;
}


 