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

	class _DepthVisionBase : public _VisionBase
	{
	public:
		_DepthVisionBase();
		virtual ~_DepthVisionBase();

		virtual bool init(void *pKiss);
		virtual void draw(void *pFrame);

		virtual Frame *Depth(void);
		float d(const vInt4 &bb);
		float d(const vFloat4 &bb);
		virtual Frame *DepthShow(void);

	public:
		vInt2 m_vDsize;
		Frame m_fDepth;
		
		vFloat2 m_vRange;
		float m_dScale;
		float m_dOfs;

		int m_nHistLev;
		int m_iHistFrom;
		float m_minHistD;

		Frame m_fDepthShow;
		bool m_bDepthShow;
	};

}
#endif
