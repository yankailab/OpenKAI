/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#include "TagDetector.h"
#include "UnionFindSimple.h"
#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "MathUtil.h"
#include "Geometry.h"
#include "GrayModel.h"
#include "DebugImage.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef HAVE_CGAL
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/extremal_polygon_2.h>
#define NEW_QUAD_ALGORITHM (params.newQuadAlgorithm)
#else
#define NEW_QUAD_ALGORITHM (false)
#endif

#define _USE_FAST_MATH_

typedef std::vector<at::real> RealArray;
typedef std::vector<size_t> SizeArray;
typedef std::vector<uint64_t> Uint64Array;

const char* TagDetector::kDefaultDebugWindowName = "";
const bool TagDetector::kDefaultDebug = false;
const bool TagDetector::kDefaultDebugNumberFiles = false;

static const at::real kDefaultSigma = 0;
static const at::real kDefaultSegSigma = 0.8;
static const bool     kDefaultSegDecimate = false;
static const at::real kDefaultMinMag = 0.004;
static const at::real kDefaultMaxEdgeCost = 30*M_PI/180;
static const at::real kDefaultThetaThresh = 100;
static const at::real kDefaultMagThresh = 1200;
static const at::real kDefaultMinimumLineLength = 4;
static const at::real kDefaultMinimumSegmentSize = 4;
static const at::real kDefaultMinimumTagSize = 6;
static const at::real kDefaultMaxQuadAspectRatio = 32;
static const bool     kDefaultRefineQuads = false;
static const bool     kDefaultRefineBad = false;
static const bool     kDefaultNewQuadAlgorithm = false;
static const at::real kDefaultAdaptiveThresholdValue = 5;
static const int      kDefaultAdaptiveThresholdRadius = 9;

#define START_PROFILE(v, i, what) prof.start(v, i, what)
#define END_PROFILE(v, i) prof.end(v, i)

void TagDetector::reportTimers() {

  std::cout << "report averaged over " << prof.num_iterations << " frames with " << prof.num_detections << " detections (" << (double(prof.num_detections)/prof.num_iterations) << " per frame)\n\n";

  for (TimingLookup::const_iterator i=prof.timers.begin(); i!=prof.timers.end(); ++i) {
    int usec = 1e6 * i->second.run / prof.num_iterations;
    std::cout << std::setw(12) << usec << " usec";
    if (i->first.step) { std::cout << "  "; }
    if (i->first.substep) { std::cout << "  "; }
    std::cout << " - " << i->second.desc << "\n";
  }

}


//////////////////////////////////////////////////////////////////////

typedef std::vector< cv::Scalar > ScalarVec;

static const ScalarVec& getCColors() {
  static ScalarVec ccolors;
  if (ccolors.empty()) {
    ccolors.push_back(CV_RGB(  0, 255, 255));
    ccolors.push_back(CV_RGB(255,   0,   0));
    ccolors.push_back(CV_RGB(  0, 191, 255));
    ccolors.push_back(CV_RGB(255,  63,   0));
    ccolors.push_back(CV_RGB(  0, 127, 255));
    ccolors.push_back(CV_RGB(255, 127,   0));
    ccolors.push_back(CV_RGB(  0,  63, 255));
    ccolors.push_back(CV_RGB(255, 191,   0));
    ccolors.push_back(CV_RGB(  0,   0, 255));
    ccolors.push_back(CV_RGB(255, 255,   0));
    ccolors.push_back(CV_RGB( 63,   0, 255));
    ccolors.push_back(CV_RGB(191, 255,   0));
    ccolors.push_back(CV_RGB(127,   0, 255));
    ccolors.push_back(CV_RGB(127, 255,   0));
    ccolors.push_back(CV_RGB(191,   0, 255));
    ccolors.push_back(CV_RGB( 63, 255,   0));
    ccolors.push_back(CV_RGB(255,   0, 255));
    ccolors.push_back(CV_RGB(  0, 255,   0));
    ccolors.push_back(CV_RGB(255,   0, 191));
    ccolors.push_back(CV_RGB(  0, 255,  63));
    ccolors.push_back(CV_RGB(255,   0, 127));
    ccolors.push_back(CV_RGB(  0, 255, 127));
    ccolors.push_back(CV_RGB(255,   0,  63));
    ccolors.push_back(CV_RGB(  0, 255, 191));
  }
  return ccolors;
}

//////////////////////////////////////////////////////////////////////

static void countingSortLongArray(Uint64Array& v,
				  size_t vlength, 
				  int maxv, uint64_t mask) {

  if (maxv < 0) {
    for (size_t i = 0; i < vlength; i++)
      maxv = std::max(maxv, (int) (v[i]&mask));
  }

  // For weight 'w', counts[w] will give the output position for
  // the next sample with weight w.  To build this, we begin by
  // counting how many samples there are with each weight. (Note
  // that the initial position for weight w is only affected by
  // the number of weights less than w, hence the +1 in
  // counts[w+1].
  //  int counts[] = new int[maxv+2];
  SizeArray counts(maxv+2, 0);
  
  for (size_t i = 0; i < vlength; i++) {
    int w = (int) (v[i]&mask);
    counts[w+1]++;
  }
  
  // accumulate.
  for (size_t i = 1; i < counts.size(); i++) {
    counts[i] += counts[i-1];
  }
  
  //long newv[] = new long[vlength];
  Uint64Array newv(vlength);

  for (size_t i = 0; i < vlength; i++) {
    int w = (int) (v[i]&mask);
    newv[counts[w]] = v[i];
    counts[w]++;
  }
  
  /*       // test (debugging code)
           for (int i = 0; i+1 < newv.length; i++) {
           int w0 = (int) (newv[i]&mask);
           int w1 = (int) (newv[i+1]&mask);
           assert(w0 <= w1);
           }
  */
  
  newv.swap(v);

}

//////////////////////////////////////////////////////////////////////

bool detectionsOverlapTooMuch(const TagDetection& a, const TagDetection& b)
{
  // Compute a sort of "radius" of the two targets. We'll do
  // this by computing the average length of the edges of the
  // quads (in pixels).
  at::real radius = 0.0625*(pdist(a.p[0], a.p[1]) +
                          pdist(a.p[1], a.p[2]) +
                          pdist(a.p[2], a.p[3]) +
                          pdist(a.p[3], a.p[0]) +
                          pdist(b.p[0], b.p[1]) +
                          pdist(b.p[1], b.p[2]) +
                          pdist(b.p[2], b.p[3]) +
                          pdist(b.p[3], b.p[0]));
  
  // distance (in pixels) between two tag centers.
  at::real d = pdist(a.cxy, b.cxy);
  
  // reject pairs where the distance between centroids is
  // smaller than the "radius" of one of the tags.
  return (d < radius);

}



//////////////////////////////////////////////////////////////////////


TagDetectorParams::TagDetectorParams() :
  sigma(kDefaultSigma),
  segSigma(kDefaultSegSigma),
  segDecimate(kDefaultSegDecimate),
  minMag(kDefaultMinMag),
  maxEdgeCost(kDefaultMaxEdgeCost),
  thetaThresh(kDefaultThetaThresh),
  magThresh(kDefaultMagThresh),
  minimumLineLength(kDefaultMinimumLineLength),
  minimumSegmentSize(kDefaultMinimumSegmentSize),
  minimumTagSize(kDefaultMinimumTagSize),
  maxQuadAspectRatio(kDefaultMaxQuadAspectRatio),
  adaptiveThresholdValue(kDefaultAdaptiveThresholdValue),
  adaptiveThresholdRadius(kDefaultAdaptiveThresholdRadius),
  refineQuads(kDefaultRefineQuads),
  refineBad(kDefaultRefineBad),
  newQuadAlgorithm(kDefaultNewQuadAlgorithm) 
{}


TagDetector::TagDetector(const TagFamily& f,
                         const TagDetectorParams& parameters) :
    tagFamily(f),
    params(parameters),
    debug(kDefaultDebug),
    debugNumberFiles(kDefaultDebugNumberFiles),
    debugWindowName(kDefaultDebugWindowName)

