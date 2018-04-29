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
	virtual bool draw(void);
	virtual void reset(void);

	void sleep(void);
	void wakeUp(void);
	void sleepTime(int64_t usec);
	void disableSleep(bool bDisable);

	void updateTime(void);
	double getFrameRate(void);
	void setTargetFPS(int fps);
	void autoFPSfrom(void);
	void autoFPSto(void);

public:
	pthread_t m_threadID;
	bool m_bThreadON;

	uint64_t m_tStamp;
	uint64_t m_dTime;

	pthread_mutex_t m_wakeupMutex;
	pthread_cond_t m_wakeupSignal;

	int64_t m_FPS;
	int64_t m_targetFPS;
	int64_t m_targetFrameTime;
	int64_t m_timeFrom;
	int64_t m_timeTo;
	bool	m_bSleep;
	bool	m_bDisableSleep;

};

}
#endif
