/*
 * CameraMarkerDetect.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef DETECTOR_BGFGDETECTOR_H_
#define DETECTOR_BGFGDETECTOR_H_

#include "../Base/common.h"
#include "../Stream/_Stream.h"
#include "DetectorBase.h"
#include "../Base/_ThreadBase.h"
#include "../Base/cv.h"

#define NUM_MARKER 128
#define MIN_MARKER_SIZE 10

#define LOCK_LEVEL_NONE 0
#define LOCK_LEVEL_POS 1
#define LOCK_LEVEL_SIZE 2
#define LOCK_LEVEL_ATT 3

namespace kai
{

class _BgFg : public DetectorBase, public _ThreadBase
{
public:
	_BgFg();
	virtual ~_BgFg();

	bool init(JSON* pJson);
	bool start(void);

	//Object detection using markers
	void setObjROI(vDouble3 ROI);
	bool getObjPosition(vDouble3* pPos);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_BgFg*) This)->update();
		return NULL;
	}

public:
	int		 m_numAllMarker;
	vDouble4 m_pAllMarker[NUM_MARKER];

	//0:Not locked,1:Pos locked, 2:Size and Attitude locked
	vDouble3 m_objPos;
	vDouble3 m_objROIPos;

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
