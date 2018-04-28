/*
 * _Flow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Flow_H_
#define OpenKAI_src_Vision__Flow_H_

#include "../Base/common.h"
#include "FrameGroup.h"
#include "_VisionBase.h"

namespace kai
{

class _Flow: public _ThreadBase
{
public:
	_Flow();
	virtual ~_Flow();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	bool addFrame(bool bFrame, Frame* pGray);
	GpuMat* flowMat(void);

private:
//	void generateFlowMap(const GpuMat& d_flow);

	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Flow*) This)->update();
		return NULL;
	}

public:
	int	m_width;
	int m_height;
	_VisionBase*					m_pVision;
	FrameGroup*						m_pGrayFrames;

	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;
	GpuMat							m_gFlow;

};

}

#endif
