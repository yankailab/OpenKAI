/*
 * ThreadBase.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_THREADBASE_H_
#define SRC_THREADBASE_H_

#include "common.h"
#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"

namespace kai
{

class _ThreadBase: public BASE
{
public:
	_ThreadBase();
	virtual ~_ThreadBase();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual bool start(void);
	virtual void complete(void);
	virtual bool draw(void);

	void sleepThread(int64_t usec);
	void updateTime(void);

	double getFrameRate(void);
	void setTargetFPS(int fps);
	void autoFPSfrom(void);
	void autoFPSto(void);

public:
	pthread_t m_threadID;
	bool m_bThreadON;

	int64_t m_timeStamp;
	int64_t m_dTime;

	pthread_mutex_t m_wakeupMutex;
	pthread_cond_t m_wakeupSignal;

	int64_t m_defaultFPS;
	int64_t m_targetFPS;
	int64_t m_targetFrameTime;
	int64_t m_timeFrom;
	int64_t m_timeTo;

};

}

#endif
