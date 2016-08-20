/*
 *
 */


#ifndef DETECTOR_DEPTHDETECTOR_H_
#define DETECTOR_DEPTHDETECTOR_H_

#include "../Navigation/_Universe.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "DetectorBase.h"
#include "../Camera/_Stream.h"
#include "../Vision/_Flow.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

namespace kai
{

class _DepthDetector: public DetectorBase, public _ThreadBase
{
public:
	_DepthDetector();
	~_DepthDetector();

	bool init(JSON* pJson, string camName);
	bool start(void);

private:
	void detect(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DepthDetector*) This)->update();
		return NULL;
	}

public:
	_Universe*	m_pUniverse;
	Frame*		m_pDepth;
	uint64_t	m_camFrameID;
	_Stream*	m_pCamStream;
	Camera*		m_pCam;
	_Flow*		m_pFlow;

	Mat			m_Mat;

	int			m_minObjArea;
	int			m_maxObjArea;
};
}

#endif /* SRC_DEPTHDETECTOR_H_ */
