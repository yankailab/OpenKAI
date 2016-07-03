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


namespace kai
{

struct MARKER_CIRCLE
{
	double	m_x;
	double	m_y;
	double	m_r;
};

class _MarkerDetector : public DetectorBase, public _ThreadBase
{
public:
	_MarkerDetector();
	virtual ~_MarkerDetector();

	bool init(JSON* pJson, string name);
	bool start(void);

	bool getCircleCenter(fVector2* pCenter);


private:
	void detectCircle(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MarkerDetector*) This)->update();
		return NULL;
	}


public:
	int		 m_numCircle;
	MARKER_CIRCLE m_pCircle[NUM_MARKER];

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
