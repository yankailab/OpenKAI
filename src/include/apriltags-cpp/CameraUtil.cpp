/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#include "CameraUtil.h"
#include <opencv2/calib3d/calib3d.hpp>

namespace CameraUtil {

static at::real sq(at::real x) { return x*x; }

/** Given a 3x3 homography matrix and the focal lengths of the
 * camera, compute the pose of the tag. The focal lengths should
 * be given in pixels. For example, if the camera's focal length
 * is twice the width of the sensor, and the sensor is 600 pixels
 * across, the focal length in pixels is 2*600. Note that the
 * focal lengths in the fx and fy direction will be approximately
 * equal for most lenses, and is not a function of aspect ratio.
 *
 * Theory: The homography matrix is the product of the camera
 * projection matrix and the tag's pose matrix (the matrix that
 * projects points from the tag's local coordinate system to the
 * camera's coordinate frame).
 *
 * [ h00 h01 h02 h03] = [ 1/fx 0     0 0 ] [ R00 R01 R02 TX ]
 * [ h10 h11 h12 h13] = [ 0    1/fy  0 0 ] [ R10 R11 R12 TY ]
 * [ h20 h21 h22 h23] = [ 0    0     1 0 ] [ R20 R21 R22 TZ ]
 *                                         [ 0   0   0   1  ]
 *
 * When observing a tag, the points we project in world space all
 * have z=0, so we can form a 3x3 matrix by eliminating the 3rd
 * column of the pose matrix.
 *
 * [ h00 h01 h02 ] = [ 1/fx 0     0 0 ] [ R00 R01 TX ]
 * [ h10 h11 h12 ] = [ 0    1/fy  0 0 ] [ R10 R11 TY ]
 * [ h20 h21 h22 ] = [ 0    0     1 0 ] [ R20 R21 TZ ]
 *                                      [ 0   0   1  ]
 *
 * (note that these h's are different from the ones above.)
 *
 * We can multiply the right-hand side to yield a set of equations
 * relating the values of h to the values of the pose matrix.
 *
 * There are two wrinkles. The first is that the homography matrix
 * is known only up to scale. We recover the unknown scale by
 * constraining the magnitude of the first two columns of the pose
 * matrix to be 1. We use the geometric average scale. The sign of
 * the scale factor is recovered by constraining the observed tag
 * to be in front of the camera. Once scaled, we recover the first
 * two colmuns of the rotation matrix. The third column is the
 * cross product of these.
 *
 * The second wrinkle is that the computed rotation matrix might
 * not be exactly orthogonal, so we perform a polar decomposition
 * to find a good pure rotation approximation.
 *
 * Tagsize is the size of the tag in your desired units. I.e., if
 * your tag measures 0.25m along the side, your tag size is
 * 0.25. (The homography is computed in terms of *half* the tag
 * size, i.e., that a tag is 2 units wide as it spans from -1 to
 * +1, but this code makes the appropriate adjustment.)
 **/

 at::Mat homographyToPose(at::real fx, at::real fy, 
                          at::real tagSize,
                          const at::Mat& horig,
                          bool openGLStyle) {
    
   // flip the homography along the Y axis to align the
   // conventional image coordinate system (y=0 at the top) with
   // the conventional camera coordinate system (y=0 at the
   // bottom).

   at::Mat h = horig;

   at::Mat F = at::Mat::eye(3,3);
   F(1,1) = F(2,2) = -1;
   h = F * horig;

   at::Mat M(4,4);
   M(0,0) =  h(0,0) / fx;
   M(0,1) =  h(0,1) / fx;
   M(0,3) =  h(0,2) / fx;
   M(1,0) =  h(1,0) / fy;
   M(1,1) =  h(1,1) / fy;
   M(1,3) =  h(1,2) / fy;
   M(2,0) =  h(2,0);
   M(2,1) =  h(2,1);
   M(2,3) =  h(2,2);

   // Compute the scale. The columns of M should be made to be
   // unit vectors. This is over-determined, so we take the
   // geometric average.
   at::real scale0 = sqrt(sq(M(0,0)) + sq(M(1,0)) + sq(M(2,0)));
   at::real scale1 = sqrt(sq(M(0,1)) + sq(M(1,1)) + sq(M(2,1)));
   at::real scale = sqrt(scale0*scale1);
    
   M *= 1.0/scale;

   // recover sign of scale factor by noting that observations must
   // occur in front of the camera.
   if (M(2,3) > 0) {
     M *= -1;
   }

   // The bottom row should always be [0 0 0 1].  
   M(3,0) = 0;
   M(3,1) = 0;
   M(3,2) = 0;
   M(3,3) = 1;

   // recover third rotation vector by crossproduct of the other two
   // rotation vectors
   at::Vec3 a( M(0,0), M(1,0), M(2,0) );
   at::Vec3 b( M(0,1), M(1,1), M(2,1) );
   at::Vec3 ab = a.cross(b);
    
   M(0,2) = ab[0];
   M(1,2) = ab[1];
   M(2,2) = ab[2];

   // pull out just the rotation component so we can normalize it.

   at::Mat R(3,3);
   for (int i=0; i<3; ++i) {
     for (int j=0; j<3; ++j) {
       R(i,j) = M(i,j);
     }
   }

   // polar decomposition, R = (UV')(VSV')

   cv::SVD svd(R);
   at::Mat MR = svd.u * svd.vt;
   
   if (!openGLStyle) { MR = F * MR; }

   for (int i=0; i<3; ++i) {
     for (int j=0; j<3; ++j) {
       M(i,j) = MR(i,j);
     }
   }

   // Scale the results based on the scale in the homography. The
   // homography assumes that tags span from -1 to +1, i.e., that
   // they are two units wide (and tall).
   for (int i = 0; i < 3; i++) {
     at::real scl = openGLStyle ? 1 : F(i,i);
     M(i,3) *= scl * tagSize / 2;
   }
    
   return M;
    

}

at::Point project(const at::Mat& H, 
                    at::real x, at::real y) {

  at::real z = H[2][0]*x + H[2][1]*y + H[2][2];
  
  return at::Point( (H[0][0]*x + H[0][1]*y + H[0][2])/z,
                      (H[1][0]*x + H[1][1]*y + H[1][2])/z );
  

}

void homographyToPoseCV(at::real fx, at::real fy,
                        at::real tagSize,
                        const at::Mat& horig,
                        cv::Mat& rvec,
                        cv::Mat& tvec) {
  
  at::Point3 opoints[4] = {
    at::Point3(-1, -1, 0),
    at::Point3( 1, -1, 0),
    at::Point3( 1,  1, 0),
    at::Point3(-1,  1, 0)
  };

  at::Point ipoints[4];

  at::real s = 0.5*tagSize;

  for (int i=0; i<4; ++i) {
    ipoints[i] = project(horig, opoints[i].x, opoints[i].y);
    opoints[i] *= s;
  }

  at::real Kdata[9] = { 
    fx, 0, 0,
    0, fy, 0, 
    0, 0, 1
  };

  at::Mat Kmat(3, 3, Kdata);

  at::Mat dcoeffs = at::Mat::zeros(4, 1);

  cv::Mat_<at::Point3> omat(4, 1, opoints);
  cv::Mat_<at::Point> imat(4, 1, ipoints);

  cv::Mat r, t;

  cv::solvePnP(omat, imat, Kmat, dcoeffs, r, t);

  if (rvec.type() == CV_32F) {
    r.convertTo(rvec, rvec.type());
  } else {
    rvec = r;
  }

  if (tvec.type() == CV_32F) {
    t.convertTo(tvec, tvec.type());
  } else {
    tvec = t;
  }

}


}


