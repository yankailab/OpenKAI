/*
 * _OpenTracker.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__OpenTracker_H_
#define OpenKAI_src_Tracker__OpenTracker_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"
#include "_TrackerBase.h"

#ifdef USE_OPENTRACKER

#include "eco.hpp"
using namespace eco;

namespace kai
{

class _OpenTracker: public _TrackerBase
{
public:
	_OpenTracker();
	virtual	~_OpenTracker();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool updateROI(vDouble4& roi);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OpenTracker*) This)->update();
		return NULL;
	}

public:
    ECO m_eco;

};

}
#endif
#endif
