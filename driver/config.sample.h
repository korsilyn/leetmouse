#include "fixedptc.h"
// Maximum number of packets allowed to be sent from the mouse at once. Linux's
// default value is 8.
// Increase this, if 'dmesg -w' tells you to!
#define BUFFER_SIZE 8

// Changes behaviour of the scroll-wheel. Default is 3.0f
#define SCROLLS_PER_TICK fixedpt_rconst(3.0)

// Main settings
#define SENSITIVITY fixedpt_rconst(0.85)
#define ACCELERATION fixedpt_rconst(0.26)
#define SENS_CAP fixedpt_rconst(4.0)
#define OFFSET fixedpt_rconst(0.0)
#define SPEED_CAP fixedpt_rconst(0.0)

// 1 is Linear, 2 is Classic, 3 is Motivity
#define ACCELERATION_MODE 1

// For exponential curves.
#define EXPONENT fixedpt_rconst(2.0)

// Midpoint for sigmoid curves
#define MIDPOINT fixedpt_rconst(1.0)
