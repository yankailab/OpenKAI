/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _TAGDETECTOR_H_
#define _TAGDETECTOR_H_

#include "Geometry.h"
#include "TagFamily.h"
#include "Refine.h"
#include "Profiler.h"

//////////////////////////////////////////////////////////////////////
// Class to hold parameters for TagDetector. 

class TagDetectorParams {
public:

  TagDetectorParams();

  at::real sigma;
  at::real segSigma;
  bool     segDecimate;
  at::real minMag;
  at::real maxEdgeCost;
  at::real thetaThresh;
  at::real magThresh;
  at::real minimumLineLength;
  at::real minimumSegmentSize;
  at::real minimumTagSize;
  at::real maxQuadAspectRatio;
  at::real adaptiveThresholdValue;
  int      adaptiveThresholdRadius;
  bool     refineQuads;
  bool     refineBad;
  bool     newQuadAlgorithm;

};


//////////////////////////////////////////////////////////////////////
// A tag detector, workhorse class of apriltags system.

class TagDetector {
public:

  enum { WEIGHT_SCALE = 100 };

  static const bool kDefaultDebug;
  static const bool kDefaultDebugNumberFiles;
  static const char* kDefaultDebugWindowName;

  // Construct with given parameters and TagFamily.
  explicit TagDetector(const TagFamily& f,
                       const TagDetectorParams& parameters =
                       TagDetectorParams());

  // Fast atan2. TODO: shove this into MathUtil.h
  static at::real arctan2(at::real y, at::real x);

  //  Compute the cost/weight of a pair of image gradients given in
  //  polar form.
  int edgeCost(at::real theta0, at::real mag0, 
	       at::real theta1, at::real mag1) const;

  // Workhorse function: find detections in a given image.
  void process(const cv::Mat& image,
               const at::Point& opticalCenter,
               TagDetectionArray& detections) const;
  
  // Dump output of profiling timer.
  void reportTimers();

  // Reference to tag family.
  const TagFamily& tagFamily;

  // Current parameters.
  TagDetectorParams params;

  // Display/save debug images?
  bool debug;

  // Should saved debug images be numbered by iteration?
  bool debugNumberFiles;
  
  // Name of window for debug images. If this is empty, will instead
  // emit files.
  std::string debugWindowName; 

  // Hold our profiling information. This should be the only local
  // variable that is modified during the process function.
  mutable Profiler prof;

private:

  struct Images {

    cv::Mat orig;
    cv::Mat_<cv::Vec3b> origRGB;
    cv::Mat origBW;
    cv::Mat_<unsigned char> origBW8;

    at::Mat fimOrig; 
    at::Mat fim;
    
    at::Point opticalCenter;

  };
  
  static void sampleGradient(const Images& images, 
                             int x, int y,
                             at::real& gx, at::real& gy);


  TPointArray tpoints;

  void makeImages(const cv::Mat& image, 
                  const at::Point& opticalCenter,
                  Images& images) const;
  
  void getQuads_AT(const Images& images,
                   QuadArray& quads) const;

  void getQuads_MZ(const Images& images,
                   QuadArray& quads) const;

  void refineQuadL(const Images& images, Quad& quad) const;

  void refineQuadTT(const Images& images, Quad& quad) const;

  void refineQuadLSQ(const Images& images, Quad& quad) const;

  void refineQuads(const Images& images,
                   QuadArray& quads) const;
  
  void debugShowQuads(const Images& images,
                      const QuadArray& quads,
                      int step, const std::string& label) const;

  void decode(const Images& images,
              const QuadArray& quads,
              TagDetectionArray& detections) const;

  bool decodeQuad(const Images& images,
                  const Quad& quad, 
                  size_t quadIndex,
                  TagDetectionArray& detections) const;

  void search(const at::Point& opticalCenter,
              QuadArray& quads, 
              Segment* path[5],
              Segment* parent, 
              int depth) const;


};



#endif