{

  int dd = tagFamily.d + 2 * tagFamily.blackBorder;

  for (int i=-1; i<=dd; ++i) {
    at::real fi = at::real(i+0.5) / dd;
    for (int j=-1; j<=dd; ++j) {
      at::real fj = at::real(j+0.5) / dd;
      at::real t = -1;
      if (i == -1 || j == -1 || i == dd || j == dd) {
        t = 255; 
      } else if (i == 0 || j == 0 || i+1 == dd || j+1 == dd) {
        t = 0;
      }
      if (t >= 0) {
        tpoints.push_back(TPoint(fi, fj, t));
      }
    }
  }

}

//////////////////////////////////////////////////////////////////////
// lousy approximation of arctan function, but good enough for our
// purposes (about 4 degrees)

at::real TagDetector::arctan2(at::real y, at::real x) {

  at::real coeff_1 = at::real(M_PI/4);
  at::real coeff_2 = 3*coeff_1;
  at::real abs_y = fabs(y)+MathUtil::epsilon;      // kludge to prevent 0/0 condition
  
  at::real angle;
  
  if (x >= 0) {
    at::real r = (x - abs_y) / (x + abs_y);
    angle = coeff_1 - coeff_1 * r;
  } else {
    at::real r = (x + abs_y) / (abs_y - x);
      angle = coeff_2 - coeff_1 * r;
  }
  
  if (y < 0) {
    return -angle;     // negate if in quad III or IV
  } else {
    return angle;
  }
  
}

int TagDetector::edgeCost(at::real theta0, at::real mag0, 
			  at::real theta1, at::real mag1) const {

  if (mag0 < params.minMag || mag1 < params.minMag) {
    return -1;
  }

  at::real thetaErr = fabs(MathUtil::mod2pi(theta1 - theta0));
  if (thetaErr > params.maxEdgeCost) {
    return -1;
  }

  at::real normErr = thetaErr / params.maxEdgeCost;

  assert( int(normErr*WEIGHT_SCALE) >= 0 );
    
  return (int) (normErr * WEIGHT_SCALE);

}

cv::Mat gaussianBlur(const cv::Mat& input, at::real sigma) {

  cv::Mat output;
  cv::GaussianBlur(input, output, cv::Size(0,0), sigma);
  return output;
  
}

void emitDebugImage(const std::string& windowName,
                    int step, int substep, bool number,
                    const std::string& label,
                    const cv::Mat& img,
                    ScaleType type,
                    bool resize) {

  static int num = -1;

  if (windowName.empty()) {

    char buf[1024];
    if (number) {
      if (step == 0 && substep == 0) { ++num; }
      snprintf(buf, 1024, "debug_%04d_%d_%d.png", num, step, substep);
    } else {
      snprintf(buf, 1024, "debug_%d_%d.png", step, substep);
    }

    cv::Mat tmp = rescaleImageIntensity(img, type);
    labelImage(tmp, label);

    cv::imwrite(buf, tmp);

  } else {

    labelAndWaitForKey(windowName, label, img, type, resize);

  }

}

#ifdef HAVE_CGAL

static bool ccw(const at::Point& p1, 
                const at::Point& p2, 
                const at::Point& p3) {

  return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x) > 0;

}

#endif


void TagDetector::getQuads_MZ(const Images& images,
                              QuadArray& quads) const {

#ifndef HAVE_CGAL

  std::cerr << "*** ERROR: CAN'T GET QUADS WITH NEW ALGORITHM, COMPILED WITHOUT CGAL SUPPORT! ***\n";
  exit(1);

#else

  START_PROFILE(2, 0, "threshold image");

  cv::Mat thresh;

  cv::adaptiveThreshold(images.origBW8, thresh, 255, 
                        cv::ADAPTIVE_THRESH_MEAN_C,
                        cv::THRESH_BINARY_INV, 
                        params.adaptiveThresholdRadius, 
                        params.adaptiveThresholdValue);


  END_PROFILE(2, 0);

  if (debug) {
    emitDebugImage(debugWindowName,
                   2, 0, debugNumberFiles,
                   "Thresholded",
                   thresh,
                   ScaleNone, true);
  }


  START_PROFILE(3, 0, "find contours");

  std::vector< std::vector< cv::Point2i > > contours;
  std::vector< cv::Vec4i > hierarchy;

  cv::findContours(thresh, contours, hierarchy,
                   CV_RETR_CCOMP,
                   CV_CHAIN_APPROX_SIMPLE);

  END_PROFILE(3, 0);

  if (debug) {

    cv::Mat rgbu = images.origRGB / 2 + 127;
    int scl = resizeToDisplay(rgbu, rgbu);

    const ScalarVec& ccolors = getCColors();
    at::Point delta(0.5, 0.5);

    for (size_t i=0; i<contours.size(); ++i) {
      cv::Scalar color = ccolors[i % ccolors.size() ];
      if (hierarchy[i][3] < 0) {
        //cv::drawContours( rgbu, contours, i, color, 1, CV_AA );
        for (size_t j=0; j<contours[i].size(); ++j) {

          size_t jj = (j+1) % contours[i].size();

          at::Point cj = contours[i][j];
          at::Point cjj = contours[i][jj];

          cv::line(rgbu,
                   (cj+delta)*scl,
                   (cjj+delta)*scl,
                   color,
                   1, CV_AA);
        }
      }
    }

    emitDebugImage(debugWindowName,
                   3, 0, debugNumberFiles,
                   "Contours",
                   rgbu,
                   ScaleNone, false);


  }


  START_PROFILE(4, 0, "compute convex hulls");

  std::vector< std::vector< cv::Point2i > > hulls;
  std::vector< at::real > hareas;

  int sl = tagFamily.d + 2*tagFamily.blackBorder;
  int ta = sl*sl;
    
  for (size_t i=0; i<contours.size(); ++i) {


    if (hierarchy[i][3] < 0 && contours[i].size() >= 4) {
      std::vector<cv::Point2i> hull;
      cv::convexHull( contours[i], hull );

      at::real ca = area(&(contours[i][0]), contours[i].size());
      at::real ha = area(&(hull[0]), hull.size());

      assert( ha >= ca );

      if (ca / ha > at::real(0.8) && ha >= ta) {
        hulls.push_back(hull);
        hareas.push_back(ha);
      }

    }
  }

  END_PROFILE(4, 0);

  if (debug) {

    cv::Mat rgbu = images.origRGB / 2 + 127;
    int scl = resizeToDisplay(rgbu, rgbu);

    const ScalarVec& ccolors = getCColors();
    at::Point delta(0.5, 0.5);

    for (size_t i=0; i<hulls.size(); ++i) {
      cv::Scalar color = ccolors[i % ccolors.size() ];
      //cv::drawContours( rgbu, contours, i, color, 1, CV_AA );
      for (size_t j=0; j<hulls[i].size(); ++j) {

        size_t jj = (j+1) % hulls[i].size();

        at::Point cj = hulls[i][j];
        at::Point cjj = hulls[i][jj];

        cv::line(rgbu,
                 (cj+delta)*scl,
                 (cjj+delta)*scl,
                 color,
                 1, CV_AA);

      }
    }

    emitDebugImage(debugWindowName,
                   4, 0, debugNumberFiles,
                   "Hulls",
                   rgbu,
                   ScaleNone, false);


  }


  START_PROFILE(5, 0, "find maximum inscribed quadrilaterals");

  typedef CGAL::Simple_cartesian<int>   K;
  typedef K::Point_2                    Point;
  typedef CGAL::Polygon_2<K>            Polygon_2;
  
  for (size_t i=0; i<hulls.size(); ++i) {
    
    Polygon_2 cpoly;
    for (size_t j=0; j<hulls[i].size(); ++j) {
      cpoly.push_back( Point( hulls[i][j].x, hulls[i][j].y ) );
    }
    
    Polygon_2 k_gon;
    CGAL::maximum_area_inscribed_k_gon_2( cpoly.vertices_begin(), 
                                          cpoly.vertices_end(), 4, 
                                          std::back_inserter(k_gon));

    at::Point p[4];
    
    bool ok = true;

    for (int j=0; j<4; ++j) {
      const Point& ki = k_gon[4-j-1];
      if (ki.x() <= 1 || ki.x() >= images.orig.cols-2 || 
          ki.y() <= 1 || ki.y() >= images.orig.rows-2) {
        ok = false;
        break;
      }
      p[j] = at::Point( ki.x() + 0.5, ki.y() + 0.5 );
    }

    if (ok) {
      at::real pa = area(p, 4);
      at::real ha = hareas[i];
      assert(ha >= pa);
      if (pa / ha < at::real(0.8)) {
        ok = false;
      }
    }

    if (ok && ccw(p[0], p[1], p[2])) {
      std::swap(p[0], p[3]);
      std::swap(p[1], p[2]);
    }

    at::real observedPerimeter = 0;
    at::real dmax = 0;
    at::real dmin = images.orig.cols + images.orig.rows;

    if (ok) {

      for (int i=0; i<4; ++i) {
        int j = (i+1)%4;
        at::Point diff = p[i] - p[j];
        at::real dij = sqrt(diff.dot(diff));
        dmax = std::max(dmax, dij);
        if (dij < params.minimumTagSize) {
          ok = false;
          break;
        }
        observedPerimeter += dij;
      }

      for (int i=0; i<4; ++i) {
        for (int j=0; j<i; ++j) {
          at::Point diff = p[i] - p[j];
          at::real dij = sqrt(diff.dot(diff));
          dmin = std::min(dmin, dij);
        }
      }

    }

    if (ok && dmax / dmin < 6) {
      quads.push_back(new Quad(p, images.opticalCenter, observedPerimeter));
    }
    
  }

  END_PROFILE(5, 0);

#endif

}


