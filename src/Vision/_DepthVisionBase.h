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
	virtual bool link(void);
	virtual bool draw(void);
	virtual void postProcessDepth(void);

	virtual void updateFilteredDistance(void);
	virtual Frame* Depth(void);
	virtual vDouble2 range(void);

	vInt2 matrixDim(void);
	double d(vInt4* pROI, vInt2* pPos);
	double d(vDouble4* pROI, vInt2* pPos);
	double dMedian(void);

public:
	int m_wD;
	int m_hD;

	Frame	m_fDepth;
	vDouble2 m_range;
	Mat m_mZ;
	Mat m_mD;

	vInt2		m_mDim;
	Median* 	m_pFilterMatrix;
	int			m_nFilter;
	Frame		m_fMatrixFrame;
	double		m_dMedian;

	Frame	m_depthShow;
	double	m_dShowAlpha;
	Window* m_pDepthWin;
	bool	m_bShowRawDepth;
	double	m_dShowRawBase;

};

}
#endif
