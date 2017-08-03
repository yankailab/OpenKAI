/*
 * Camera.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_VISION_CAMERA_H_
#define SRC_VISION_CAMERA_H_

#include "../Base/common.h"
#include "_VisionBase.h"

namespace kai
{

class _Camera: public _VisionBase
{
public:
	_Camera();
	virtual ~_Camera();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	void complete(void);

private:
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Camera *) This)->update();
		return NULL;
	}

public:
	int m_deviceID;
	VideoCapture m_camera;

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
