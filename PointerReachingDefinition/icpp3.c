#include <stdio.h>

void icpp3() 
{
  int x, y, *p, **pp;
  p = &x;
  pp = &p;
  *pp = &y;
  *p = 3;
  **pp = 4;
}
