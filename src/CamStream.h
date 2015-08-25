/*
 * CamStream.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMSTREAM_H_
#define SRC_CAMSTREAM_H_

#include "common.h"
#include "cvplatform.h"

#include "CamInput.h"
#include "CamFrame.h"
#include "CamMarkerDetect.h"
#include "CamDenseFlow.h"
#include "CamSparseFlow.h"
#include "CamMonitor.h"
#include "CamStereo.h"

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

	bool openWindow(void);
	void closeWindow(void);

public:
	string				m_camName;
	CamInput*			m_pCamL;
	CamInput*			m_pCamR;
	CamFrame*			m_pFrameL;
	CamFrame*			m_pFrameR;
	CamFrame**			m_pFrameProcess;
	CamFrame*			m_pHSV;
	CamFrame*			m_pGray;
	CamFrame*			m_pDepth;

	CamMarkerDetect* 	m_pMarkerDetect;
	CamDenseFlow*		m_pDenseFlow;
	CamSparseFlow*		m_pSparseFlow;
	CamStereo*			m_pStereo;

	CamMonitor*			m_pMonitor;

	bool		m_bStereoCam;
	bool		m_bMarkerDetect;
	bool		m_bDenseFlow;
	bool		m_bSparseFlow;
	bool		m_bHSV;
	bool		m_bGray;
	bool		m_bShowWindow;

private:
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
