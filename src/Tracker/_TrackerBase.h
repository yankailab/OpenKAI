/*
 * _TrackerBase.h
 *
 *  Created on: Aug 28, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tracker__TrackerBase_H_
#define OpenKAI_src_Tracker__TrackerBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Vision/_VisionBase.h"

namespace kai
{

class _TrackerBase: public _ThreadBase
{
public:
	_TrackerBase();
	virtual
	~_TrackerBase();

	bool init(void* pKiss);
	void update(void);

	virtual void createTracker(void);
	bool updateBB(vDouble4& bb);
	bool bTracking(void);
	vDouble4* getBB(void);

public:
	_VisionBase* m_pVision;
	Rect2d m_rBB;
	vDouble4 m_bb;
	string m_trackerType;
	uint64_t m_tStampBGR;
	bool	m_bTracking;
};

}
#endif
