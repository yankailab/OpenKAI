#pragma once
#include "platform.h"

//Common include
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/features2d.hpp"

#ifdef PLATFORM_WIN
#include "opencv2/highgui.hpp"

#elif PLATFORM_UBUNTU
//#include <sys/time.h>

#endif

#ifdef USE_CUDA
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"
#endif

#define PER_FRAME_TIME_LOGGING 0
#define SHOW_DEBUG_WINDOW 1
#define DRAW_DEBUG_DATA 0


#include "defaultparam.h"



using namespace std;
using namespace cv;
using namespace cv::cuda;


struct fVector4
{
	double m_x;
	double m_y;
	double m_z;
	double m_w;
};

struct fVector3
{
	double m_x;
	double m_y;
	double m_z;
};

struct fVector2
{
	double m_x;
	double m_y;
};

struct CAMERA_DATA
{
	bool m_bBall;



};

struct SYSTEM
{
	char m_mode;
};



