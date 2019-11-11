/*
 * _Mynteye.h
 *
 *  Created on: Nov 9, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Mynteye_H_
#define OpenKAI_src_Vision__Mynteye_H_

#include "../Base/common.h"
#include "_DepthVisionBase.h"
#include "../Utility/util.h"

#ifdef USE_MYNTEYE
#define WITH_OPENCV
#include <mynteyed/camera.h>
#include <mynteyed/utils.h>

MYNTEYE_USE_NAMESPACE

namespace kai
{

class _Mynteye: public _DepthVisionBase
{
public:
	_Mynteye();
	virtual ~_Mynteye();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	bool open(void);
	void close(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Mynteye *) This)->update();
		return NULL;
	}

public:
	Camera m_me;
	DeviceInfo m_devInfo;
	OpenParams m_op;

};

}
#endif
#endif
