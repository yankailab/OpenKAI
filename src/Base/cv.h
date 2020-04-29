
#ifdef USE_OPENCV

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

#if CV_VERSION_MAJOR==3
#define COLOR_BGR2YCrCb CV_BGR2YCrCb
#define COLOR_YCrCb2BGR CV_YCrCb2BGR
#define COLOR_GRAY2BGR CV_GRAY2BGR
#define COLOR_BGR2GRAY CV_BGR2GRAY
#define COLOR_RGB2BGR CV_RGB2BGR
#define COLOR_BGR2HSV CV_BGR2HSV

#define IMWRITE_PNG_COMPRESSION CV_IMWRITE_PNG_COMPRESSION
#define IMWRITE_JPEG_QUALITY CV_IMWRITE_JPEG_QUALITY

#define WINDOW_NORMAL CV_WINDOW_NORMAL
#define WND_PROP_FULLSCREEN CV_WND_PROP_FULLSCREEN
#define WINDOW_FULLSCREEN CV_WINDOW_FULLSCREEN
#define WINDOW_AUTOSIZE CV_WINDOW_AUTOSIZE

#define CAP_PROP_FRAME_WIDTH CV_CAP_PROP_FRAME_WIDTH
#define CAP_PROP_FRAME_HEIGHT CV_CAP_PROP_FRAME_HEIGHT
#define CAP_PROP_FPS CV_CAP_PROP_FPS

#define RETR_EXTERNAL CV_RETR_EXTERNAL
#define CHAIN_APPROX_NONE CV_CHAIN_APPROX_NONE
#endif


#define KEY_ARROW_UP 82
#define KEY_ARROW_DOWN 84
#define KEY_ARROW_LEFT 81
#define KEY_ARROW_RIGHT 83

#endif
