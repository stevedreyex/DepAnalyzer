#include <stdio.h>

void icpp() 
{
  int x, y, *p, **pp;
  p = &x;
  pp = &p;
  *pp = &y;
  *p = 3;
  **pp = *p;
}
