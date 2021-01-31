
int counter = 0; // Stored in RAM

int main()
{
  int* ptr = &counter;
  while(*ptr < 21) {
    ++(*ptr);
  }
  
  ptr = (int*) 0x20000002U; // unsigned int ptr, casted to int*
  *ptr = 0xDEADBEEF;
  return 0;
}
 