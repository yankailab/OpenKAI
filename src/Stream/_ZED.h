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

	void getRange(double* pMin, double* pMax);
	double dist(Rect* pR);

	void startTracking(void);
	void stopTracking(void);
	vDouble3 getAccumulatedPos(void);
	void setAttitude(vDouble3* pYPR);
	bool isTracking(void);
	void setHeading(double hdgDeg);

private:
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
	sl::zed::TRACKING_STATE m_zedTrackState;
	int m_zedResolution;
	int m_zedFPS;
	int m_zedQuality;
	bool m_bZedFlip;
	double m_zedMinDist;
	double m_zedMaxDist;
	int m_zedConfidence;

	bool m_bTracking;
    sl::zed::MotionPoseData m_zedMotion;
    vDouble3 m_dMotion;
    double	m_sinHdg;
    double	m_cosHdg;
    double	m_sinHdgP;
    double	m_cosHdgP;

	Window* m_pDepthWin;

};

}

#endif
#endif
