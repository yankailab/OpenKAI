/*
 * _ECO.h
 *
 *  Created on: July 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__ECO_H_
#define OpenKAI_src_Tracker__ECO_H_

#include "../Base/common.h"
#include "../Base/_ObjectBase.h"

//#include "../Dependency/OpenTracker/eco/eco.hpp"

//using namespace eco;

namespace kai
{

class _ECO: public _ThreadBase
{
public:
	_ECO();
	virtual ~_ECO();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

	void startTrack(vDouble4& roi);
	void stopTrack(void);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ECO*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pVision;
//    ECO m_tracker;

    vDouble4 m_roi;
	bool m_bTracking;
};

}
#endif
