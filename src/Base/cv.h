
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/bgsegm.hpp>
#include <opencv2/dnn.hpp>

#ifdef USE_OPENCV_CONTRIB
#include <opencv2/tracking.hpp>
#endif

#ifdef USE_CUDA
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudastereo.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudabgsegm.hpp>
using namespace cv::cuda;
#endif

using namespace cv;
using namespace dnn;

#define KEY_ARROW_UP 82
#define KEY_ARROW_DOWN 84
#define KEY_ARROW_LEFT 81
#define KEY_ARROW_RIGHT 83
