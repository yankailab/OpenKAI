/*
 * CameraMarkerDetect.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef DETECTOR_BGFGDETECTOR_H_
#define DETECTOR_BGFGDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Camera/_Stream.h"
#include "DetectorBase.h"
#include "../Base/_ThreadBase.h"

#define NUM_MARKER 128
#define MIN_MARKER_SIZE 10

#define LOCK_LEVEL_NONE 0
#define LOCK_LEVEL_POS 1
#define LOCK_LEVEL_SIZE 2
#define LOCK_LEVEL_ATT 3

namespace kai
{

class _BgFgDetector : public DetectorBase, public _ThreadBase
{
public:
	_BgFgDetector();
	virtual ~_BgFgDetector();

	bool init(JSON* pJson, string name);
	bool start(void);

	//Object detection using markers
	void setObjROI(fVector3 ROI);
	bool getObjPosition(fVector3* pPos);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_BgFgDetector*) This)->update();
		return NULL;
	}

public:
	int		 m_numAllMarker;
	fVector4 m_pAllMarker[NUM_MARKER];

	//0:Not locked,1:Pos locked, 2:Size and Attitude locked
	fVector3 m_objPos;
	fVector3 m_objROIPos;

	double	m_minSize;
	int		m_objLockLevel;

	_Stream*		m_pCamStream;
	UMat				m_Mat;

#ifdef USE_CUDA
	int		m_cudaDeviceID;

	GpuMat m_gFg;
    GpuMat m_gBg;

    Ptr<BackgroundSubtractor> m_pBgSubtractor;
#else

#endif
};

} /* namespace kai */

#endif /* SRC_MarkerDetector_H_ */
