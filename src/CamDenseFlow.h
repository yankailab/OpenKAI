/*
 * CameraOpticalFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMDENSEFLOW_H_
#define SRC_CAMDENSEFLOW_H_

#include "common.h"
#include "stdio.h"
#include "cvplatform.h"

namespace kai
{

class CamDenseFlow
{
public:
	CamDenseFlow();
	virtual ~CamDenseFlow();


private:
	unsigned int  m_frameID;
	fVector4 m_flow;
	GpuMat   m_flowMat;

	Mat		 m_uFlowMat;
	UMat	 m_flowX;
	UMat	 m_flowY;

	Ptr<cuda::FarnebackOpticalFlow> m_pFarn;
};

} /* namespace kai */

#endif /* SRC_CAMDENSEFLOW_H_ */
