/*
 * _DepthVisionBase.h
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__DepthVisionBase_H_
#define OpenKAI_src_Vision__DepthVisionBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "_VisionBase.h"

namespace kai
{

class _DepthVisionBase: public _VisionBase
{
public:
	_DepthVisionBase();
	virtual ~_DepthVisionBase();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual void reset(void);
	virtual bool draw(void);

	virtual Frame* depth(void);
	virtual vDouble2 range(void);

public:
	int m_wD;
	int m_hD;

	Frame*	m_pDepth;
	vDouble2 m_range;
	Mat m_mZ;
	Mat m_mD;

	Frame*	m_pDepthShow;
	Window* m_pDepthWin;

};

}

#endif
