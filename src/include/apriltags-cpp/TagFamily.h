/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _TAGFAMILY_H_
#define _TAGFAMILY_H_

#include <vector>
#include <string>
#include "TagDetection.h"

/** Encapsulate information about a tag family. */

class TagFamily {
public:

  typedef unsigned char byte;
  typedef at::code_t code_t;
  typedef at::uint uint;

  typedef std::vector<code_t> CodeArray;
  typedef std::vector<byte> ByteArray;
  typedef std::vector<std::string> StringArray;


  /** Static list of available tag families. */
  static StringArray families();

  /** Default constructor doesn't do much of anything, call init()
      later to set up. */
  TagFamily();

  /** Construct with named family (returned by families() method). */
  TagFamily(const std::string& name);

  /** Construct with given data. */
  TagFamily(int bits, int minimumHammingDistance, 
            size_t count, const code_t* data);

  /** Initialize using the named family (returned by families() method). */
  void init(const std::string& name);

  /** How many pixels wide is the outer-most white border? This is
   * only used when rendering a tag, not for decoding a tag (since
   * we look for the white/black edge). **/
  uint whiteBorder;

  /** How many pixels wide is the inner black border? **/
  uint blackBorder;
  
  /** number of bits in the tag. Must be a square (n^2). **/
  uint bits;

  /** dimension of tag. e.g. for 16 bits, d=4. Must be sqrt(bits). **/
  uint d;

  /** What is the minimum hamming distance between any two codes
   * (accounting for rotational ambiguity? The code can recover
   * (minHammingDistance-1)/2 bit errors.
   **/
  uint minimumHammingDistance;

  /** The error recovery value determines our position on the ROC
   * curve. We will report codes that are within errorRecoveryBits
   * of a valid code. Small values mean greater rejection of bogus
   * tags (but false negatives). Large values mean aggressive
   * reporting of bad tags (but with a corresponding increase in
   * false positives).
   **/
  uint errorRecoveryBits;

  /** The array of the codes. The id for a code is its index. **/
  CodeArray codes;

  void init(int bits, 
            int minimumHammingDistance,
            size_t count,
            const code_t* data);

  void setErrorRecoveryBits(int b);

  void setErrorRecoveryFraction(at::real v);

  /** if the bits in w were arranged in a d*d grid and that grid was
   * rotated, what would the new bits in w be?
   * The bits are organized like this (for d = 3):
   *
   *  8 7 6       2 5 8      0 1 2
   *  5 4 3  ==>  1 4 7 ==>  3 4 5    (rotate90 applied twice)
   *  2 1 0       0 3 6      6 7 8
   **/
  static code_t rotate90(code_t w, uint d);

  /** Compute the hamming distance between two code_ts. **/
  static uint hammingDistance(code_t a, code_t b);

  /** How many bits are set in the code_t? **/
  static uint popCountReal(code_t w);

  enum { popCountTableShift = 12 };

  static const ByteArray& getPopCountTable();

  static uint popCount(code_t w);

  void decode(TagDetection& det, code_t rcode) const;


  uint getTagRenderDimension() const;

  void printHammingDistances() const;

  cv::Mat_<byte> makeImage(size_t id) const;

  cv::Mat getWarp(const TagDetection& det) const;

  cv::Mat superimposeDetection(const cv::Mat& image,
                               const TagDetection& det) const;

  cv::Mat detectionImage(const TagDetection& det,
                         const cv::Size& size, int cvtype,
                         const cv::Scalar& bgcolor = cv::Scalar(0,0,0,0)) const;
  
  cv::Mat superimposeDetections(const cv::Mat& image,
                                const TagDetectionArray& detections) const;

};


#endif
