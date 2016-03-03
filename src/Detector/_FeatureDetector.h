

#ifndef DETECTOR_FeatureDetector_H_
#define DETECTOR_FeatureDetector_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "DetectorBase.h"
#include "../Utility/util.h"
#include "../Camera/_CamStream.h"
#include "../Camera/FrameGroup.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

#define TRD_INTERVAL_FEATUREDETECTOR 10

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

private:
	Mat			m_Mat;
	GpuMat		m_GMat;
	CamFrame*	m_pGray;

};
}

#endif /* SRC_FASTDETECTOR_H_ */
