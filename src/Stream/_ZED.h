/*
 * ZED.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_STREAM_ZED_H_
#define SRC_STREAM_ZED_H_

#include "../Base/common.h"
#include "_StreamBase.h"

#ifdef USE_ZED
#include <zed/Camera.hpp>

namespace kai
{

class _ZED: public _StreamBase
{
public:
	_ZED();
	virtual ~_ZED();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ZED *) This)->update();
		return NULL;
	}

private:
	sl::zed::Camera* m_pZed;
	sl::zed::SENSING_MODE m_zedMode;
	int m_zedResolution;
	double m_zedMinDist;
	int m_zedFPS;
	Mat m_depthMat;

	//operation
	GpuMat m_Gdepth;
	GpuMat m_Gdepth2;

	Window* m_pDepthWin;

};

}

#endif
#endif
