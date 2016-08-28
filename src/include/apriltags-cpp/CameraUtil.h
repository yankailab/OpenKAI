/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _CAMERAUTIL_H_
#define _CAMERAUTIL_H_

#include "AprilTypes.h"

namespace CameraUtil {

  /** Convert a 3x3 homography matrix to a 4x4 rigid body
   *  transformation, given focal distances in X/Y, and tag size in m.
   *  This function uses the original APRIL tags algorithm, but the
   *  OpenCV-based algorithm may be more accurate.
   */
  at::Mat homographyToPose(at::real fx, at::real fy, 
                           at::real tagSize,
                           const at::Mat& horig,
                           bool openGLStyle=false);

  /** Convert a 3x3 homography matrix to a 4x4 rigid body
   *  transformation, given focal distances in X/Y, and tag size in m.
   *  This function calls the OpenCV solvePnP function under the hood
   *  to compute a rotation vector and translation vector.
   */
  void homographyToPoseCV(at::real fx, at::real fy,
                          at::real tagSize,
                          const at::Mat& horig,
                          cv::Mat& rvec,
                          cv::Mat& tvec);

};

#endif
