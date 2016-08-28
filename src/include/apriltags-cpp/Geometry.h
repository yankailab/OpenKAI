/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

// Note: Geometry.h is a bit of a misnomer, as this holds a bunch of
// classes all used by the TagDetector class.

#include "AprilTypes.h"
#include <map>


//////////////////////////////////////////////////////////////////////
// Struct for a line segment, which holds start/end points, length,
// angle.  A segment may also have a number of children, as well as a
// sibling in the grid structure (see Gridder below).  Used by
// TagDetector to find quads.

struct Segment;
typedef std::vector<Segment*> SegmentArray;

struct Segment {
  at::real x0, y0, x1, y1, length, theta;
  SegmentArray children;
  Segment* nextGrid;
};

bool intersect(const Segment* s1, const Segment* s2, at::Point& pinter);

//////////////////////////////////////////////////////////////////////
// Class to grid up a bunch of line segments to support region-based
// search of line segments. Used by TagDetector to find quads.

class Gridder {
public:
  
  SegmentArray cells;
  at::real x0, y0, x1, y1;
  int width, height;
  at::real metersPerCell;

  Gridder(at::real x0, at::real y0, 
          at::real x1, at::real y1, 
          at::real metersPerCell);

  int sub2ind(int x, int y) const;

  void add(at::real x, at::real y, Segment* s);

  void find(at::real x, at::real y, at::real range, 
            SegmentArray& results) const;

};

//////////////////////////////////////////////////////////////////////
// Convenience functions to use Green's theorem to compute the area of
// a polygonal region using either floating-point or integer
// coordinates.

at::real area(const at::Point* p, size_t n);
at::real area(const cv::Point* p, size_t n);

//////////////////////////////////////////////////////////////////////
// Class to represent a quadrilateral inside an image, along with a
// homography mapping to/from.

class Quad {
public:

  at::Point p[4];

  at::Point opticalCenter;
  at::Mat H;

  at::real observedPerimeter;

  at::real area() const;

  Quad();

  Quad(const at::Point p[4],
       const at::Point& opticalCenter,
       at::real observedPerimeter);

  void recomputeHomography();

  at::Point interpolate(const at::Point& p) const;
  at::Point interpolate(at::real x, at::real y) const;


  at::Point interpolate01(const at::Point& p) const;
  at::Point interpolate01(at::real x, at::real y) const;

};

typedef std::vector<Quad*> QuadArray;

//////////////////////////////////////////////////////////////////////
// Utility functions to compute distances between points

at::real pdist(const at::Point& p1, const at::Point& p2);
at::real pdist(const at::Point& p, int x, int y);

//////////////////////////////////////////////////////////////////////
// Lightweight line segment class, used by TagDetector to find quad
// edges after quad detection.

class GLineSegment2D {
public:

  at::Point p1, p2;
  
  GLineSegment2D();

  GLineSegment2D(const at::Point& p1, const at::Point& p2);

  at::real length() const;

};

bool intersect(const GLineSegment2D& s1, 
               const GLineSegment2D& s2, 
               at::Point& pinter);

//////////////////////////////////////////////////////////////////////
// Class for a weighted (x,y) point, used for line segment fitting by
// TagDetector.

class XYW {
public:
  at::real x, y, w;
  XYW();
  XYW(at::real x, at::real y, at::real w);
  at::Point point() const;
};

typedef std::vector<XYW> XYWArray;
typedef std::map< int, XYWArray > ClusterLookup;

GLineSegment2D lsqFitXYW(const XYWArray& points);

#endif