const bool refine_debug = false;
const int refine_max_iter = 10;
const at::real refine_max_grad = 1e-3;

/*
void TagDetector::refineQuadTT(const Images& images,
                               Quad& quad) const {

    refineQuad( images.origBW8, 
                images.gx, images.gy, 
                quad.p, tpoints, 
                refine_debug,
                refine_max_iter,
                refine_max_grad );

    quad.recomputeHomography();

}
*/

struct DSegment {

  at::Point p;      // starting endpoint
  at::Point t;      // unit tangent vector
  at::Point n;      // unit normal vector
  at::real  length; // lenght

  DSegment() {}

  DSegment(const at::Point& p1, 
           const at::Point& p2) {

    p = p1;
    t = p2 - p1;
    length = sqrt(t.dot(t));
    t = t * (1 / length);
    n = at::Point( t.y, -t.x );

  }

  bool query(const at::Point& q, 
             at::real utol,
             at::real dmin,
             at::real dmax) {

    at::Point diff = q - p;
    at::real u = diff.dot(t);

    if (u < utol || u > length - utol) {
      return false;
    }

    at::real d = diff.dot(n);
    if (d < dmin || d > dmax) {
      return false;
    }

    return true;

  }
  
};

#define refineQuad(i, q) refineQuadLSQ(i, q)

void TagDetector::refineQuadL(const Images& images,
                              Quad& quad) const {


  int border = 3;
  at::real edge = 1;
  at::real outer = 2;
  at::real inner = 1.5;

  cv::Size sz = images.orig.size();
  cv::Rect r = boundingRect(quad.p, sz);
  dilate(r, border, sz);

  DSegment dsegs[4];
    
  for (int i=0; i<4; ++i) {
    int ii = (i+1)%4;
    dsegs[i] = DSegment(quad.p[i], quad.p[ii]);
  }

  int x1 = r.x + r.width;
  int y1 = r.y + r.height;

  XYWArray xywarrays[4];

  at::Point delta(0.5, 0.5);

  at::real imax = 0;
  at::real imin = 255;

  for (int y=r.y; y<y1; ++y) {
    for (int x=r.x; x<x1; ++x) {
      at::real ixy = images.origBW8(y,x);
      imax = std::max(imax, ixy);
      imin = std::min(imin, ixy);
    }
  }

  at::real iscl = 1/(imax-imin);
      
  for (int y=r.y; y<y1; ++y) {
    for (int x=r.x; x<x1; ++x) {
      at::Point p = at::Point(x,y) + delta;
      for (int i=0; i<4; ++i) {
        if (dsegs[i].query(p, edge, -outer, inner)) {
          at::real ixy = (images.origBW8(y,x)-imin)*iscl;
          if (ixy >= 0.7) { 
            xywarrays[i].push_back( XYW(p.x, p.y,  1) );
          } else if (ixy <= 0.3) {
            xywarrays[i].push_back( XYW(p.x, p.y, -1) );
          }
        }
      }
    }
  }

  at::Vec3 lvecs[4];

  for (int i=0; i<4; ++i) {

    const XYWArray& pts = xywarrays[i];

    at::Vec3 l(-dsegs[i].n.x, -dsegs[i].n.y,
               dsegs[i].p.dot(dsegs[i].n));

    const at::real m = 0.5;

    for (int iter=0; iter<50; ++iter) {

      at::Vec3 g(0,0,0);
      at::real loss = 0;
      
      for (size_t j=0; j<pts.size(); ++j) {
        
        at::Vec3 pj(pts[j].x, pts[j].y, 1);
        at::real cj = pts[j].w;

        at::real u = l.dot(pj) * cj;
        
        if (u < m) { 
          loss += m-u; 
          g += cj*pj;
        }
        
      }

      //std::cout << "at iter " << iter << ", loss=" << loss << ", g=" << g[0] << "," << g[1] << "," << g[2] << "\n";

      l += at::real(1e-7)*g;
      l *= 1 / sqrt( l[0]*l[0] + l[1]*l[1] );

    }

    lvecs[i] = l;

  }

  for (int i=0; i<4; ++i) {
    at::Vec3 pi = lvecs[i].cross(lvecs[(i+1)%4]);
    quad.p[i] = at::Point(pi[0]/pi[2], pi[1]/pi[2]);
    //std::cout << "p[i] = " << p[i] << ", newp[i] = " << newp[i] << "\n";
  }

  quad.recomputeHomography();

  if (debug && 0) {

    cv::Mat_<cv::Vec3b> rgbu = images.origRGB/2 + 127;
    cv::Mat_<cv::Vec3b> small(rgbu, r);
    int scl = resizeToDisplay(small, small);

    at::Point delta = at::Point(-r.x, -r.y);
    at::Point lx(0.2*scl, 0);
    at::Point ly(0, 0.2*scl);

    const ScalarVec& ccolors = getCColors();

    for (int i=0; i<4; ++i) {

      const cv::Scalar& color = ccolors[ i % ccolors.size() ];

      cv::line( small, 
                (dsegs[i].p+delta)*scl, 
                (dsegs[i].p+delta+dsegs[i].length*dsegs[i].t)*scl, 
                color, 1, CV_AA );

      cv::line( small, 
                (quad.p[i]+delta)*scl, 
                (quad.p[(i+1)%4]+delta)*scl, 
                color, 2, CV_AA );


      for (size_t j=0; j<xywarrays[i].size(); ++j) {
        const XYW& pj = xywarrays[i][j];
        at::Point p = (at::Point(pj.x, pj.y) + delta)*scl;
        cv::line( small, p-lx, p+lx, color, 1, CV_AA);
        if (pj.w > 0) {
          cv::line( small, p-ly, p+ly, color, 1, CV_AA);
        }
      }
    }

    emitDebugImage(debugWindowName, 
                   8, 0, debugNumberFiles,
                   "refineQuadL",
                   small, ScaleNone, false);
    

  }



}


void TagDetector::refineQuads(const Images& images,
                              QuadArray& quads) const {

  START_PROFILE(7,1, "refine quads");

  for (size_t i=0; i<quads.size(); ++i) {
    
    Quad& quad = *(quads[i]);

    refineQuad( images, quad );

  }

  END_PROFILE(7,1);

}

