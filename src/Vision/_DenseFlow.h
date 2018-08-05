/*
 * _DenseFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Flow_H_
#define OpenKAI_src_Vision__Flow_H_

#include "../Base/common.h"
#include "FrameGroup.h"
#include "_VisionBase.h"

#ifdef USE_CUDA

namespace kai
{

class _DenseFlow: public _ThreadBase
{
public:
	_DenseFlow();
	virtual ~_DenseFlow();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	bool isFlowCorrect(Point2f u);
	Vec3b computeColor(float fx, float fy);
	void drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DenseFlow*) This)->update();
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
#endif
