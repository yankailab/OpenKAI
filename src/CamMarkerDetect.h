/*
 * CameraMarkerDetect.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMMARKERDETECT_H_
#define SRC_CAMMARKERDETECT_H_

#include "common.h"
#include "stdio.h"
#include "cvplatform.h"

#define NUM_MARKER 128
#define NUM_TARGET_MARKER 2
#define MARKER_AREA_RATIO 0.5
#define MIN_MARKER_SIZE 5

namespace kai
{

class CamMarkerDetect
{
public:
	CamMarkerDetect();
	virtual ~CamMarkerDetect();


	unsigned int  m_frameID;

	fVector4 m_flow;
	int		 m_numAllMarker;
	fVector4 m_pAllMarker[NUM_MARKER];
	fVector4 m_pTargetMarker[NUM_TARGET_MARKER];
	//	Ptr<cuda::HoughCirclesDetector> m_pHoughCircle;

	//0:Not locked,1:Pos locked, 2:Size and Attitude locked
	int		 m_objLockLevel;
	fVector3 m_targetObjPos;
	fVector3 m_objPos;
	fVector3 m_objROIPos;
	fVector3 m_objAtt;	//Roll, Pitch, Yaw

	double	m_minMarkerSize = MIN_MARKER_SIZE;
	double  m_areaRatio = MARKER_AREA_RATIO;

	Ptr<SimpleBlobDetector> m_pBlobDetector;

#ifdef USE_CUDA
	GpuMat  m_Huered;
	GpuMat  m_Scalehuered;
	GpuMat  m_Scalesat;
	GpuMat  m_Balloonyness;
	GpuMat  m_Thresh;
#else

#endif
};

} /* namespace kai */

#endif /* SRC_CAMMARKERDETECT_H_ */
