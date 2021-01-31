#include <stdint.h>

#include "TM4C123GH6PM.h"
#include "delay.h"

#define LED_RED (1u << 1)
#define LED_BLUE (1u << 2)
#define LED_GREEN (1u << 3)

/*
struct {
  uint16_t x;
  uint8_t y;
} pa, pb; // list of variables immediately after works, interestingly
*/

/*
struct Point {
  uint16_t x;
  uint8_t y;
}; // This part alone takes no storage. Can be used later to declare variables
struct Point pa, pb; // valid declaration for empty variables
*/

// cleaner way: use typedef
/*
typedef struct Point Point;
Point p1, p2; // allowed due to previous line
*/

// Even cleaner:
/*
typedef struct Point {
  uint16_t x;
  uint8_t y;
} Point;
// But struct tag names are almost never needed (except self-referential structs like nodes of trees, linkedlists).
   so we can clean this up more
*/

// Cleanest declaration
typedef struct {
  uint16_t x;
  uint8_t y;
} Point;

Point p1, p2;

typedef struct {
  Point top_left;
  Point bottom_right;
} Window;

typedef struct {
  Point corners[3];
} Triangle;

Window w, w2;
Triangle t; // Called instances

int main()
{ 
  Point *pp; // lol pp
  Window *wp;
  
  p1.x = sizeof(Point);
  p1.y = p1.x - 3u;
  
  w.top_left.x = 1u;
  w.bottom_right.y = 2u;
  
  t.corners[0].x = 1u;
  t.corners[2].y = 2u;
  
  p2 = p1;
  w2 = w; // Recall that this is a shallow copy. 
  
  pp = &p1;
  wp = &w2;
  
  pp->x = 1u;
  wp->top_left = *pp;
  
  // when using bit set idiom (OR operation), make sure datasheet permits R/W on the bits 
  SYSCTL->RCGC2 |= 1u << 5; // Set bit 5 (the sixth bit) to 1. Manages clock gating, powers on block for GPIO-F
  SYSCTL->GPIOHBCTL |= (1u << 5); // enable high performance bus
  
  GPIOF_AHB->DIR |= (LED_RED | LED_BLUE | LED_GREEN); // Set bits 1, 2, and 3 to 1. Sets LED pin as output (like DDR)
  GPIOF_AHB->DEN |= (LED_RED | LED_BLUE | LED_GREEN); // Set bits 1, 2, and 3 to 1 in digital function register (?)
  
  GPIOF_AHB->DATA_Bits[LED_BLUE] = LED_BLUE; // Set bit 1, turns on blue LED.

  while(1) {
    // interrupt safe way of changing GPIO bits
   GPIOF_AHB->DATA_Bits[LED_RED] = LED_RED; 
   delay(1000000);
   
   GPIOF_AHB->DATA_Bits[LED_RED] = 0;
   delay(500000);
   
  }
 
}


 