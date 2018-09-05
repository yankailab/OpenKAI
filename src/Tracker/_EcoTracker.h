/*
 * _EcoTracker.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__EcoTracker_H_
#define OpenKAI_src_Tracker__EcoTracker_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"
#include "_TrackerBase.h"

#ifdef USE_OPENTRACKER

#include "eco.hpp"
using namespace eco;

namespace kai
{

class _EcoTracker: public _TrackerBase
{
public:
	_EcoTracker();
	virtual	~_EcoTracker();

	bool init(void* pKiss);
	bool start(void);
	bool startTrack(vDouble4& bb);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_EcoTracker*) This)->update();
		return NULL;
	}

public:
    ECO m_eco;
	EcoParameters m_param;
	pthread_mutex_t m_ecoMutex;

};

}
#endif
#endif
