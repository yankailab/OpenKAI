/*
 * _Camera.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Camera_H_
#define OpenKAI_src_Vision__Camera_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#include "_VisionBase.h"

namespace kai
{

class _Camera: public _VisionBase
{
public:
	_Camera();
	virtual ~_Camera();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Camera*) This)->update();
		return NULL;
	}

public:
	int m_deviceID;
	VideoCapture m_camera;
	int m_nInitRead;
	bool m_bResetCam;
};

}
#endif
#endif