void TagDetector::getQuads_AT(const Images& images,
                              QuadArray& quads) const {


  ///////////////////////////////////////////////////////////
  // Step two. For each pixel, compute the local gradient. We
  // store the direction and magnitude.

  START_PROFILE(2, 0, "compute gradient direction and magnitude");

  // This step is quite sensitive to noise, since a few bad
  // theta estimates will break up segments, causing us to miss
  // quads. It is helpful to do a Gaussian low-pass on this step
  // even if we don't want it for decoding.

  at::Mat fimseg;

  if (params.segSigma > 0) {
    if (params.segSigma == params.sigma) {
      fimseg = images.fim;
    } else {
      cv::GaussianBlur(images.fimOrig, fimseg, cv::Size(0,0), params.segSigma);
    }
  } else {
    fimseg = images.fimOrig;
  }

  if (params.segDecimate) {
    at::Mat small(fimseg.rows/2, fimseg.cols/2);
    for (int y=0; y<small.rows; ++y) {
      for (int x=0; x<small.cols; ++x) {
        small(y,x) = 0.25 * ( fimseg(2*y+0, 2*x+0) +
                              fimseg(2*y+0, 2*x+1) +
                              fimseg(2*y+1, 2*x+0) + 
                              fimseg(2*y+1, 2*x+1) );
      }
    }
    fimseg = small;
  }

  at::Mat fimTheta( fimseg.size() );
  at::Mat fimMag( fimseg.size() );

  for (int x=0; x<fimseg.cols; ++x) {
    fimTheta(0, x) = fimTheta(fimseg.rows-1, x) =
      fimMag(0, x) = fimMag(fimseg.rows-1, x) = 0;
  }

  for (int y=0; y<fimseg.rows; ++y) {
    fimTheta(y, 0) = fimTheta(y, fimseg.cols-1) =
      fimMag(y, 0) = fimMag(y, fimseg.cols-1) = 0;
  }

  for (int y=1; y+1<fimseg.rows; ++y) {
    for (int x=1; x+1<fimseg.cols; ++x) {

      at::real Ix = fimseg(y, x+1) - fimseg(y, x-1);
      at::real Iy = fimseg(y+1, x) - fimseg(y-1, x);
        
      at::real mag = Ix*Ix + Iy*Iy;

      fimMag(y, x) = mag;

#ifdef _USE_FAST_MATH_
      at::real theta = arctan2(Iy, Ix);
#else
      at::real theta = atan2(Iy, Ix);
#endif
      
      fimTheta(y, x) = theta;

    }
  }

  END_PROFILE(2,0);

  if (debug) {

    if (params.segSigma > 0) {
      emitDebugImage(debugWindowName, 
                     2, 0, debugNumberFiles,
                     "Seg. Blur", 
                     fimseg, ScaleNone, true); 
    }

    at::real mmin =  AT_REAL_MAX;
    at::real mmax = -AT_REAL_MAX;
    for (int y=1; y+1<fimseg.rows; ++y) {
      for (int x=1; x+1<fimseg.cols; ++x) {
        at::real m = fimMag(y,x);
        mmin = std::min(mmin, m);
        mmax = std::max(mmax, m);
      }
    }

    size_t nbins = 10;
    std::vector<int> hist(nbins, 0);
    for (int y=1; y+1<fimseg.rows; ++y) {
      for (int x=1; x+1<fimseg.cols; ++x) {
        at::real m = fimMag(y,x);
        at::real f = (m-mmin)/(mmax-mmin);
        int bin = int(f*(nbins-1) + 0.5);
        ++hist[bin];
      }
    }

    std::cout << "max mag = " << mmax << ", min mag = " << mmin << "\n";

    for (size_t i=0; i<nbins; ++i) {
      at::real rmin = mmin + i*(mmax-mmin)/nbins;
      at::real rmax = mmin + (i+1)*(mmax-mmin)/nbins;
      std::cout << rmin << "-" << rmax << ": " << hist[i] << "\n";
    }

    std::cout << "\n";
    
    emitDebugImage(debugWindowName,
                   2, 1, debugNumberFiles,
                   "Theta", 
                   fimTheta, ScaleMinMax, true);
    
    emitDebugImage(debugWindowName, 
                   2, 2, debugNumberFiles,
                   "Magnitude", 
                   fimMag, ScaleMinMax, true);

  }


  ///////////////////////////////////////////////////////////
  // Step three. Segment the edges, grouping pixels with similar
  // thetas together. This is a greedy algorithm: we start with
  // the most similar pixels.  We use 4-connectivity.

  START_PROFILE(3,0, "segment");

  UnionFindSimple uf(fimseg.cols*fimseg.rows);

  if (true) {
    
    START_PROFILE(3,1, "build edges array");

    int width = fimseg.cols;
    int height = fimseg.rows;
    
    Uint64Array edges(width*height*4);

    int nedges = 0;

    // for efficiency, each edge is encoded as a single
    // long. The constants below are used to pack/unpack the
    // long.
    const uint64_t IDA_SHIFT = 40, IDB_SHIFT = 16, 
      INDEX_MASK = (1<<24) - 1, WEIGHT_MASK=(1<<16)-1;
    
    // bounds on the thetas assigned to this group. Note that
    // because theta is periodic, these are defined such that the
    // average value is contained *within* the interval.
    RealArray tmin(width*height);
    RealArray tmax(width*height);
    RealArray mmin(width*height);
    RealArray mmax(width*height);
    
    for (int y = 2; y+2 < fimseg.rows; y++) {
      for (int x = 2; x+2 < fimseg.cols; x++) {

        at::real mag0 = fimMag(y,x);

        if (mag0 < params.minMag) {
          continue;
        }

        mmax[y*width+x] = mag0;
        mmin[y*width+x] = mag0;

        at::real theta0 = fimTheta(y,x);
        tmin[y*width+x] = theta0;
        tmax[y*width+x] = theta0;

        int edgeCost;

        // RIGHT
        edgeCost = this->edgeCost(theta0, mag0, 
                                  fimTheta(y, x+1), 
                                  fimMag(y,x+1));

        if (edgeCost >= 0) {
          edges[nedges++] = 
            (uint64_t(y*width+x)<<IDA_SHIFT) + 
	    (uint64_t(y*width+x+1)<<IDB_SHIFT) + edgeCost;
        }

        // DOWN
        edgeCost = this->edgeCost(theta0, mag0, 
                                  fimTheta(y+1,x), 
                                  fimMag(y+1,x));

        if (edgeCost >= 0) {
          edges[nedges++] = 
            (uint64_t(y*width+x)<<IDA_SHIFT) + 
            (uint64_t((y+1)*width+x)<<IDB_SHIFT) + edgeCost;
        }

        // DOWN & RIGHT
        edgeCost = this->edgeCost(theta0, mag0, 
                                  fimTheta(y+1,x+1), 
                                  fimMag(y+1,x+1));

        if (edgeCost >= 0) {
          edges[nedges++] = 
            (uint64_t(y*width+x)<<IDA_SHIFT) + 
            (uint64_t((y+1)*width+x+1)<<IDB_SHIFT) + edgeCost;
        }



        // DOWN & LEFT
        edgeCost = (x == 0) ? -1 : this->edgeCost(theta0, mag0, 
                                                  fimTheta(y+1,x-1), 
                                                  fimMag(y+1,x-1));

        if (edgeCost >= 0) {
          edges[nedges++] = 
            (uint64_t(y*width+x)<<IDA_SHIFT) + 
            (uint64_t((y+1)*width+x-1)<<IDB_SHIFT) + edgeCost;
        }

        // XXX Would 8 connectivity help for rotated tags?
        // (Probably not much, so long as input filtering
        // hasn't been disabled.)
      }
    }
    
    END_PROFILE(3,1);

    START_PROFILE(3,2, "sort edges array");

    //sort those edges by weight (lowest weight first).
    countingSortLongArray(edges, nedges, -1, WEIGHT_MASK);

    END_PROFILE(3,2);

    START_PROFILE(3,3, "merge edges");

    // process edges in order of increasing weight, merging
    // clusters if we can do so without exceeding the
    // params.thetaThresh.
    for (int i = 0; i < nedges; i++) {

      int ida = (int) ((edges[i]>>IDA_SHIFT)&INDEX_MASK);
      int idb = (int) ((edges[i]>>IDB_SHIFT)&INDEX_MASK);

      ida = uf.getRepresentative(ida);
      idb = uf.getRepresentative(idb);

      if (ida == idb) {
        continue;
      }

      int sza = uf.getSetSize(ida);
      int szb = uf.getSetSize(idb);

      at::real tmina = tmin[ida], tmaxa = tmax[ida];
      at::real tminb = tmin[idb], tmaxb = tmax[idb];

      at::real costa = (tmaxa-tmina);
      at::real costb = (tmaxb-tminb);

      // bshift will be a multiple of 2pi that aligns the spans
      // of b with a so that we can properly take the union of
      // them.
      at::real bshift = MathUtil::mod2pi((tmina+tmaxa)/2, 
                                       (tminb+tmaxb)/2) - (tminb+tmaxb)/2;

      at::real tminab = std::min(tmina, tminb + bshift);
      at::real tmaxab = std::max(tmaxa, tmaxb + bshift);

      if (tmaxab - tminab > 2*M_PI) {
        // corner case that's probably not useful to handle correctly. oh well.
        tmaxab = tminab + 2*M_PI;
      }

      at::real mmaxab = std::max(mmax[ida], mmax[idb]);
      at::real mminab = std::min(mmin[ida], mmin[idb]);

      // merge these two clusters?
      at::real costab = (tmaxab - tminab);
      if (costab <= (std::min(costa, costb) + params.thetaThresh/(sza+szb)) &&
          (mmaxab-mminab) <= (std::min(mmax[ida]-mmin[ida],
                                       mmax[idb]-mmin[idb]) +
                              params.magThresh/(sza+szb))) {

        int idab = uf.connectNodes(ida, idb);

        tmin[idab] = tminab;
        tmax[idab] = tmaxab;

        mmin[idab] = mminab;
        mmax[idab] = mmaxab;

      }
    }

    END_PROFILE(3,3);

  }

  END_PROFILE(3,0);

  ///////////////////////////////////////////////////////////
  // Step four. Loop over the pixels again, collecting
  // statistics for each cluster. We will soon fit lines to
  // these points.

  START_PROFILE(4,0, "build clusters");
  
  ClusterLookup clusters;

  for (int y = 0; y+1 < fimseg.rows; y++) {
    for (int x = 0; x+1 < fimseg.cols; x++) {

      if (uf.getSetSize(y*fimseg.cols+x) < params.minimumSegmentSize) {
        continue;
      }
      
      int rep = (int) uf.getRepresentative(y*fimseg.cols + x);
      
      clusters[rep].push_back(XYW(x+0.5,y+0.5,fimMag(y,x)));

    }
  }

  END_PROFILE(4,0);

  if (debug) {

    size_t cidx = 0;
    cv::Mat_<cv::Vec3b> m = cv::Mat_<cv::Vec3b>::zeros(fimseg.size());

    const ScalarVec& ccolors = getCColors();

    for (ClusterLookup::const_iterator i=clusters.begin(); 
         i!=clusters.end(); ++i) {

      const XYWArray& xyw = i->second;

      const cv::Scalar& c = ccolors[cidx % ccolors.size()];

      for (size_t j=0; j<xyw.size(); ++j) {
        const XYW& pi = xyw[j];
        const at::real fmax = 0.5;
        at::real f = std::min(pi.w, fmax) / fmax;
        assert( pi.x >= 0 && pi.x < m.cols );
        assert( pi.y >= 0 && pi.y < m.rows );
        m(pi.y, pi.x) = cv::Vec3b(c[0]*f, c[1]*f, c[2]*f);
      }

      ++cidx;

    }

    emitDebugImage(debugWindowName, 
                   4, 0, debugNumberFiles,
                   "Clusters", 
                   m, ScaleNone, true);

  }

  ///////////////////////////////////////////////////////////
  // Step five. Loop over the clusters, fitting lines (which we
  // call Segments).

  START_PROFILE(5,0, "fit lines to clusters");

  SegmentArray segments;

  for (ClusterLookup::const_iterator i = clusters.begin();
       i != clusters.end(); ++i) {

    //GLineSegment2D gseg = GLineSegment2D.lsqFitXYW(points);
    GLineSegment2D gseg = lsqFitXYW(i->second);

    // filter short lines
    at::real length = gseg.length();

    if (length < params.minimumLineLength) {
      continue;
    }
    
    Segment* seg = new Segment();
    at::real dy = gseg.p2.y - gseg.p1.y;
    at::real dx = gseg.p2.x - gseg.p1.x;

#ifdef _USE_FAST_MATH_
    seg->theta = MathUtil::atan2(dy, dx);
#else
    seg->theta = atan2(dy, dx);
#endif

    seg->length = length;

    // We add an extra semantic to segments: the vector
    // p1->p2 will have dark on the left, white on the right.
    // To do this, we'll look at every gradient and each one
    // will vote for which way they think the gradient should
    // go. (This is way more retentive than necessary: we
    // could probably sample just one point!)
    at::real flip = 0, noflip = 0;

    for (size_t j=0; j<i->second.size(); ++j) {

      const at::Point& cs = i->second[j].point();

      at::real theta = fimTheta(cs.y, cs.x);
      at::real mag = fimMag(cs.y, cs.x);

      // err *should* be +Math.PI/2 for the correct winding,
      // but if we've got the wrong winding, it'll be around
      // -Math.PI/2.
      at::real err = MathUtil::mod2pi(theta - seg->theta);

      if (err < 0) {
        noflip += mag;
      } else {
        flip += mag;
      }

    }

    if (flip > noflip) {
      seg->theta += M_PI;
    }

    at::real dot = dx*cos(seg->theta) + dy*sin(seg->theta);

    if (dot > 0) {
      seg->x0 = gseg.p2.x; seg->y0 = gseg.p2.y;
      seg->x1 = gseg.p1.x; seg->y1 = gseg.p1.y;
    } else {
      seg->x0 = gseg.p1.x; seg->y0 = gseg.p1.y;
      seg->x1 = gseg.p2.x; seg->y1 = gseg.p2.y;
    }

    if (params.segDecimate) {
      seg->x0 = 2*seg->x0;
      seg->y0 = 2*seg->y0;
      seg->x1 = 2*seg->x1;
      seg->y1 = 2*seg->y1;
      seg->length *= 2;
    }

    segments.push_back(seg);

  }

  END_PROFILE(5,0);

  if (debug) {
    cv::Mat rgbu = images.origRGB / 2 + 127;
    int scl = resizeToDisplay(rgbu, rgbu);
    const ScalarVec& ccolors = getCColors();
    for (size_t i=0; i<segments.size(); ++i) {
      const Segment* seg = segments[i];
      const cv::Scalar& color = ccolors[ i % ccolors.size() ];
      cv::line( rgbu, 
                scl*at::Point(seg->x0, seg->y0),
                scl*at::Point(seg->x1, seg->y1),
                color, 1, CV_AA );
    }
    emitDebugImage(debugWindowName, 
                   5, 0, debugNumberFiles,
                   "Segmented", 
                   rgbu, ScaleNone, false);
  }

  
  ////////////////////////////////////////////////////////////////
  // Step six. For each segment, find segments that begin where
  // this segment ends. (We will chain segments together
  // next...) The gridder accelerates the search by building
  // (essentially) a 2D hash table.

  START_PROFILE(6,0, "find children");

  int width = images.fim.cols, height = images.fim.rows;

  Gridder gridder(0, 0, width, height, 10);
  for (size_t i=0; i<segments.size(); ++i) {
    Segment* seg = segments[i];
    gridder.add(seg->x0, seg->y0, seg);
  }

  SegmentArray findResults;

  for (size_t i=0; i<segments.size(); ++i) {

    Segment* parent = segments[i];

    gridder.find(parent->x1, parent->y1, 0.5*parent->length, findResults);

    for (size_t j=0; j<findResults.size(); ++j) {
      
      Segment* child = findResults[j];

      if (MathUtil::mod2pi(child->theta - parent->theta) > 0) {
        continue;
      }

      at::Point p;
      if (!intersect(parent, child, p)) {
        continue;
      }

      at::real parentDist = pdist(p, parent->x1, parent->y1);
      at::real childDist = pdist(p, child->x0, child->y0);
      
      if (std::max(parentDist, childDist) > parent->length) {
        continue;
      }

      // everything's okay, this child is a reasonable successor.
      parent->children.push_back(child);


    }
  }

  END_PROFILE(6,0);

  ////////////////////////////////////////////////////////////////
  // Step seven. Search all connected segments to see if any
  // form a loop of length 4. Add those to the quads list.

  START_PROFILE(7,0, "build quads");
  
  Segment* path[5];

  for (size_t i=0; i<segments.size(); ++i) {
    Segment* seg = segments[i];
    path[0] = seg;
    search(images.opticalCenter, quads, path, seg, 0);
  }

  while (!segments.empty()) {
    delete segments.back();
    segments.pop_back();
  }

  END_PROFILE(7,0);

}

