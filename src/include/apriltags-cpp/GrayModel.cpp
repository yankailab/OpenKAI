/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#include "GrayModel.h"
#include <opencv2/core/core.hpp>
 
GrayModel::GrayModel(): nobs(0), have_solution(false) {

  memset(A, 0, sizeof(A));
  memset(b, 0, sizeof(b));
  memset(X, 0, sizeof(X));

}


void GrayModel::addObservation(at::real x, at::real y, at::real gray) {

  at::real xy = x*y;

  // update only upper-right elements. A'A is symmetric,
  // we'll fill the other elements in later.
  A[0][0] += x*x;
  A[0][1] += x*y;
  A[0][2] += x*xy;
  A[0][3] += x;
  A[1][1] += y*y;
  A[1][2] += y*xy;
  A[1][3] += y;
  A[2][2] += xy*xy;
  A[2][3] += xy;
  A[3][3] += 1;

  b[0] += x*gray;
  b[1] += y*gray;
  b[2] += xy*gray;
  b[3] += gray;

  nobs++;
  have_solution = false;

}

int GrayModel::getNumObservations() {
  return nobs;
}

void GrayModel::compute() {

  if (have_solution) { return; }

  if (nobs >= 6) {
    // we really only need 4 linearly independent
    // observations to fit our answer, but we'll be very
    // sensitive to noise if we don't have an
    // over-determined system. Thus, require at least 6
    // observations (or we'll use a constant model below).

    // make symmetric
    for (int i = 0; i < 4; i++)
      for (int j = i+1; j < 4; j++)
        A[j][i] = A[i][j];

    at::Mat Amat(4, 4, &(A[0][0]));
    at::Mat bvec(4, 1, b);
    at::Mat xvec(4, 1, X);

    have_solution = cv::solve(Amat, bvec, xvec);

  }

  if (!have_solution) {
    // not enough samples to fit a good model. Use a flat model.
    X[0] = X[1] = X[2] = 0;
    X[3] = b[3] / nobs;
  }

}

at::real GrayModel::interpolate(at::real x, at::real y) {
  compute();
  return X[0]*x + X[1]*y + X[2]*x*y + X[3];
}


/*
  void testGrayModel() {

  at::real cx = 5;
  at::real cy = -3;
  at::real cxy = -0.1;
  at::real cb = 0.5;

  GrayModel gmodel;

  for (int y=-5; y<=5; ++y) {
  for (int x=-5; x<=5; ++x) {
  at::real g = cx*x + cy*y + cxy*x*y + cb;
  gmodel.addObservation(x, y, g);
  }
  }

  gmodel.compute();

  for (int i=0; i<4; ++i) {
  std::cout << "gmodel.X[" << i << "] = " << gmodel.X[i] << "\n";
  }

  }
*/
