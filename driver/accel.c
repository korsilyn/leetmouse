// SPDX-License-Identifier: GPL-2.0-or-later

#include "accel.h"
#include "config.h"
#include "util.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h> //strlen
#include <linux/time.h>
#include <linux/init.h>
#include "fixedptc.h"

// Needed for kernel_fpu_begin/end
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
// Pre Kernel 5.0.0
#include <asm/i387.h>
#else
#include <asm/fpu/api.h>
#endif

MODULE_AUTHOR(
    "Christopher Williams <chilliams (at) gmail (dot) com>"); // Original idea
                                                              // of this module
MODULE_AUTHOR(
    "Klaus Zipfel <klaus (at) zipfel (dot) family>"); // Current maintainer of leetmouse

//Converts a preprocessor define's value in "config.h" to a string - Suspect this to change in future version without a "config.h"
#define _s(x) #x
#define s(x) _s(x)

// Convenient helper for float based parameters, which are passed via a string
// to this module (must be individually parsed via atof() - available in util.c)
#define PARAM_F(param, default, desc)              \
  fixedpt g_##param = default;			                                       \
  static char *g_param_##param = s(default);		                           \
  module_param_named(param, g_param_##param, charp, 0644);                     \
  MODULE_PARM_DESC(param, desc);

#define PARAM(param, default, desc)                                            \
  static char g_##param = default;                                             \
  module_param_named(param, g_##param, byte, 0644);                            \
  MODULE_PARM_DESC(param, desc);

// ########## Kernel module parameters

// Simple module parameters (instant update)
PARAM(no_bind, 0,
      "This will disable binding to this driver via 'leetmouse_bind' by udev.");
PARAM(update, 0, "Triggers an update of the acceleration parameters below");
PARAM(AccelerationMode, ACCELERATION_MODE,
      "Sets the algorithm to be used for acceleration");

// PARAM(AccelMode,        MODE,           "Acceleration method: 0 power law, 1:
// saturation, 2: log"); //Not yet implemented

// Acceleration parameters (type pchar. Converted to float via "update_params"
// triggered by /sys/module/leetmouse/parameters/update)
PARAM_F(SpeedCap, SPEED_CAP,
        "Limit the maximum pointer speed before applying acceleration.");
PARAM_F(Sensitivity, SENSITIVITY, "Mouse base sensitivity.");
PARAM_F(Acceleration, ACCELERATION, "Mouse acceleration sensitivity.");
PARAM_F(SensitivityCap, SENS_CAP, "Cap maximum sensitivity.");
PARAM_F(Offset, OFFSET, "Mouse base sensitivity.");
PARAM_F(Exponent, EXPONENT, "Exponent for algorithms that use it");
PARAM_F(Midpoint, MIDPOINT, "Midpoint for sigmoid function");
// PARAM_F(AngleAdjustment,XXX,            "");           //Not yet implemented.
// Douptful, if I will ever add it - Not very useful and needs me to implement
// trigonometric functions from scratch in C. PARAM_F(AngleSnapping,  XXX, "");
// //Not yet implemented. Douptful, if I will ever add it - Not very useful and
// needs me to implement trigonometric functions from scratch in C.
PARAM_F(ScrollsPerTick, SCROLLS_PER_TICK,
        "Amount of lines to scroll per scroll-wheel tick.");

void update_param(const char* str, fixedpt* result) {
	*result = *result;
}

// Updates the acceleration parameters. This is purposely done with a delay!
// First, to not hammer too much the logic in "accelerate()", which is called
// VERY OFTEN! Second, to fight possible cheating. However, this can be OFC
// changed, since we are OSS...
#define PARAM_UPDATE(param) update_param(g_param_##param, &g_##param);

static ktime_t g_next_update = 0;
INLINE void update_params(ktime_t now) {
  if (!g_update)
    return;
  if (now < g_next_update)
    return;
  g_update = 0;
  g_next_update = now + 1000000000ll; // Next update is allowed after 1s of
                                      // delay

  PARAM_UPDATE(SpeedCap);
  PARAM_UPDATE(Sensitivity);
  PARAM_UPDATE(Acceleration);
  PARAM_UPDATE(SensitivityCap);
  PARAM_UPDATE(Offset);
  PARAM_UPDATE(ScrollsPerTick);
  PARAM_UPDATE(Exponent);
  PARAM_UPDATE(Midpoint);
}

// ########## Acceleration code

// Acceleration happens here
void accelerate(int *x, int *y, int *wheel) {
  fixedpt delta_x, delta_y, delta_whl, ms, speed, accel_sens;
  // fixedpt product, motivity; // Used for mode 3
  static fixedpt carry_x = fixedpt_rconst(0.0);
  static fixedpt carry_y = fixedpt_rconst(0.0);
  static fixedpt carry_whl = fixedpt_rconst(0.0);
  static fixedpt last_ms = fixedpt_rconst(1.0);
  static ktime_t last;
  ktime_t now;
  
  accel_sens = g_Sensitivity;

  delta_x = fixedpt_fromint(*x);
  delta_y = fixedpt_fromint(*y);
  delta_whl = fixedpt_fromint(*wheel);

  // Calculate frametime
  now = ktime_get();
  ms = fixedpt_div(fixedpt_fromint(now - last), fixedpt_fromint(1000 * 1000));
  last = now;
  if (ms < fixedpt_rconst(1.0))
    ms = last_ms; // Sometimes, urbs appear bunched -> Beyond µs resolution so
                  // the timing reading is plain wrong. Fallback to last known
                  // valid frametime
  if (ms > fixedpt_rconst(100.0))
    ms = fixedpt_rconst(100.0); // Original InterAccel has 200 here. RawAccel rounds to 100. So do
              // we.
  last_ms = ms;

  // Update acceleration parameters periodically
  update_params(now);

  // Get distance traveled
  speed = fixedpt_add(fixedpt_mul(delta_x, delta_x), fixedpt_mul(delta_y, delta_y));
  speed = fixedpt_sqrt(speed);

  if (g_SpeedCap != fixedpt_rconst(0.0)) {
    if (speed >= g_SpeedCap) {
      speed = g_SpeedCap;
    }
  }

  // Calculate rate from travelled overall distance and add possible rate
  // offsets
  speed = fixedpt_div(speed, ms);
  speed = fixedpt_sub(speed, g_Offset);

  // Apply acceleration if movement is over offset
  if (speed > fixedpt_rconst(0.0)) {

    // Linear acceleration
    if (g_AccelerationMode == 1) {
      // Speed * Acceleration
      speed = fixedpt_mul(speed, g_Acceleration);
      speed = fixedpt_add(speed, fixedpt_rconst(1.0));
    }

    // Classic acceleration
    if (g_AccelerationMode == 2) {
      // (Speed * Acceleration)^Exponent
      speed = fixedpt_mul(speed, g_Acceleration);
      speed = fixedpt_add(speed, fixedpt_rconst(1.0));
	  fixedpt_pow(speed, g_Exponent);
    }

    // Motivity (Sigmoid function)
	// Not implemented for now, idk how to add exponent in fixedpt
    //if (g_AccelerationMode == 3) {
    //  // Acceleration / ( 1 + e ^ (midpoint - x))
    //  product = fixedpt_sub(g_Midpoint, speed);
    //  motivity = e;
    //  B_pow(&motivity, &product);
    //  motivity = g_Acceleration / (1 + motivity);
    //  speed = motivity;
    //}
  }

  // Apply acceleration
  delta_x = fixedpt_mul(delta_x, speed);
  delta_y = fixedpt_mul(delta_y, speed);

  // Like RawAccel, sensitivity will be a final multiplier:
  delta_x = fixedpt_mul(delta_x, g_Sensitivity);
  delta_y = fixedpt_mul(delta_y, g_Sensitivity);

  delta_x = fixedpt_add(delta_x, carry_x);
  delta_y = fixedpt_add(delta_y, carry_y);

  delta_whl = fixedpt_mul(delta_whl, fixedpt_div(g_ScrollsPerTick, fixedpt_rconst(3.0)));

  // Cast back to int
  *x = fixedpt_toint(delta_x);
  *y = fixedpt_toint(delta_y);
  *wheel = fixedpt_toint(delta_whl);

  // Save carry for next round
  carry_x = fixedpt_sub(delta_x, fixedpt_fromint(*x));
  carry_y = fixedpt_sub(delta_y, fixedpt_fromint(*y));
  carry_whl = fixedpt_sub(delta_whl, fixedpt_fromint(*wheel));
}