void TagDetector::makeImages(const cv::Mat& orig, 
                             const at::Point& opticalCenter,
                             Images& images) const {

  images.orig = orig;

  images.opticalCenter = opticalCenter;

  if (debug) { 
    emitDebugImage(debugWindowName, 
                   0, 0, debugNumberFiles,
                   "Orig", 
                   orig, ScaleNone, true); 
  }

  // This is a very long function, but it can't really be
  // factored any more simply: it's just a long sequence of
  // sequential operations.

  ///////////////////////////////////////////////////////////
  // Step one. Preprocess image (convert to float (grayscale)
  // and low pass if necessary.)

  START_PROFILE(1, 0, "preprocess images");

  START_PROFILE(1, 1, "convert to grayscale");

  if (orig.channels() == 1) {
    images.origBW = orig;
  } else {
    cv::cvtColor(orig, images.origBW, cv::COLOR_RGB2GRAY);
  }

  END_PROFILE(1, 1);

  if (NEW_QUAD_ALGORITHM || params.refineQuads || params.refineBad) {

    START_PROFILE(1, 2, "convert to 8-bit");
    
    if (images.origBW.depth() == CV_8U) {
      images.origBW8 = images.origBW;
    } else {
      images.origBW.convertTo(images.origBW8, CV_8U, 255);
    }

    END_PROFILE(1, 2);

      /*

    START_PROFILE(1, 3, "compute image gradients");

    if (params.refineQuads || params.refineBad) {
      
      cv::Sobel( images.origBW8, images.gx, at::IMAGE_TYPE, 1, 0 );
      cv::Sobel( images.origBW8, images.gy, at::IMAGE_TYPE, 0, 1 );
      
      images.gx *= 0.25;
      images.gy *= 0.25;

    }

    END_PROFILE(1, 3);
    
      */

  }

  if (!NEW_QUAD_ALGORITHM) {

    START_PROFILE(1, 4, "convert to floating point");

    if (orig.depth() != at::REAL_IMAGE_TYPE) {
      images.origBW.convertTo(images.fimOrig, at::REAL_IMAGE_TYPE, 1.0/255);
    } else {
      images.fimOrig = images.origBW;
    }

    END_PROFILE(1, 4);

    START_PROFILE(1, 5, "blur floating point");

    if (params.sigma > 0) {
      cv::GaussianBlur(images.fimOrig, images.fim, cv::Size(0,0), params.sigma);
    } else {
      images.fim = images.fimOrig;
    }

    END_PROFILE(1, 5);

  }

  END_PROFILE(1,0);

  if (debug) {

    if (!NEW_QUAD_ALGORITHM && params.sigma > 0) {
      emitDebugImage(debugWindowName, 
                     1, 0, debugNumberFiles,
                     "Blur", 
                     images.fim, ScaleNone, true); 
    }

    cv::Mat rgb;
    if (images.orig.channels() == 3) {
      rgb = images.orig;
    } else {
      cv::cvtColor(images.orig, rgb, cv::COLOR_GRAY2RGB);
    }
    if (rgb.depth() != CV_8U) {
      at::real scl = 1;
      if (rgb.depth() == CV_32F || rgb.depth() == CV_64F) {
        scl = 255;
      }
      rgb.convertTo(images.origRGB, scl);
    } else {
      images.origRGB = rgb;
    }

  }
  

}

