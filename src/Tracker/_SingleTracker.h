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
#include "../Base/_ObjectBase.h"
#include "../Vision/_VisionBase.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _SingleTracker: public _ThreadBase
{
public:
	_SingleTracker();
	virtual
	~_SingleTracker();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

	void createTracker(void);
	bool setROI(vInt4 roi);
	bool bTracking(void);

private:
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SingleTracker*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pVision;
	Rect2d m_ROI;
	Ptr<Tracker> m_pTracker;
	string m_trackerType;
	uint64_t m_tStampBGR;
	bool	m_bTracking;
	OBJECT* m_pObj;
};

}
#endif
#endif
