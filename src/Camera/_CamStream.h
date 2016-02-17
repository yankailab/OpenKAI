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
#include "../Detector/_ObjectDetector.h"

#include "CamInput.h"
#include "CamFrame.h"
#include "FrameGroup.h"

#define TRD_INTERVAL_CAMSTREAM 0

//#define CAMSTREAM_MUTEX_ORIGINAL 0
//#define CAMSTREAM_MUTEX_GRAY 1
//#define CAMSTREAM_MUTEX_HSV 2

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

	CamFrame* getLastFrame(void);
	CamFrame* getLastGrayFrame(void);
	CamFrame* getLastHSVFrame(void);

	FrameGroup* getFrameGroup(void);
	FrameGroup* getGrayFrameGroup(void);
	FrameGroup* getHSVFrameGroup(void);

public:
	bool		m_bHSV;
	bool		m_bGray;

private:
	string				m_camName;
	CamInput*			m_pCamInput;

	FrameGroup*			m_pCamFrames;
	FrameGroup*			m_pGrayFrames;
	FrameGroup*			m_pHSVframes;

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