void TagDetector::process(const cv::Mat& orig,
                          const at::Point& opticalCenter,
                          TagDetectionArray& detections) const {

  
  if (params.newQuadAlgorithm && !NEW_QUAD_ALGORITHM) {
    std::cerr << "*** WARNING: NEW QUAD ALGORITHM REQUESTED, BUT COMPILED WITHOUT CGAL SUPPORT, FALLING BACK TO OLD ALGORITHM. ***\n";
  }

  START_PROFILE(0,0, "overall time");

  Images images;

  makeImages(orig, opticalCenter, images);

  QuadArray quads;


  if (NEW_QUAD_ALGORITHM) {
    getQuads_MZ(images, quads);
  } else {
    getQuads_AT(images, quads);
  }

  if (debug) { debugShowQuads(images, quads, 7, "Quads"); }

  if (params.refineQuads) {
    refineQuads(images, quads);
    if (debug) { debugShowQuads(images, quads, 8, "Refined quads"); }
  } 

  decode(images, quads, detections);

  while (!quads.empty()) {
    delete quads.back();
    quads.pop_back();
  }

  END_PROFILE(0,0);

}

void TagDetector::sampleGradient(const Images& images, 
                                 int x, int y,
                                 at::real& gx, at::real& gy) {

  /*
  gx = images.gx(y,x);
  gy = images.gy(y,x);
  */

  at::real i00 = bsample(images.origBW8, x-1, y-1);
  at::real i10 = bsample(images.origBW8,   x, y-1);
  at::real i20 = bsample(images.origBW8, x+1, y-1);

  at::real i01 = bsample(images.origBW8, x-1,   y);
  at::real i21 = bsample(images.origBW8, x+1,   y);

  at::real i02 = bsample(images.origBW8, x-1, y+1);
  at::real i12 = bsample(images.origBW8,   x, y+1);
  at::real i22 = bsample(images.origBW8, x+1, y+1);

  gx = at::real(0.25/255)*(-i00 - 2*i01 - i02 + i20 + 2*i21 + i22);
  gy = at::real(0.25/255)*(-i00 - 2*i10 - i20 + i02 + 2*i12 + i22);

}


