
int main()
{
  *((unsigned int*) 0x400FE608U) = 0x20U; // Set bit 5 (the sixth bit) to 1. Manages clock gating, powers on block for GPIO-F
  *((unsigned int*) 0x40025400U) = 0x0EU; // Set bits 1, 2, and 3 to 1. Sets LED pin as output (like DDR)
  *((unsigned int*) 0x4002551CU) = 0x0EU; // Set bits 1, 2, and 3 to 1 in digital function register (?)
  
  while(1) {
   *((unsigned int*) 0x400253FCU) = 0x02U; // Set bit 1, turns on LED.
   
   int counter = 0;
   while(counter < 1000) counter++;
   
   *((unsigned int*) 0x400253FCU) = 0x00U; // Set bit 1, turns off LED.
   counter = 0;
   while(counter < 1000) counter++;
  }
 
  return 0;
}
 