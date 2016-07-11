

#ifndef DETECTOR_FeatureDetector_H_
#define DETECTOR_FeatureDetector_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Detector/DetectorBase.h"
#include "../Utility/util.h"
#include "../Camera/_CamStream.h"
#include "../Camera/FrameGroup.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

#define CASCADE_CPU 0
#define CASCADE_CUDA 1

namespace kai
{

class _FeatureDetector: public DetectorBase, public _ThreadBase
{
public:
	_FeatureDetector();
	~_FeatureDetector();

	bool init(string name, JSON* pJson);
	bool start(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_FeatureDetector *) This)->update();
		return NULL;
	}

public:
	int					m_device;

	Ptr<cuda::CornersDetector> m_pDetector;
	Ptr<cuda::SparsePyrLKOpticalFlow> m_pPyrLK;
	unsigned int m_numCorners;

	uint64_t			m_frameID;

	_CamStream*			m_pCamStream;
	FrameGroup*			m_pGrayFrames;

	Ptr<AKAZE> m_pAkaze;
	Ptr<cv::DescriptorMatcher> m_pMatcher;

	Mat			m_Mat;
	Mat			m_targetMat;
	std::vector<cv::KeyPoint> m_targetKeypoint;
	std::vector<cv::KeyPoint> m_imgKeypoint;
	cv::Mat m_targetDescriptor;
	cv::Mat m_imgDescriptor;

	GpuMat		m_GMat;
	CamFrame*	m_pGray;

};
}

#endif /* SRC_FASTDETECTOR_H_ */