void TagDetector::refineQuadLSQ(const Images& images,
                                Quad& quad) const {


  at::real wb = tagFamily.whiteBorder;
  at::real bb = tagFamily.blackBorder;
  at::real tag_side = tagFamily.d + 2*bb;
  at::real tag_area = tag_side * tag_side;

  at::real quad_area = quad.area();

  at::real zoom = sqrt(quad_area / tag_area);

  at::real outer = std::min(zoom*wb, at::real(5));
  at::real inner = std::max(at::real(0.75)*zoom*bb, at::real(0.75));
  at::real edge  = std::max(at::real(0.5)*zoom*bb, at::real(0.5));

  int border = outer+at::real(0.5);

  cv::Size sz = images.orig.size();
  cv::Rect r = boundingRect(quad.p, sz);
  dilate(r, border, sz);

  DSegment dsegs[4];

  for (int i=0; i<4; ++i) {
    int ii = (i+1)%4;
    dsegs[i] = DSegment(quad.p[i], quad.p[ii]);
  }

  int x1 = r.x + r.width;
  int y1 = r.y + r.height;

  XYWArray xywarrays[4];

  at::Point delta(0.5, 0.5);

  for (int y=r.y; y<y1; ++y) {
    for (int x=r.x; x<x1; ++x) {
      at::Point p = at::Point(x,y) + delta;
      at::real gx, gy;
      sampleGradient(images, x, y, gx, gy);
      at::real gm = gx*gx + gy*gy;
      for (int i=0; i<4; ++i) {
        if (dsegs[i].query(p, edge, -outer, inner)) {
          xywarrays[i].push_back( XYW(p.x, p.y, gm) );
        }
      }
    }
  }

  if (debug && 0) {

    cv::Mat_<cv::Vec3b> rgbu = images.origRGB/2 + 127;

    cv::Rect rect = boundingRect(quad.p, rgbu.size());
    cv::Mat_<cv::Vec3b> small = subimageWithBorder(rgbu, rect, border);

    at::Point delta = at::Point(border-rect.x, border-rect.y);
    
    for (int i=0; i<4; ++i) {

      const ScalarVec& ccolors = getCColors();
      const cv::Scalar& color = ccolors[i];
      const cv::Vec3b vc(color[0], color[1], color[2]);      

      std::cout << "zoom = " << zoom << ", outer = " << outer << " inner = " << inner << "\n";

      for (size_t j=0; j<xywarrays[i].size(); ++j) {
        const XYW& pj = xywarrays[i][j];
        at::real f = pj.w;
        if (f > 1) { f = 1; }
        int x = pj.x + delta.x;
        int y = pj.y + delta.y;
        if (x >= 0 && x < small.cols &&
            y >= 0 && y < small.rows) {
          small(y, x) = (1-f)*small(y, x) + f*vc;
        }
      }
        
    }
    
    emitDebugImage(debugWindowName,
                   7, 0, debugNumberFiles,
                   "Quad clusters",
                   small, ScaleNone, true);

  }
  
  GLineSegment2D lines[4];

  for (int i=0; i<4; ++i) {
    lines[i] = lsqFitXYW(xywarrays[i]);
  }

  at::Point pnew[4];
  bool ok = true;

  for (int i=0; i<4; ++i) {
    if (!intersect(lines[i], lines[(i+1)%4], pnew[i])) {
      ok = false;
      break;
    }
  }

  if (ok) {
    for (int i=0; i<4; ++i) {
      quad.p[i] = pnew[i];
    }
    quad.recomputeHomography();
  }
  

}

void TagDetector::debugShowQuads(const Images& images,
                                 const QuadArray& quads,
                                 int step, const std::string& label) const {

  if (1) {

    cv::Mat rgbu = images.origRGB / 2 + 127;

    int scl = resizeToDisplay(rgbu, rgbu);

    const ScalarVec& ccolors = getCColors();

    for (size_t i=0; i<quads.size(); ++i) {
      const Quad& quad = *(quads[i]);
      cv::Scalar color = ccolors[ i % ccolors.size() ];
      for (int j=0; j<4; ++j) {
        cv::line( rgbu, scl*quad.p[j], scl*quad.p[(j+1)%4], color, 1, CV_AA);
      }
    }

    emitDebugImage(debugWindowName,
                   step, 0, debugNumberFiles,
                   label,
                   rgbu,
                   ScaleNone, false);

  } else {
  

    for (size_t i=0; i<quads.size(); ++i) {

      const Quad& quad = *(quads[i]);

      cv::Mat_<cv::Vec3b> rgbu = images.origRGB * 0.5 + 127;

      cv::Rect rect = boundingRect(quad.p, rgbu.size());
      int border = 3;

      cv::Mat small = subimageWithBorder(rgbu, rect, border);

      at::Point delta = at::Point(border-rect.x, border-rect.y);

      cv::Mat big;
      at::real scl = resizeToDisplay(small, big);

      for (int j=0; j<4; ++j) {
        cv::line(big, (quad.p[j]+delta)*scl, (quad.p[(j+1)%4]+delta)*scl, 
                 CV_RGB(255,0,0), 1, CV_AA);
      }

      emitDebugImage(debugWindowName,
                     step, i, debugNumberFiles,
                     "Quad",
                     big,
                     ScaleNone, false);

    }

  }


}

void TagDetector::decode(const Images& images, 
                         const QuadArray& quads,
                         TagDetectionArray& detections) const {

  ////////////////////////////////////////////////////////////////
  // Step eight. Decode the quads. For each quad, we first
  // estimate a threshold color to decided between 0 and
  // 1. Then, we read off the bits and see if they make sense.

  START_PROFILE(8, 0, "decode quads");

  detections.clear();

  for (size_t i=0; i<quads.size(); ++i) {

    Quad& quad = *(quads[i]);

    bool good = decodeQuad(images, quad, i, detections);

    if (!good && params.refineBad && !params.refineQuads) {

      START_PROFILE(8, 1, "refine bad");

      refineQuad( images, quad );
      
      END_PROFILE(8, 1);

      decodeQuad(images, quad, i, detections);
      
    }

  }

  END_PROFILE(8,0);

  ////////////////////////////////////////////////////////////////
  // Step nine. Some quads may be detected more than once, due
  // to partial occlusion and our aggressive attempts to recover
  // from broken lines. When two quads (with the same id)
  // overlap, we will keep the one with the lowest error, and if
  // the error is the same, the one with the greatest observed
  // perimeter.

  START_PROFILE(9,0, "remove overlaps");

  TagDetectionArray goodDetections;

  // NOTE: allow multiple (non-overlapping) detections of the same target.
  for (size_t i=0; i<detections.size(); ++i) {

    const TagDetection& d = detections[i];

    bool newFeature = true;

    for (size_t odidx = 0; odidx < goodDetections.size(); odidx++) {

      const TagDetection& od = goodDetections[odidx];

      if (d.id != od.id || !detectionsOverlapTooMuch(d, od)) {
        continue;
      }

      // there's a conflict. we must pick one to keep.
      newFeature = false;

      // this detection is worse than the previous one... just don't use it.
      if (d.hammingDistance > od.hammingDistance) {
        continue;
      }

      // otherwise, keep the new one if it either has
      // *lower* error, or has greater perimeter
      if (d.hammingDistance < od.hammingDistance || 
          d.observedPerimeter > od.observedPerimeter) {
        goodDetections[odidx] = d;
      }

    }

    if (newFeature) {
      goodDetections.push_back(d);
      ++prof.num_detections;
    }

  }

  goodDetections.swap(detections);

  END_PROFILE(9,0);

}

