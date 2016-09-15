/*
 * _MarkerDetect.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef DETECTOR_MARKERDETECTOR_H_
#define DETECTOR_MARKERDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Stream/_Stream.h"
#include "../Base/_ThreadBase.h"
#include "DetectorBase.h"

#define NUM_MARKER 128

#define METHOD_FILL 0
#define METHOD_HOUGH 1
#define MARKER_AREA_RATIO 0.8
#define MIN_MARKER_SIZE 10


namespace kai
{

class _Bullseye : public DetectorBase, public _ThreadBase
{
public:
	_Bullseye();
	virtual ~_Bullseye();

	bool init(Config* pConfig, string name);
	bool start(void);

	bool getCircleCenter(fVec3* pCenter);

private:
	void detectCircleFill(void);
	void detectCircleHough(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Bullseye*) This)->update();
		return NULL;
	}


public:
	int		 m_numCircle;
	fVec3 m_pCircle[NUM_MARKER];

	int		m_method;

	double	m_minMarkerSize;
	double  m_areaRatio;

	int		m_kSize;
	int		m_houghMinDist;
	int		m_houghParam1;
	int		m_houghParam2;
	int		m_houghMinR;
	int		m_houghMaxR;

	Ptr<SimpleBlobDetector> m_pBlobDetector;

	_Stream*	m_pCamStream;
	Frame*		m_pFrame;

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
