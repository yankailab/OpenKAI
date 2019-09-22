/*
 * _SingleTracker.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__SingleTracker_H_
#define OpenKAI_src_Tracker__SingleTracker_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"
#include "_TrackerBase.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _SingleTracker: public _TrackerBase
{
public:
	_SingleTracker();
	virtual	~_SingleTracker();

	bool init(void* pKiss);
	bool start(void);

	void createTracker(void);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SingleTracker*) This)->update();
		return NULL;
	}

public:
	Ptr<Tracker> m_pTracker;
};

}
#endif
#endif
