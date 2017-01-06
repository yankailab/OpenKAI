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

	Object* getObject(void);
	double dist(Rect* pR);

private:
	void detectObject(void);
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ZED *) This)->update();
		return NULL;
	}

public:
	sl::zed::Camera* m_pZed;
	sl::zed::SENSING_MODE m_zedMode;
	int m_zedResolution;
	int m_zedFPS;
	int m_zedQuality;
	double m_zedMinDist;
	double m_zedMaxDist;

	Object*	 m_pObj;
	bool	 m_bDetectObject;
	double	 m_alertDist;
	double	 m_detectMinSize;
	double	 m_distMinSize;
	int		 m_nObj;
	uint64_t m_oLifetime;

	//operation
	GpuMat m_Gdepth;
	GpuMat m_Gdepth2;

	bool m_bDrawContour;
	double m_contourBlend;

	Window* m_pDepthWin;

};

}

#endif
#endif
