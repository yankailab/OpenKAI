/*
 * _DNNodometry.h
 *
 *  Created on: June 8, 2017
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN_SENSING_DNNodometry_H_
#define OPENKAI_SRC_DNN_SENSING_DNNodometry_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Vision/_Camera.h"
#include "../../Vision/FrameGroup.h"

namespace kai
{

class _DNNodometry: public _ThreadBase
{
public:
	_DNNodometry(void);
	virtual ~_DNNodometry();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	void d(vDouble3* pT, vDouble3* pR);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DNNodometry *) This)->update();
		return NULL;
	}

public:
	_Camera*	m_pCam;
	FrameGroup*	m_pFrames;

	int			m_width;
	int			m_height;
	bool		m_bCrop;
	Rect		m_cropBB;

	vDouble3	m_vT;
	vDouble3	m_vR;

};

}

#endif
