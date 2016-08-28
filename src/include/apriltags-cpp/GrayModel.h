/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _GRAYMODEL_H_
#define _GRAYMODEL_H_

#include "AprilTypes.h"

/** Fits a grayscale model over an area of the form:
 * 
 *    Ax + By + Cxy + D = value
 *
 *  We use this model to compute spatially-varying thresholds for
 *  reading bits. */

class GrayModel {
public:

  // we're solving Ax = b. For each observation, we add a row to
  // A of the form [x y xy 1] and to be of the form [gray]. x is
  // the vector [A B C D].
  //
  // The least-squares solution to the system is x = inv(A'A)A'b

  at::real A[4][4];
  at::real b[4];
  at::real X[4];

  int nobs; // how many observations?
  bool have_solution;

  GrayModel();

  void addObservation(at::real x, at::real y, at::real gray);

  int getNumObservations();

  void compute();

  at::real interpolate(at::real x, at::real y);

};

#endif
