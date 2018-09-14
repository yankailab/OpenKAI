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
#include "../Filter/Median.h"

namespace kai
{

class _DepthVisionBase: public _VisionBase
{
public:
	_DepthVisionBase();
	virtual ~_DepthVisionBase();

	virtual bool init(void* pKiss);
	virtual bool draw(void);

	virtual Frame* Depth(void);
	double d(vInt4* pROI);
	double d(vDouble4* pROI);

public:
	int m_wD;
	int m_hD;
	Frame	m_fDepth;

	Frame	m_depthShow;
	Window* m_pDepthWin;

};

}
#endif
