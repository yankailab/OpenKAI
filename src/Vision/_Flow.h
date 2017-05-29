/*
 * _Flow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_VISION_FLOW_H_
#define SRC_VISION_FLOW_H_

#include "../Base/common.h"
#include "../Vision/FrameGroup.h"
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

#ifndef USE_OPENCV4TEGRA
	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;
#else
	Ptr<DenseOpticalFlowExt> m_pFarn;
#endif
	GpuMat							m_gFlow;

};

}

#endif
