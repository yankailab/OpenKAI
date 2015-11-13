

#ifndef SRC_OPTICALFLOWDETECTOR_H_
#define SRC_OPTICALFLOWDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Camera/CamStream.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

#define TRD_INTERVAL_OBJDETECTOR 0
#define NUM_FASTOBJ 1000

namespace kai
{

struct OPTICALFLOW_OBJECT
{
	Rect		m_boundBox;
};

class OpticalFlowDetector: public ThreadBase
{
public:
	OpticalFlowDetector();
	~OpticalFlowDetector();

	bool init(JSON* pJson);
	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

	void setFrame(CamStream* pCam);
	int  getHuman(FAST_OBJECT** ppHuman);

private:
	void detect(void);

public:
	int 			m_frameID;
	CamStream*		m_pCamStream;

	Ptr<cuda::CascadeClassifier> m_pCascade;
	Ptr<cuda::HOG> m_pHumanHOG;
	int 			m_numHuman;
	FAST_OBJECT 	m_pHuman[NUM_FASTOBJ];



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



//    HOGDescriptor m_hogCar;
	int 			m_numCar;
	FAST_OBJECT 	m_pCar[NUM_FASTOBJ];

	Mat		m_frame;
	GpuMat  m_pGMat;

private:
	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((OpticalFlowDetector *) This)->update();
		return NULL;
	}


};
}

#endif /* SRC_OPTICALFLOWDETECTOR_H_ */
