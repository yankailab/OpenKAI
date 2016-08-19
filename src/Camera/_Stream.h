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
#include "Camera.h"

#include "Frame.h"

namespace kai
{

class _Stream: public _ThreadBase
{
public:
	_Stream();
	virtual ~_Stream();

	bool init(JSON* pJson, string camName);
	bool start(void);
	bool complete(void);

	Frame* getFrame(void);
	Frame* getGrayFrame(void);
	Frame* getHSVFrame(void);

	Camera* getCameraInput(void);

public:
	bool		m_bHSV;
	bool		m_bGray;

private:
	string				m_camName;
	Camera*			m_pCamInput;

	Frame*			m_pCamFrame;
	Frame*			m_pGrayFrame;
	Frame*			m_pHSVframe;

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Stream *) This)->update();
		return NULL;
	}
};

} /* namespace kai */

#endif /* SRC_CAMSTREAM_H_ */
