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
	bool start(void);
	void draw(void);

	vDouble2 vFlow(vInt4* pROI);
	vDouble2 vFlow(vDouble4* pROI);

private:
	bool isFlowCorrect(Point2f u);
	Vec3b computeColor(float fx, float fy);
	void drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DenseFlow*) This)->update();
		return NULL;
	}

public:
	int	m_w;
	int m_h;
	_VisionBase*					m_pVision;
	FrameGroup*						m_pGrayFrames;
	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;
	GpuMat							m_gFlow;
	Mat								m_pFlow[2];

	int m_nHistLev;
	vDouble2 m_vRange;
	double m_minHistD;

};

}
#endif
#endif
