/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _REFINE_H_
#define _REFINE_H_

// Data structures and functions for refining quads.

#include "AprilTypes.h"
#include <opencv2/imgproc/imgproc.hpp>

//////////////////////////////////////////////////////////////////////
// Template point holding (x,y) location and target
// intensity. Interior pixels (on tag border) should be black (t=0),
// exterior should be white (t=1).

class TPoint {
public:

  at::Point p;
  at::real  t;

  TPoint();

  TPoint(const at::Point& point,
         at::real target): p(point), t(target) {}

  TPoint(at::real x, at::real y,
         at::real target): p(x,y), t(target) {}

};

typedef std::vector<TPoint> TPointArray;

//////////////////////////////////////////////////////////////////////
// Utility functions to annotate images with point/gradient
// information.

void drawPoint(cv::Mat& m, const at::Point& p, 
               const cv::Scalar& color, 
               at::real sz=3, int thickness=1);

void drawArrow(cv::Mat& m, const at::Point& p, const at::Point& g, 
               const cv::Scalar& color, at::real scl=15.0/255.0);

//////////////////////////////////////////////////////////////////////

// Get the bounding rectangle of a set of 4 points.
cv::Rect boundingRect(const at::Point p[4], const cv::Size& sz);

// Dilate a rectangle, respecting image boundaries.
void dilate(cv::Rect& r, int b, const cv::Size& sz);

// Biilinear interpolation with jacobians.
at::Point interpolate(const at::Point p[4], const at::Point& uv, at::Mat* pJ=0);

// Homography (perspective) interpolation, with jacobians.
at::Point interpolateH(const at::Mat& H, const at::Point& uv, at::Mat* pJxy=0);

// Compute a homography given 4 image points.
void computeH(const at::Point p[4], at::Mat& H, at::Mat* pJh=0);

// Refine a detected quadrilateral using template matching.
int refineQuad(const cv::Mat& gimage,
               const at::Mat& gx,
               const at::Mat& gy,
               at::Point p[4],
               const TPointArray& tpoints,
               bool debug = false,
               int max_iter = 25,
               at::real max_grad = 1e-2);

//////////////////////////////////////////////////////////////////////
// Sample image using nearest neighbor, clamping image coordinates to
// image border.

template<class Tval>
inline Tval bsample(const cv::Mat_<Tval>& image, int x, int y) {
  int yy = borderInterpolate(y, image.rows, cv::BORDER_REPLICATE);
  int xx = borderInterpolate(x, image.cols, cv::BORDER_REPLICATE);
  return image(yy,xx);
}

template<class Tval>
inline Tval bsample(const cv::Mat_<Tval>& image, const cv::Point& p) {
  return bsample<Tval>(image, p.x, p.y);
}

template<class Tval>
inline Tval bsample(const cv::Mat_<Tval>& image, const at::Point& p) {
  return bsample<Tval>(image, int(p.x+0.5), int(p.y+0.5));
}

//////////////////////////////////////////////////////////////////////
// Cubic interpolation over 4 equally spaced points.

inline at::real cubicInterpolate(at::real p[4], at::real x) {
  return p[1] + at::real(0.5) * x * 
    (p[2] - p[0] + x*(at::real(2.0)*p[0] - 
                      at::real(5.0)*p[1] + 
                      at::real(4.0)*p[2] - 
                      p[3] + x*(at::real(3.0)*(p[1] - p[2]) + p[3] - p[0])));
}

//////////////////////////////////////////////////////////////////////
// Bicubic interpolation over equally spaced 4x4 patch.

inline at::real bicubicInterpolate (at::real p[4][4], at::real x, at::real y) {
  at::real arr[4];
  arr[0] = cubicInterpolate(p[0], y);
  arr[1] = cubicInterpolate(p[1], y);
  arr[2] = cubicInterpolate(p[2], y);
  arr[3] = cubicInterpolate(p[3], y);
  return cubicInterpolate(arr, x);
}

//////////////////////////////////////////////////////////////////////
// Bicubic interpolation of an image at a floating-point coordinate
// location, respecting border.

template <class Tval>
inline at::real bicubicInterpolate(const cv::Mat_<Tval>& image, 
                            const at::Point& pt) {

  at::real fx = pt.x - 0.5;
  at::real fy = pt.y - 0.5;

  int ix = fx;
  int iy = fy;

  fx -= ix;
  fy -= iy;

  at::real p[4][4];

  for (int py=0; py<4; ++py) {
    for (int px=0; px<4; ++px) {
      p[px][py] = bsample(image, ix+px-1, iy+py-1);
    }
  }

  return bicubicInterpolate(p, fx, fy);

}

//////////////////////////////////////////////////////////////////////
// Extract a rectangular subimage with a given amount of border.

template <class Tval>
inline cv::Mat_<Tval> subimageWithBorder(const cv::Mat_<Tval>& image,
                                  const cv::Rect& r,
                                  int border) {
  
  cv::Mat_<Tval> bimage(r.height + 2*border, r.width + 2*border);
  
  for (int y=-border; y<r.height+border; ++y) {
    for (int x=-border; x<r.width+border; ++x) {
      bimage(y+border, x+border) = bsample(image, r.x+x, r.y+y);
    }
  }

  return bimage;

}

#endif
