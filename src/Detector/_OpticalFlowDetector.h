

#ifndef SRC_OPTICALFLOWDETECTOR_H_
#define SRC_OPTICALFLOWDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Camera/_CamStream.h"
#include "DetectorBase.h"


using namespace cv;
using namespace cv::cuda;
using namespace std;

#define TRD_INTERVAL_OBJDETECTOR 0
#define NUM_OPTICALFLOW_OBJECT 100

namespace kai
{

struct OPTICALFLOW_OBJECT
{
	Rect		m_boundBox;
};

class _OpticalFlowDetector: public DetectorBase, _ThreadBase
{
public:
	_OpticalFlowDetector();
	~_OpticalFlowDetector();

	bool init(JSON* pJson);
	bool start(void);

	void setFrame(_CamStream* pCam);
	int  getHuman(OPTICALFLOW_OBJECT** ppHuman);

private:
	void detect(void);

public:
	_CamStream*		m_pCamStream;

	Ptr<cuda::CascadeClassifier> m_pCascade;
	Ptr<cuda::HOG> m_pHumanHOG;
	int 			m_numHuman;
	OPTICALFLOW_OBJECT 	m_pHuman[NUM_OPTICALFLOW_OBJECT];

    double scale;
    int nlevels;
    int gr_threshold;
    double hit_threshold;

    int win_width;
    int win_stride_width, win_stride_height;
    int block_width;
    int block_stride_width, block_stride_height;
    int cell_width;
    int nbins;


//  HOGDescriptor	m_hogCar;
	int 			m_numCar;
	OPTICALFLOW_OBJECT 	m_pCar[NUM_OPTICALFLOW_OBJECT];

	Mat		m_frame;
//	GpuMat  m_pGMat;

private:
//	pthread_t m_threadID;
//	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OpticalFlowDetector *) This)->update();
		return NULL;
	}


};
}

#endif /* SRC_OPTICALFLOWDETECTOR_H_ */
