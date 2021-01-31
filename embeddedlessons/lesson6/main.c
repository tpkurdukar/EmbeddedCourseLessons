#include "lm4f120h5qr.h"

#define LED_RED (1u << 1)
#define LED_BLUE (1u << 2)
#define LED_GREEN (1u << 3)

int main()
{
  // when using bit set idiom (OR operation), make sure datasheet permits R/W on the bits 
  SYSCTL_RCGCGPIO_R |= 1u << 5; // Set bit 5 (the sixth bit) to 1. Manages clock gating, powers on block for GPIO-F
  GPIO_PORTF_DIR_R |= (LED_RED | LED_BLUE | LED_GREEN); // Set bits 1, 2, and 3 to 1. Sets LED pin as output (like DDR)
  GPIO_PORTF_DEN_R |= (LED_RED | LED_BLUE | LED_GREEN); // Set bits 1, 2, and 3 to 1 in digital function register (?)
  
  GPIO_PORTF_DATA_R = LED_BLUE; // Set bit 1, turns on blue LED.

  while(1) {
   GPIO_PORTF_DATA_R |= LED_RED; // Set bit 1, turns on red LED while keeping blue LED on.
   // Use bitwise operator
   
   // delay loop
   int counter = 0;
   while(counter < 1000) counter++;
   
   GPIO_PORTF_DATA_R &= ~LED_RED; // Set bit 1, turns off red LED and preserves other bits
   counter = 0;
   while(counter < 1000) counter++;
  }
 
  return 0;
}
 