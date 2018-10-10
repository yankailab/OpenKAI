/*
 * ThreadBase.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__ThreadBase_H_
#define OpenKAI_src_Base__ThreadBase_H_

#include "common.h"
#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"

#define T_THREAD 0
#define T_NOTHREAD 1

namespace kai
{

class _ThreadBase: public BASE
{
public:
	_ThreadBase();
	virtual ~_ThreadBase();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual bool draw(void);
	virtual bool cli(int& iY);

	virtual void goSleep(void);
	virtual void wakeUp(void);
	virtual void sleepTime(int64_t usec);
	virtual bool bSleeping(void);

	virtual void updateTime(void);
	virtual double getFrameRate(void);
	virtual void setTargetFPS(int fps);
	virtual void autoFPSfrom(void);
	virtual void autoFPSto(void);

public:
	pthread_t m_threadID;
	bool m_bThreadON;
	int m_threadMode;
	bool m_bRealTime;

	uint64_t m_tStamp;
	uint64_t m_dTime;

	pthread_mutex_t m_wakeupMutex;
	pthread_cond_t m_wakeupSignal;

	int64_t m_FPS;
	int64_t m_targetFPS;
	int64_t m_targetFrameTime;
	int64_t m_timeFrom;
	int64_t m_timeTo;
	bool	m_bGoSleep;
	bool	m_bSleeping;

	_ThreadBase* m_pWakeUp;
};

}
#endif
