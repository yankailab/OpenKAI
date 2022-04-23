/*
 * _SingleTracker.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__SingleTracker_H_
#define OpenKAI_src_Tracker__SingleTracker_H_

#include "../Base/_ModuleBase.h"
#include "../Vision/_VisionBase.h"
#include "_TrackerBase.h"

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
	static void* getUpdate(void* This)
	{
		((_SingleTracker*) This)->update();
		return NULL;
	}

public:
	Ptr<Tracker> m_pTracker;
};

}
#endif
