/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#include "Geometry.h"
#include "MathUtil.h"
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

XYW::XYW() {}
XYW::XYW(at::real xx, at::real yy, at::real ww): x(xx), y(yy), w(ww) {}  
at::Point XYW::point() const { return at::Point(x,y); }

Gridder::Gridder(at::real x0, at::real y0, at::real x1, at::real y1, at::real metersPerCell) {

    this->x0 = x0;
    this->y0 = y0;
    this->metersPerCell = metersPerCell;
    
    width = (int) ((x1 - x0)/metersPerCell + 1);
    height = (int) ((y1 - y0)/metersPerCell + 1);
    
    this->x1 = x0 + metersPerCell*width;
    this->y1 = y0 + metersPerCell*height;
    
    cells.clear();
    cells.resize(width*height);

  }

int Gridder::sub2ind(int x, int y) const {
  return y*width + x;
}

void Gridder::add(at::real x, at::real y, Segment* s) {
  
  int ix = (int) ((x - x0)/metersPerCell);
  int iy = (int) ((y - y0)/metersPerCell);
  
  if (ix >=0 && iy >=0 && ix < width && iy < height) {
    size_t idx = sub2ind(ix,iy);
    s->nextGrid = cells[idx];
    cells[idx] = s;
  }
  
}

void Gridder::find(at::real x, at::real y, at::real range, SegmentArray& results) const {

  results.clear();

  int ix0 = (int) ((x - range - x0)/metersPerCell);
  int iy0 = (int) ((y - range - y0)/metersPerCell);

  int ix1 = (int) ((x + range - x0)/metersPerCell);
  int iy1 = (int) ((y + range - y0)/metersPerCell);

  for (int iy=iy0; iy<=iy1; ++iy) {
    for (int ix=ix0; ix<=ix1; ++ix) {

      if (ix >=0 && iy >=0 && ix < width && iy < height) {
          
        for (Segment* s = cells[sub2ind(ix,iy)]; s; s = s->nextGrid) {
          results.push_back(s);
        }

      }
        
    }
  }

}

bool intersect(const GLineSegment2D& g1,
               const GLineSegment2D& g2,
               at::Point& pinter) {

  Segment s1, s2;

  s1.x0 = g1.p1.x;
  s1.x1 = g1.p2.x;

  s1.y0 = g1.p1.y;
  s1.y1 = g1.p2.y;

  s2.x0 = g2.p1.x;
  s2.x1 = g2.p2.x;

  s2.y0 = g2.p1.y;
  s2.y1 = g2.p2.y;

  return intersect(&s1, &s2, pinter);

}

bool intersect(const Segment* s1, 
               const Segment* s2, 
               at::Point& pinter) {

  at::real m00 = s1->x1-s1->x0;
  at::real m01 = s2->x0-s2->x1;
  
  at::real m10 = s1->y1-s1->y0;
  at::real m11 = s2->y0-s2->y1;

  at::real det = m00*m11 - m01*m10;
  
  if (MathUtil::fabs(det) < 0.0000000001) {
    return false;
  }

  at::real i00 = m11/det;
  //at::real i11 = m00/det;
  at::real i01 = -m01/det;
  //at::real i10 = -m10/det;

  at::real b00 = s2->x0 - s1->x0;
  at::real b10 = s2->y0 - s1->y0;

  at::real x00 = i00*b00 + i01*b10;
  pinter.x = m00*x00 + s1->x0;
  pinter.y = m10*x00 + s1->y0;

  return true;

}

at::real pdist(const at::Point& p1, const at::Point& p2) {
  at::Point pd = p2-p1;
  return sqrt(pd.dot(pd));
}

at::real pdist(const at::Point& p, int x, int y) {
  return pdist(p, at::Point(x,y));
}

GLineSegment2D::GLineSegment2D() {}

GLineSegment2D::GLineSegment2D(const at::Point& pp1, const at::Point& pp2): 
    p1(pp1), p2(pp2) {}

at::real GLineSegment2D::length() const {
    return pdist(p1, p2);
}

static at::real square(at::real x) { return x*x; }

