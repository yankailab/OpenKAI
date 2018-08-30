/*
 * _MultiTracker.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__MultiTracker_H_
#define OpenKAI_src_Application__MultiTracker_H_

#include "../Base/common.h"
#include "../Base/_ObjectBase.h"
#include "../Tracker/_SingleTracker.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _MultiTracker : public _ObjectBase
{
public:
	_MultiTracker();
	virtual ~_MultiTracker();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool cli(int& iY);

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
	_ObjectBase*	m_pOB;
	uint64_t		m_tStampOB;
	_VisionBase*	m_pVision;
	string			m_trackerType;
	double			m_oBoundary;

};

}
#endif
#endif