bool TagDetector::decodeQuad(const Images& images, 
                             const Quad& quad, size_t i,
                             TagDetectionArray& detections) const {

  int width = images.orig.cols, height = images.orig.rows;

  GrayModel blackModel, whiteModel;

  // sample points around the black and white border in
  // order to calibrate our gray threshold. This code is
  // simpler if we loop over the whole rectangle and discard
  // the points we don't want.
  int dd = 2*tagFamily.blackBorder + tagFamily.d;

  std::vector< cv::Point > bpoints, wpoints;
  at::real gscl = 1.0/255;

  for (int iy = -1; iy <= dd; iy++) {
    for (int ix = -1; ix <= dd; ix++) {

      at::real y = (iy + .5) / dd;
      at::real x = (ix + .5) / dd;

      at::Point pxy = interpolate(quad.p, at::Point(x,y));

      at::real v = NEW_QUAD_ALGORITHM ?
        bicubicInterpolate( images.origBW8, pxy )*gscl :
        bicubicInterpolate( images.fim, pxy );

      if (pxy.x < 0 || pxy.x >= width || pxy.y < 0 || pxy.y >= height) {
        continue;
      }
        
      if ((iy == -1 || iy == dd) || (ix == -1 || ix == dd)) {
        // part of the outer white border.
        whiteModel.addObservation(x, y, v);
        if (debug) { wpoints.push_back(pxy); }
      } else if ((iy == 0 || iy == (dd-1)) || (ix == 0 || ix == (dd-1))) {
        // part of the outer black border.
        blackModel.addObservation(x, y, v);
        if (debug) { bpoints.push_back(pxy); }
      }

    }
  }

  if (debug) {
      
    GrayModel* models[2];
    models[0] = &whiteModel;
    models[1] = &blackModel;

    const char* names[2] = { "white", "black" };

    for (int m=0; m<2; ++m) {
      GrayModel* model = models[m];
      model->compute();
      at::Mat Amat(4, 4, &(model->A[0][0]));
      at::Mat bvec(4, 1, model->b);
      at::Mat xvec(4, 1, model->X);
      std::cout << "for quad " << i << ", model " << names[m] << ":\n";
      std::cout << "  A =\n" << Amat << "\n";
      std::cout << "  b = " << bvec << "\n";
      std::cout << "  x = " << xvec << "\n";
    }



  }

  bool bad = false;
  TagFamily::code_t tagCode = 0;

  if (debug) { std::cout << "\n"; }

  // Try reading off the bits.
  // XXX: todo: multiple samples within each cell and vote?
  for (at::uint iy = tagFamily.d-1; iy < tagFamily.d; iy--) {

    if (debug) { std::cout << "  "; }

    for (at::uint ix = 0; ix < tagFamily.d; ix++) {

      at::real y = (tagFamily.blackBorder + iy + .5) / dd;
      at::real x = (tagFamily.blackBorder + ix + .5) / dd;

      at::Point pxy = interpolate(quad.p, at::Point(x,y));

      at::real v = NEW_QUAD_ALGORITHM ?
        bicubicInterpolate( images.origBW8, pxy ) * gscl :
        bicubicInterpolate( images.fim, pxy );

      if (pxy.x < 0 || pxy.x >= width || pxy.y < 0 || pxy.y >= height) {
        if (debug) { std::cout << "quad " << i << " was bad!\n"; }
        bad = true;
        continue;
      }

      at::real threshold = (blackModel.interpolate(x, y) + 
                            whiteModel.interpolate(x, y))*.5;

      tagCode = tagCode << TagFamily::code_t(1);

      if (v > threshold) {
        tagCode |= TagFamily::code_t(1);
        if (debug) { wpoints.push_back(pxy); }
      } else {
        if (debug) { bpoints.push_back(pxy); }
      }

      if (debug) {
        std::cout << ((v > threshold) ? "##" : "  ");
      }

    }

    if (debug) { std::cout << "\n"; }

  }


  if (debug) { std::cout << "\n"; }

  if (!bad) {


    TagDetection d;
    tagFamily.decode(d, tagCode);

    if (debug) {

      std::cout << "for quad " << i << " got tagCode " << tagCode << "\n";
      std::cout << "closest tag is " << d.code << " and good is " << d.good << "\n";

      cv::Mat rgbu = images.origRGB / 2 + 127;

      cv::Point delta(0,0);

        
      for (size_t k=0; k<bpoints.size(); ++k) {
        cv::Point pk = bpoints[k];
        cv::rectangle(rgbu,
                      pk - delta, pk + delta,
                      CV_RGB(0,0,0), 1, 4);
          
      }

      for (size_t k=0; k<wpoints.size(); ++k) {
        cv::Point pk = wpoints[k];
        cv::rectangle(rgbu,
                      pk - delta, pk + delta,
                      CV_RGB(255,255,255), 1, 4);
      }

      if (0) {
        emitDebugImage(debugWindowName,
                       8, i, debugNumberFiles,
                       "Decode",
                       rgbu,
                       ScaleNone, true);
      }

    }


    // rotate points in detection according to decoded
    // orientation. Thus the order of the points in the
    // detection object can be used to determine the
    // orientation of the target.

    for (int i = 0; i < 4; i++) {
      d.p[(4+i-d.rotation)%4] = quad.p[i];
    }

    // compute the homography (and rotate it appropriately)
    d.homography = quad.H;
    d.hxy = quad.opticalCenter;

    if (true) {
      at::real c = cos(d.rotation*M_PI/2.0);
      at::real s = sin(d.rotation*M_PI/2.0);
      at::real R[9] = { 
        c, -s, 0, 
        s,  c, 0, 
        0,  0, 1 
      };
      at::Mat Rmat(3, 3, R);
      d.homography = d.homography * Rmat;
    }

    if (d.good) {
      d.cxy = interpolate(quad.p, at::Point(0.5, 0.5));
      d.observedPerimeter = quad.observedPerimeter;
      detections.push_back(d);
      return true;
    }

  }

  return false;

}

void TagDetector::search(const at::Point& opticalCenter,
                         QuadArray& quads, 
                         Segment* path[5],
                         Segment* parent, 
                         int depth) const {

  if (depth == 4) {

    if (path[4] != path[0]) {
      return;
    }

    at::Point p[4];
    at::real observedPerimeter = 0;

    for (int i=0; i<4; ++i) {
      at::Point pinter;
      if (!intersect(path[i], path[i+1], pinter)) {
        return;
      }
      p[i] = pinter;
      observedPerimeter += path[i]->length;
    }

    // eliminate quads that don't form a simply connected
    // loop (i.e., those that form an hour glass, or wind
    // the wrong way.)

#ifdef _USE_FAST_MATH_
    at::real t0 = MathUtil::atan2(p[1].y - p[0].y, p[1].x - p[0].x);
    at::real t1 = MathUtil::atan2(p[2].y - p[1].y, p[2].x - p[1].x);
    at::real t2 = MathUtil::atan2(p[3].y - p[2].y, p[3].x - p[2].x);
    at::real t3 = MathUtil::atan2(p[0].y - p[3].y, p[0].x - p[3].x);
#else
    at::real t0 = atan2(p[1].y - p[0].y, p[1].x - p[0].x);
    at::real t1 = atan2(p[2].y - p[1].y, p[2].x - p[1].x);
    at::real t2 = atan2(p[3].y - p[2].y, p[3].x - p[2].x);
    at::real t3 = atan2(p[0].y - p[3].y, p[0].x - p[3].x);
#endif
      
    at::real ttheta = ( MathUtil::mod2pi(t1-t0) + MathUtil::mod2pi(t2-t1) +
                      MathUtil::mod2pi(t3-t2) + MathUtil::mod2pi(t0-t3) );
        
    // the magic value is -2*PI. It should be exact,
    // but we allow for (lots of) numeric imprecision.
    if (ttheta < -7 || ttheta > -5) {
      return;
    }

    at::real d0 = pdist(p[0], p[1]);
    at::real d1 = pdist(p[1], p[2]);
    at::real d2 = pdist(p[2], p[3]);
    at::real d3 = pdist(p[3], p[0]);
    at::real d4 = pdist(p[0], p[2]);
    at::real d5 = pdist(p[1], p[3]);
      
    // check sizes
    if (d0 < params.minimumTagSize || d1 < params.minimumTagSize ||
        d2 < params.minimumTagSize || d3 < params.minimumTagSize ||
        d4 < params.minimumTagSize || d5 < params.minimumTagSize) {
      return;
    }
    
    // check aspect ratio
    at::real dmax = std::max(std::max(d0, d1), std::max(d2, d3));
    at::real dmin = std::min(std::min(d0, d1), std::min(d2, d3));
    
    if (dmax > dmin * params.maxQuadAspectRatio) {
      return;
    }
    
    Quad* q = new Quad(p, opticalCenter, observedPerimeter);
    quads.push_back(q);
     
  } else {   

    // Not terminal depth. Recurse on any children that obey the correct handedness.

    for (size_t i=0; i<parent->children.size(); ++i) {

      Segment* child = parent->children[i];
      // (handedness was checked when we created the children)
      
      // we could rediscover each quad 4 times (starting from
      // each corner). If we had an arbitrary ordering over
      // points, we can eliminate the redundant detections by
      // requiring that the first corner have the lowest
      // value. We're arbitrarily going to use theta...
      if (child->theta > path[0]->theta) {
        continue;
      }
      
      path[depth+1] = child;
      search(opticalCenter, quads, path, child, depth + 1);

    }

  }


}


/*
void TagDetector::test() {

  LongArray l;
  long maxv = 0;
  long mask = (1<<16)-1;

  for (int i=0; i<100; ++i) {
    l.push_back(rand() % 12345);
    maxv = std::max(maxv, l.back());
  }

  std::cout << "before: [ ";
  for (int i=0; i<100; ++i) {
    std::cout << l[i] << " ";
  }
  std::cout << "]\n";

  countingSortLongArray(l, l.size(), maxv, mask);

  size_t prev = 0;
  std::cout << "after: [ ";
  for (int i=0; i<100; ++i) {
    if (l[i] < prev) {
      std::cerr << "*** CRAP ***";
    }
    prev = l[i];
    std::cout << l[i] << " ";
  }
  std::cout << "]\n";
  



}
*/