GLineSegment2D lsqFitXYW(const XYWArray& points) {

  at::real Cxx=0, Cyy=0, Cxy=0, Ex=0, Ey=0, mXX=0, mYY=0, mXY=0, mX=0, mY=0;
  at::real n=0;
  
  int idx = 0;
  for (size_t i=0; i<points.size(); ++i) {

    const XYW& tp = points[i];
    
    at::real x = tp.x;
    at::real y = tp.y;
    at::real alpha = tp.w;
    
    mY  += y*alpha;
    mX  += x*alpha;
    mYY += y*y*alpha;
    mXX += x*x*alpha;
    mXY += x*y*alpha;
    n   += alpha;
    
    idx++;

  }
  
  Ex  = mX/n;
  Ey  = mY/n;
  Cxx = mXX/n - square(mX/n);
  Cyy = mYY/n - square(mY/n);
  Cxy = mXY/n - (mX/n)*(mY/n);
  
  // find dominant direction via SVD
  at::real phi = 0.5*atan2(-2*Cxy,(Cyy-Cxx));
  //at::real rho = Ex*cos(phi) + Ey*sin(phi);
  
  // compute line parameters
  at::real dx = -sin(phi);
  at::real dy =  cos(phi); 
  at::real px = Ex;
  at::real py = Ey;

  at::real maxcoord = -DBL_MAX;
  at::real mincoord =  DBL_MAX;

  for (size_t i=0; i<points.size(); ++i) {
    const XYW& tp = points[i];
    at::real coord = dx*(tp.x-px) + dy*(tp.y-py);
    maxcoord = std::max(coord, maxcoord);
    mincoord = std::min(coord, mincoord);
  }

  at::Point p0(px, py);
  at::Point dd(dx, dy);
  
  at::Point p1 = p0 + mincoord*dd;
  at::Point p2 = p0 + maxcoord*dd;

  return GLineSegment2D( p1, p2 );

};

at::real area(const at::Point* p, size_t n) {


  if (n < 3) { return 0; }

  at::real a = 0;
  
  for (size_t i=0; i<n; ++i) {

    const at::Point& p0 = p[i];
    const at::Point& p1 = p[(i+1)%n];

    a += p0.x * p1.y - p0.y * p1.x;

  }

  if (a<0) { a = -a; }
  a *= at::real(0.5);

  return a;

}

at::real area(const cv::Point* p, size_t n) {

  if (n < 3) { return 0; }

  at::real a = 0;
  
  for (size_t i=0; i<n; ++i) {

    const cv::Point& p0 = p[i];
    const cv::Point& p1 = p[(i+1)%n];

    a += p0.x * p1.y - p0.y * p1.x;

  }

  if (a<0) { a = -a; }
  a *= at::real(0.5);

  return a;

}

Quad::Quad() {}

at::real Quad::area() const {
  return ::area(p, 4);
}

Quad::Quad(const at::Point pp[4], const at::Point& oc, at::real op):
  opticalCenter(oc), observedPerimeter(op)
{


  for (size_t i=0; i<4; ++i) { p[i] = pp[i]; }

  recomputeHomography();

}

void Quad::recomputeHomography() {

  at::Point src[4] = {
    at::Point(-1, -1),
    at::Point( 1, -1),
    at::Point( 1,  1),
    at::Point(-1,  1),
  };

  at::Point dst[4];

  for (size_t i=0; i<4; ++i) {
    dst[i] = p[i] - opticalCenter;
  }

  cv::Mat_<at::Point> srcmat(4, 1, src);
  cv::Mat_<at::Point> dstmat(4, 1, dst);

  H = cv::findHomography(srcmat, dstmat);

}

at::Point Quad::interpolate(const at::Point& p) const {

  return interpolate(p.x, p.y);
  
}

at::Point Quad::interpolate(at::real x, at::real y) const {

  at::real z = H[2][0]*x + H[2][1]*y + H[2][2];
  
  return at::Point( (H[0][0]*x + H[0][1]*y + H[0][2])/z + opticalCenter.x,
                      (H[1][0]*x + H[1][1]*y + H[1][2])/z + opticalCenter.y );

}

at::Point Quad::interpolate01(const at::Point& p) const {
  return interpolate01(p.x, p.y);
}

at::Point Quad::interpolate01(at::real x, at::real y) const {
  return interpolate(2*x-1, 2*y-1);
}

