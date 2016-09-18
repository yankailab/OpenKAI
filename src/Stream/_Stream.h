/*
 * _Stream.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_STREAM_STREAM_H_
#define SRC_STREAM_STREAM_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Base/_ThreadBase.h"

#include "../Camera/Camera.h"
#include "../Camera/ZED.h"
#include "Frame.h"

namespace kai
{

class _Stream: public _ThreadBase
{
public:
	_Stream();
	virtual ~_Stream();

	bool init(Config* pConfig);
	bool start(void);
	bool complete(void);
	bool draw(Frame* pFrame, iVec4* textPos);

	Frame* getBGRFrame(void);
	Frame* getGrayFrame(void);
	Frame* getHSVFrame(void);

	CamBase* getCameraInput(void);

public:
	bool		m_bHSV;
	bool		m_bGray;

private:
	CamBase*		m_pCamera;

	Frame*			m_pCamFrame;
	Frame*			m_pGrayFrame;
	Frame*			m_pHSVframe;

	int				m_showDepth;

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Stream *) This)->update();
		return NULL;
	}
};

} /* namespace kai */

#endif /* SRC_STREAM_STREAM_H_ */
