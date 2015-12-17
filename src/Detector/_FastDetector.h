

#ifndef DETECTOR_FASTDETECTOR_H_
#define DETECTOR_FASTDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "DetectorBase.h"
#include "../Utility/util.h"


using namespace cv;
using namespace cv::cuda;
using namespace std;

#define TRD_INTERVAL_FASTDETECTOR 1000000

namespace kai
{

struct FAST_OBJECT
{
	uint16_t		m_status;
	uint64_t		m_frameID;

	Rect			m_boundBox;
};

class _FastDetector: public DetectorBase, public _ThreadBase
{
public:
	_FastDetector();
	~_FastDetector();

	bool init(JSON* pJson);
	bool start(void);

	void updateFrame(CamFrame* pFrame, CamFrame* pGray);

	int  getHuman(FAST_OBJECT** ppHuman);

private:
	inline int findVacancy(int iStart);
	inline void deleteOutdated(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_FastDetector *) This)->update();
		return NULL;
	}

public:
//	_CamStream*		m_pCamStream;

	Ptr<cuda::CascadeClassifier> m_pCascade;
	Ptr<cuda::HOG>	m_pHumanHOG;
	int 				m_iHuman;
	int				m_numHuman;
	FAST_OBJECT* 	m_pHuman;
	uint64_t			m_frameID;
	uint64_t			m_objLifeTime;



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
//	int 			m_numCar;
//	FAST_OBJECT 	m_pCar[NUM_FASTOBJ];

	Mat			m_Mat;
	CamFrame*	m_pBGRA;
//	GpuMat*  	m_pGMat;


};
}

#endif /* SRC_FASTDETECTOR_H_ */
