#include "delay.h"

void delay(int iter) {
    // delay loop
   int volatile counter = 0;
   while(counter < iter) counter++;
}