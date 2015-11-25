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

#include "CamInput.h"
#include "CamFrame.h"
#include "CamDenseFlow.h"
#include "CamSparseFlow.h"
#include "CamStereo.h"

#define TRD_INTERVAL_CAMSTREAM 1000

namespace kai
{

class CamStream: public ThreadBase
{
public:
	CamStream();
	virtual ~CamStream();

	bool setup(JSON* pJson, string camName);
	bool init(void);
	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

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
	CamFrame*			m_pBGRAL;

	CamDenseFlow*		m_pDenseFlow;
	CamSparseFlow*		m_pSparseFlow;
	CamStereo*			m_pStereo;

	bool		m_bStereoCam;
	bool		m_bDenseFlow;
	bool		m_bSparseFlow;
	bool		m_bHSV;
	bool		m_bGray;

private:
public:
	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((CamStream *) This)->update();
		return NULL;
	}
};

} /* namespace kai */

#endif /* SRC_CAMSTREAM_H_ */
