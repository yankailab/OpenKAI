/*
 * _TrafficTracker.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__TrafficTracker_H_
#define OpenKAI_src_Application__TrafficTracker_H_

#include "../Base/common.h"
#include "../Base/_ObjectBase.h"
#include "../Tracker/_SingleTracker.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _TrafficTracker : public _ObjectBase
{
public:
	_TrafficTracker();
	virtual ~_TrafficTracker();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	bool cli(int& iY);

private:
	void updateTarget(void);
	void addNewTarget(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_TrafficTracker*) This)->update();
		return NULL;
	}

public:
	_ObjectBase*	m_pOB;
	uint64_t		m_tStampOB;
	_VisionBase*	m_pVision;
	string			m_trackerType;
	double			m_oBoundary;

};

}
#endif
#endif
