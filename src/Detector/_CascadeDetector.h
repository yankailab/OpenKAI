

#ifndef DETECTOR_CASCADEDETECTOR_H_
#define DETECTOR_CASCADEDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "DetectorBase.h"
#include "../Utility/util.h"
#include "../Camera/_CamStream.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

#define TRD_INTERVAL_CASCADEDETECTOR 0

namespace kai
{

struct CASCADE_OBJECT
{
	uint16_t		m_status;
	uint64_t		m_frameID;
	Rect			m_boundBox;
};

class _CascadeDetector: public DetectorBase, public _ThreadBase
{
public:
	_CascadeDetector();
	~_CascadeDetector();

	bool init(string name, JSON* pJson);
	bool start(void);

//	void updateFrame(CamFrame* pFrame, CamFrame* pGray);

	int  getObjList(CASCADE_OBJECT** ppObj);

private:
	inline int findVacancy(int iStart);
	inline void deleteOutdated(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CascadeDetector *) This)->update();
		return NULL;
	}

public:
	Ptr<cuda::CascadeClassifier> m_pCascade;
	int 				m_iObj;
	int					m_numObj;
	CASCADE_OBJECT* 	m_pObj;
	uint64_t			m_frameID;
	uint64_t			m_objLifeTime;
	int					m_posDiff;

	Mat			m_Mat;

	_CamStream*			m_pCamStream;

};
}

#endif /* SRC_FASTDETECTOR_H_ */
