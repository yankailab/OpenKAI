#ifndef REGULARIZATION_FILTER_HPP
#define REGULARIZATION_FILTER_HPP

//#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>

#include <cmath>

#include "../../../Dependency/OpenTracker/eco/debug.hpp"
#include "../../../Dependency/OpenTracker/eco/fftTool.hpp"
#include "../../../Dependency/OpenTracker/eco/parameters.hpp"

namespace eco{
cv::Mat  get_regularization_filter(cv::Size sz, cv::Size2f target_sz, const EcoParameters& params);
}
#endif 