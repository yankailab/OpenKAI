/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#ifndef _IMAGEUTIL_H_
#define _IMAGEUTIL_H_

#include <opencv2/core/core.hpp>

enum ScaleType {
  ScaleNone,
  ScaleMinMax,
  ScaleAbs
};


/** Rescales the image to normalize the brightness. */
cv::Mat rescaleImageIntensity(const cv::Mat& img, ScaleType type);

enum {
  DEFAULT_SCREEN_W = 1280,
  DEFAULT_SCREEN_H = 760
};

/** Multiplies the image by the largest integer scale factor that will
 *  make it no bigger than the screen dimensions given, and returns
 *  the scale factor.
 */
int resizeToDisplay(const cv::Mat& src, cv::Mat& dst,
                    int w=DEFAULT_SCREEN_W, int h=DEFAULT_SCREEN_H);

/** Places a black and white label on an image. */
void labelImage(cv::Mat& img, const std::string& text);

/** Convenience function to call the above three functions and place
 *  the result on the screen. */
void labelAndWaitForKey(const std::string& window,
                        const std::string& text, 
                        const cv::Mat& img, 
                        ScaleType type,
                        bool resize,
                        int w=DEFAULT_SCREEN_W,
                        int h=DEFAULT_SCREEN_H);

#endif
