#include "math.h"

#include <nustd/math.h>

void get_forward(float xangle, float yangle, float *x, float *y, float *z)
{
  *x = cos(yangle);
  *y = sin(xangle);
  *z = sin(yangle);
}
