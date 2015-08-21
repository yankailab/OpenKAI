#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"

#ifdef USE_CUDA
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"
#endif

using namespace cv;
using namespace cv::cuda;
