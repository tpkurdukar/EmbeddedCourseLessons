#include "lm4f120h5qr.h"
#include "delay.h"

#define LED_RED (1u << 1)
#define LED_BLUE (1u << 2)
#define LED_GREEN (1u << 3)

unsigned fact(unsigned n);
int * swap(int *x, int *y);

unsigned fact(unsigned n) {
  unsigned foo[6];
  foo[n] = n;
  if(n == 0u) return 1u;
  return foo[n] * fact(foo[n] - 1u);
}

int * swap(int *x, int *y) {
  static int tmp[2];
  // If we do not make this static, then tmp will be allocated on the stack as a local var. 
  // Then during callee teardown, tmp will be erased. See the problem?
  // In main, when tmp is used, it will be pointing to an unreachable part of memory. Bad.
  // So, we use the static keyword: this forces the variable to be stored in SRAM, not on the call stack.
  // An alternative thing we could do is store the variable in the heap using malloc.
  tmp[0] = *x;
  tmp[1] = *y;
  *x = tmp[1];
  *y = tmp[0];
  return tmp;
}

int main()
{
  
  /*
  unsigned volatile x; // volatile so that compiler does not optimize it away
  x = fact(0u);
  x = 2u + 3u * fact(1u);
  (void) fact(4u); // call the function without doing anything with the return value. use void to make clear that you do not care about the return value
  */
  
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
   int x = 1000000;
   int y = x/2;
   int *p = swap(&x, &y);
   
   delay(p[0]);
   
   //GPIO_PORTF_DATA_R &= ~LED_RED; // Set bit 1, turns off red LED and preserves other bits
   GPIO_PORTF_AHB_DATA_BITS_R[LED_RED] = 0;
   delay(p[1]);
   
  }
 
  //return 0;
}


 