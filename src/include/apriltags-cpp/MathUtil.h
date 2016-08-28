/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _MATHUTIL_H_
#define _MATHUTIL_H_

#include <math.h>
#include "AprilTypes.h"

// Fast math routines for april tags. TODO: profile to see if these
// are still necessary after portint to C++.

class MathUtil {
private:


  // only good for positive numbers.
  static at::real mod2pi_pos(at::real vin);

public:

  static const at::real epsilon;
  static const at::real twopi_inv;
  static const at::real twopi;

  static at::real fabs(at::real f);

  /** Ensure that v is [-PI, PI] **/
  static at::real mod2pi(at::real vin);

  static at::real mod2pi(at::real ref, at::real v);

  static at::real atan2(at::real y, at::real x);

  static at::real atan(at::real x);
  
  static at::real atan_mag1(at::real x);

};

#endif
