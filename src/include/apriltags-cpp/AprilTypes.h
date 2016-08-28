/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _APRILTYPES_H_
#define _APRILTYPES_H_

#include <opencv2/core/core.hpp>
#include <stdint.h>
#include <float.h>

// Namespace to hold data type definitions for apriltags library.
namespace at {

  typedef uint64_t code_t;
  typedef unsigned int uint;

#if 1 // Change this to zero to make everything double-precision

  typedef float real;
  enum { REAL_IMAGE_TYPE = CV_32F };
#define AT_REAL_MAX FLT_MAX
#define AT_EPSILON  0.0000001;  

#else

  typedef double real;
  enum { REAL_IMAGE_TYPE = CV_64F };
#define AT_REAL_MAX DBL_MAX
#define AT_EPSILON  0.000000001

#endif

  typedef cv::Point_<real> Point;
  typedef cv::Point3_<real> Point3;
  typedef cv::Vec<real, 3> Vec3;
  typedef cv::Mat_<real> Mat;

};

#endif
