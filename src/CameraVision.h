#pragma once
#include "common.h"
#include "stdio.h"

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

#define SMALL_WIDTH 300
#define SMALL_HEIGHT 300

#define NUM_MARKER 128
#define NUM_TARGET_MARKER 2
#define MARKER_AREA_RATIO 0.5
#define MIN_MARKER_SIZE 5

#define LOCK_LEVEL_NONE 0
#define LOCK_LEVEL_POS 1
#define LOCK_LEVEL_SIZE 2
#define LOCK_LEVEL_ATT 3

#define OPF_BROX 0
#define OPF_LK 1
#define OPF_FARN 2
#define OPF_TVL1 3

#define FRAME_SMALL 1
#define FRAME_GRAY (1<<1)
#define FRAME_GRAY_SMALL (1<<2)
#define FRAME_HSV (1<<3)

#define MAX_FRAME_ID 0xffffffff

struct GENERAL_MATRICES
{
	UMat m_uFrame;

	//Frame Index, updated in each frame update
	unsigned int  m_frameID;

	//index for switching New and Old frame
	unsigned char m_iFrame;
	cv::Size m_smallSize;

#ifdef USE_CUDA
	GpuMat* m_pNew;
	GpuMat* m_pOld;
	GpuMat* m_pSmallNew;
	GpuMat* m_pSmallOld;
	GpuMat* m_pGrayNew;
	GpuMat* m_pGrayOld;
	GpuMat* m_pSmallGrayNew;
	GpuMat* m_pSmallGrayOld;

	GpuMat  m_pFrame[2];
	GpuMat  m_pSmallFrame[2];
	GpuMat  m_pGrayFrame[2];
	GpuMat  m_pSmallGrayFrame[2];

	GpuMat  m_Hsv;
	GpuMat  m_Hue;
	GpuMat  m_Sat;
	GpuMat  m_Val;

#else
	UMat* m_pNew;
	UMat* m_pOld;
	UMat* m_pSmallNew;
	UMat* m_pSmallOld;
	UMat* m_pGrayNew;
	UMat* m_pGrayOld;
	UMat* m_pSmallGrayNew;
	UMat* m_pSmallGrayOld;

	UMat  m_pFrame[2];
	UMat  m_pSmallFrame[2];
	UMat  m_pGrayFrame[2];
	UMat  m_pSmallGrayFrame[2];
#endif
};

struct MARKER_FLOW
{
	unsigned int  m_frameID;

	fVector4 m_flow;
	int		 m_numAllMarker;
	fVector4 m_pAllMarker[NUM_MARKER];
	fVector4 m_pTargetMarker[NUM_TARGET_MARKER];
	//	Ptr<cuda::HoughCirclesDetector> m_pHoughCircle;

	//0:Not locked,1:Pos locked, 2:Size and Attitude locked
	int		 m_objLockLevel;
	fVector3 m_targetObjPos;
	fVector3 m_objPos;
	fVector3 m_objROIPos;
	fVector3 m_objAtt;	//Roll, Pitch, Yaw

	double	m_minMarkerSize = MIN_MARKER_SIZE;
	double  m_areaRatio = MARKER_AREA_RATIO;

	Ptr<SimpleBlobDetector> m_pBlobDetector;

#ifdef USE_CUDA
	GpuMat  m_Huered;
	GpuMat  m_Scalehuered;
	GpuMat  m_Scalesat;
	GpuMat  m_Balloonyness;
	GpuMat  m_Thresh;
#else

#endif

};

struct OPTICAL_FLOW
{
	unsigned int  m_frameID;
	fVector4 m_flow;
	GpuMat   m_flowMat;

	Mat		 m_uFlowMat;
	UMat	 m_flowX;
	UMat	 m_flowY;

	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;

//	int	m_method;
//	Ptr<cuda::BroxOpticalFlow> m_pBrox;
//	Ptr<cuda::DensePyrLKOpticalFlow> m_pLK;
//	Ptr<cuda::OpticalFlowDual_TVL1> m_pTVL1;

};

struct FEATURE_FLOW
{
	unsigned int      m_frameID;
	fVector4 m_flow;
	fVector3 m_diffPrev;

#ifdef USE_CUDA
	Ptr<cuda::CornersDetector> m_pDetector;
	Ptr<cuda::SparsePyrLKOpticalFlow> m_pPyrLK;

	GpuMat** m_pFrameNew;
	GpuMat** m_pFrameOld;
	unsigned int m_numCorners;

#else
	std::vector<cv::KeyPoint> m_keypoint1;
	std::vector<cv::KeyPoint> m_keypoint2;

	cv::Ptr<cv::DescriptorExtractor> m_pExtractor;
	cv::UMat m_descriptor1;
	cv::UMat m_descriptor2;

	cv::Ptr<cv::DescriptorMatcher> m_pMatcher;
	std::vector<cv::DMatch> m_dMatch;

#endif
};

struct BENCHMARK
{
	int m_nFrames;
};


class CameraVision
{
public:
	CameraVision();
	~CameraVision();

	bool init(void);
	void updateFrame(UMat* pFrame);

	//Object detection using markers
	void initMarkerFlow(void);
	void updateMarkerFlow(void);
	void setObjROI(fVector3 ROI);
	int  getObjLockLevel(void);
	bool getObjPosition(fVector3* pPos);
	bool getObjAttitude(fVector3* pAtt);

	//Optical flow
	void initOpticalFlow(void);
	void updateOpticalFlow(void);
	void getOpticalFlowVelocity(fVector4* pFlow);

	//Feature Flow
	void initFeatureFlow(void);
	void updateFeatureFlow(void);
	void getFeatureFlowVelocity(fVector4* pFlow);

	//Flow result
	void getGlobalFlowVelocity(fVector4* pFlow);
	void getPartialFlowVelocity(fVector4* pFlowObj);

	//Frame interface
	cv::UMat* getMat(void);

private:
	void drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion = -1);
	void showFlow(const char* name, const GpuMat& d_flow);
	Vec3b computeColor(float fx, float fy);
	inline bool isFlowCorrect(Point2f u);

	void drawFeatureFlow(Mat& frame, const vector<Point2f>& prevPts, const vector<Point2f>& nextPts, const vector<uchar>& status, Scalar line_color = Scalar(0, 0, 255));

	void recordTime(long delta, double *avgTime);
	void log(const char* msg, ...);

	//Switch
	unsigned int m_functionFlag;

	//General Mat
	GENERAL_MATRICES	m_Mat;
	MARKER_FLOW*		m_pMarkerFlow;
	OPTICAL_FLOW*		m_pOpticalFlow;
	FEATURE_FLOW*		m_pFeatureFlow;

	//benchmark info
	BENCHMARK*			m_pBenchmark;


};

