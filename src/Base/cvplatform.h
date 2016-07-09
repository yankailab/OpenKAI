#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/saliency.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/tracking.hpp>

using namespace cv;

#ifdef USE_CUDA
#include "opencv2/cudaobjdetect.hpp"
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudastereo.hpp"
#include "opencv2/cudawarping.hpp"
#include "opencv2/cudabgsegm.hpp"
using namespace cv::cuda;
#endif

#ifdef USE_ZED
#include <zed/Camera.hpp>
#endif


