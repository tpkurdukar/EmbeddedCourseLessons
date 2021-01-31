#include "delay.h"

void delay(int volatile iter) {
    // delay loop
   while(iter > 0) iter--;
}