/*
 * _DepthVisionBase.h
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__DepthVisionBase_H_
#define OpenKAI_src_Vision__DepthVisionBase_H_

#include "_VisionBase.h"
#include "../Filter/Median.h"

namespace kai
{

class _DepthVisionBase: public _VisionBase
{
public:
	_DepthVisionBase();
	virtual ~_DepthVisionBase();

	virtual bool init(void* pKiss);
	virtual void cvDraw(void* pWindow);

	virtual Frame* Depth(void);
	float d(vInt4* pROI);
	float d(vFloat4* pROI);

public:
    vInt2   m_vDsize;
	Frame	m_fDepth;
	int		m_nHistLev;
	int		m_iHistFrom;
	float	m_dScale;
	vInt4	m_vDroi;

	vFloat2	m_vRange;
	float	m_minHistD;

	Frame	m_fDepthShow;
	_WindowCV* m_pDepthWin;

};

}
#endif
