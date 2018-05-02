/*
 * _Camera.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Camera_H_
#define OpenKAI_src_Vision__Camera_H_

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
	void reset(void);

private:
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Camera*) This)->update();
		return NULL;
	}

	void updateTPP(void);
	static void* getTPP(void* This)
	{
		((_Camera*) This)->updateTPP();
		return NULL;
	}

public:
	int m_deviceID;
	VideoCapture m_camera;
};

}

#endif
