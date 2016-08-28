/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _TAGDETECTION_H_
#define _TAGDETECTION_H_

#include "AprilTypes.h"

class TagFamily;

/** Encapsulate the result of a tag detection. */
class TagDetection {
public:

  /** Is the detection good enough? **/
  bool good;

  /** Observed code **/
  at::code_t obsCode;

  /** Matched code **/
  at::code_t code;

  /** What was the ID of the detected tag? **/
  size_t id;

  /** The hamming distance between the detected code and the true code. **/
  at::uint hammingDistance;

  /** How many 90 degree rotations were required to align the code. **/
  int rotation;

  ////////////////// Fields below here are filled in by TagDetector ////////////////
  /** Position (in fractional pixel coordinates) of the detection. The
   * points travel around counter clockwise around the target,
   * always starting from the same corner of the tag. Dimensions
   * [4][2]. **/
  at::Point p[4];

  /** Center of tag in pixel coordinates. **/
  at::Point cxy;

  /** Measured in pixels, how long was the observed perimeter
   * (i.e., excluding inferred perimeter which is used to
   * connect incomplete quads) **/
  at::real observedPerimeter;

  /** A 3x3 homography that computes pixel coordinates from
   * tag-relative coordinates. Both the input and output coordinates
   * are 2D homogenous vectors, with y = Hx. y are pixel
   * coordinates, x are tag-relative coordinates. Tag coordinates
   * span from (-1,-1) to (1,1). The orientation of the homography
   * reflects the orientation of the target. **/
  at::Mat homography;

  /** The homography is relative to image center, whose coordinates
   * are below.
   **/
  at::Point hxy;

  at::Point interpolate(at::real x, at::real y) const {

    const at::Mat& H = homography;
    at::real z = H[2][0]*x + H[2][1]*y + H[2][2];
    return at::Point( (H[0][0]*x + H[0][1]*y + H[0][2])/z + hxy.x,
                      (H[1][0]*x + H[1][1]*y + H[1][2])/z + hxy.y );

  }

  at::Point interpolate(const at::Point& p) const {
    return interpolate(p.x, p.y);
  }
  
};

typedef std::vector<TagDetection> TagDetectionArray;

#endif
