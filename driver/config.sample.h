#include "fixedptc.h"
// Maximum number of packets allowed to be sent from the mouse at once. Linux's
// default value is 8, which at least causes EOVERFLOW for my mouse (SteelSeries
// Rival 600). Increase this, if 'dmesg -w' tells you to!
#define BUFFER_SIZE 8

/*
 * This should be your desired acceleration. It needs to end with an f.
 * For example, setting this to "0.1f" should be equal to
 * cl_mouseaccel 0.1 in Quake.
 */

// Changes behaviour of the scroll-wheel. Default is 3.0f
#define SCROLLS_PER_TICK fixedpt_rconst(3.0)

// Emulate Windows' "Enhanced Pointer Precision" for my mouse (1000 Hz) by
// approximating it with a linear accel
#define SENSITIVITY fixedpt_rconst(0.3)
#define ACCELERATION fixedpt_rconst(0.02)
#define SENS_CAP fixedpt_rconst(2.0)
#define OFFSET fixedpt_rconst(0.0)
#define SPEED_CAP fixedpt_rconst(0.0)

// 1 is Linear, 2 is Classic, 3 is Motivity
#define ACCELERATION_MODE 1

// For exponential curves.
#define EXPONENT fixedpt_rconst(2.0)

// Midpoint for sigmoid curves
#define MIDPOINT fixedpt_rconst(1.0)
