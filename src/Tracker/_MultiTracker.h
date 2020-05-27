/*
 * _MultiTracker.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__MultiTracker_H_
#define OpenKAI_src_Application__MultiTracker_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_OPENCV_CONTRIB
#include "../Detector/_DetectorBase.h"
#include "../Tracker/_SingleTracker.h"

namespace kai
{

class _MultiTracker : public _DetectorBase
{
public:
	_MultiTracker();
	virtual ~_MultiTracker();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

private:
	void updateTarget(void);
	void addNewTarget(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MultiTracker*) This)->update();
		return NULL;
	}

public:
	_DetectorBase*	m_pOB;
	uint64_t		m_tStampOB;
	_VisionBase*	m_pVision;
	string			m_trackerType;
	double			m_oBoundary;

};

}
#endif
#endif
#endif
