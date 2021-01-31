#include "lm4f120h5qr.h"
#define RCGCGPIO (*((unsigned int*) 0x400FE608U)) // clock gating control
#define GPIOF_BASE 0x40025000u

// Applies offset
#define GPIOF_DIR (*((unsigned int*) (GPIOF_BASE + 0x400u))) // direction
#define GPIOF_DEN (*((unsigned int*) (GPIOF_BASE + 0x51Cu))) // digital enable
#define GPIOF_DATA (*((unsigned int*) (GPIOF_BASE + 0x3FCu))) // data register for GPIOF

// Example of volatile
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
int main()
{
  RCGCGPIO = 0x20U; // Set bit 5 (the sixth bit) to 1. Manages clock gating, powers on block for GPIO-F
  GPIOF_DIR = 0x0EU; // Set bits 1, 2, and 3 to 1. Sets LED pin as output (like DDR)
  GPIOF_DEN = 0x0EU; // Set bits 1, 2, and 3 to 1 in digital function register (?)
  
  while(1) {
   GPIOF_DATA = 0x02U; // Set bit 1, turns on LED.
   
   // delay loop
   int volatile counter = 0;
   while(counter < 1000) counter++;
   
   GPIOF_DATA = 0x00U; // Set bit 1, turns off LED.
   counter = 0;
   while(counter < 1000) counter++;
  }
 
  return 0;
}
 