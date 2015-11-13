/*
 * CameraOpticalFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMDENSEFLOW_H_
#define SRC_CAMDENSEFLOW_H_

#include "../Base/common.h"
#include "stdio.h"
#include "../Base/cvplatform.h"

#include "../Camera/CamFrame.h"

namespace kai
{

class CamDenseFlow
{
public:
	CamDenseFlow();
	virtual ~CamDenseFlow();

	bool init(void);
	fVector4 detect(CamFrame* pFrame);


	inline bool isFlowCorrect(Point2f u);
	Vec3b computeColor(float fx, float fy);
	void drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion);
	void showFlow(const char* name, const GpuMat& d_flow);

public:
	fVector4 m_flow;
	GpuMat m_flowMat;

	Mat m_uFlowMat;
	Mat m_flowX;
	Mat m_flowY;

	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;
};

} /* namespace kai */

#endif /* SRC_CAMDENSEFLOW_H_ */
