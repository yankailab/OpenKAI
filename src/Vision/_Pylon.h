/*
 * _Pylon.h
 *
 *  Created on: Apr 9, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Pylon_H_
#define OpenKAI_src_Vision__Pylon_H_

#include "../Base/common.h"
#include "_VisionBase.h"

#ifdef USE_PYLON
#include <pylon/PylonIncludes.h>

using namespace Pylon;

namespace kai
{

class _Pylon: public _VisionBase
{
public:
	_Pylon();
	virtual ~_Pylon();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	void reset(void);

private:
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Pylon *) This)->update();
		return NULL;
	}

public:
	PylonAutoInitTerm m_pylonAutoInit;
	CInstantCamera* m_pPylonCam;
	CImageFormatConverter m_pylonFC;
	CPylonImage m_pylonImg;

	//calibration
	bool m_bCalibration;
	bool m_bFisheye;
	Mat m_cameraMat;
	Mat m_distCoeffs;
	GpuMat m_Gmap1;
	GpuMat m_Gmap2;

	//crop
	bool m_bCrop;
	Rect m_cropBB;

};

}

#endif
#endif
