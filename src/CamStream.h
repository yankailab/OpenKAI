/*
 * CamStream.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMSTREAM_H_
#define SRC_CAMSTREAM_H_

#include "common.h"
#include "stdio.h"
#include "cvplatform.h"

#include "CamInput.h"
#include "CamFrame.h"
#include "CamMarkerDetect.h"
#include "CamDenseFlow.h"
#include "CamSparseFlow.h"
#include "CamMonitor.h"

namespace kai
{

class CamStream: public ThreadBase
{
public:
	CamStream();
	virtual ~CamStream();

	bool init(void);
	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

private:
	CamInput*			m_pCamL;
	CamInput*			m_pCamR;
	CamFrame*			m_pFrameL;
	CamFrame*			m_pFrameR;
	CamFrame**			m_pFrameProcess;
	CamFrame*			m_pHSV;

	CamMarkerDetect* 	m_pMarkerDetect;
	CamDenseFlow*		m_pDenseFlow;
	CamSparseFlow*		m_pSparseFlow;

	CamMonitor*			m_pMonitor;


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
