/*
 * CameraMarkerDetect.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef DETECTOR_MARKERDETECTOR_H_
#define DETECTOR_MARKERDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Camera/_CamStream.h"
#include "DetectorBase.h"
#include "../Base/_ThreadBase.h"

#define NUM_MARKER 128
#define NUM_TARGET_MARKER 2
#define MARKER_AREA_RATIO 0.6
#define MIN_MARKER_SIZE 5

#define LOCK_LEVEL_NONE 0
#define LOCK_LEVEL_POS 1
#define LOCK_LEVEL_SIZE 2
#define LOCK_LEVEL_ATT 3

#define TRD_INTERVAL_MARKERDETECTOR 100

namespace kai
{

class _MarkerDetector : public DetectorBase, public _ThreadBase
{
public:
	_MarkerDetector();
	virtual ~_MarkerDetector();

	bool init(JSON* pJson, string name);
	bool start(void);

	//Object detection using markers
	void setObjROI(fVector3 ROI);
	int  getObjLockLevel(void);
	bool getObjPosition(fVector3* pPos);
	bool getObjAttitude(fVector3* pAtt);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MarkerDetector*) This)->update();
		return NULL;
	}


public:
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

	double	m_minMarkerSize;
	double  m_areaRatio;

	Ptr<SimpleBlobDetector> m_pBlobDetector;

	_CamStream*			m_pCamStream;


#ifdef USE_CUDA
	GpuMat m_HSV;
	GpuMat m_Hue;
	GpuMat m_Sat;
	GpuMat m_Val;

	GpuMat  m_Huered;
	GpuMat  m_Scalehuered;
	GpuMat  m_Scalesat;
	GpuMat  m_Balloonyness;
	GpuMat  m_Thresh;

	int		m_cudaDeviceID;
#else

#endif
};

} /* namespace kai */

#endif /* SRC_MarkerDetector_H_ */
