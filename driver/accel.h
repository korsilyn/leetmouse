#ifndef _ACCEL_H
#define _ACCEL_H

#include "fixedptc.h"

void accelerate(int *x, int *y, int *wheel);
void update_param(const char *str, fixedpt *result);

#endif /* _ACCEL_H */
