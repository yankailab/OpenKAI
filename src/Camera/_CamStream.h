/*
 * CamStream.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMSTREAM_H_
#define SRC_CAMSTREAM_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Image/_DenseFlow.h"
#include "../Detector/_ObjectDetector.h"
#include "../AI/_SegNet.h"
#include "../Detector/_CascadeDetector.h"

#include "CamInput.h"
#include "CamFrame.h"
#include "CamSparseFlow.h"
#include "CamStereo.h"

#define TRD_INTERVAL_CAMSTREAM 0

#define FRAME_USER_NUM 5

namespace kai
{

class _CamStream: public _ThreadBase
{
public:
	_CamStream();
	virtual ~_CamStream();

	bool init(JSON* pJson, string camName);
	bool start(void);
	bool complete(void);

	bool addOriginalUser(CamFrame** ppFrame);
	bool addGrayUser(CamFrame** ppFrame);
	bool addHSVUser(CamFrame** ppFrame);

public:
	string				m_camName;
	CamInput*			m_pCamL;
	CamInput*			m_pCamR;
	CamFrame*			m_pFrameL;
	CamFrame*			m_pFrameR;
	CamFrame**			m_pFrameProcess;
	CamFrame*			m_pHSV;
	CamFrame*			m_pGrayL;
	CamFrame*			m_pGrayR;
	CamFrame*			m_pDepth;
//	CamFrame*			m_pBGRAL;

	CamSparseFlow*		m_pSparseFlow;
	CamStereo*			m_pStereo;

	bool		m_bStereoCam;
//	bool		m_bSparseFlow;
	bool		m_bHSV;
	bool		m_bGray;

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_CamStream *) This)->update();
		return NULL;
	}
};

} /* namespace kai */

#endif /* SRC_CAMSTREAM_H_ */
