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
#include "../Stream/Frame.h"

#define NUM_MUTEX 5

namespace kai
{

class _ThreadBase: public BASE
{
public:
	_ThreadBase();
	virtual ~_ThreadBase();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	void complete(void);
	void waitForComplete(void);

	void sleepThread(int32_t sec, int32_t nsec);
	void wakeupThread(void);
	void updateTime(void);
	bool mutexTrylock(int mutex);
	void mutexLock(int mutex);
	void mutexUnlock(int mutex);

	double getFrameRate(void);

	void setTargetFPS(double fps);
	void autoFPSfrom(void);
	void autoFPSto(void);

	virtual bool start(void);
	virtual bool draw(Frame* pFrame, vInt4* pTextPos);

public:
	pthread_t m_threadID;
	bool m_bThreadON;

	uint64_t m_timeStamp;
	double m_dTime;
	double m_dTimeAvr;

	pthread_mutex_t m_wakeupMutex;
	pthread_cond_t m_wakeupSignal;
	pthread_mutex_t m_mutex[NUM_MUTEX];

	double m_defaultFPS;
	double m_targetFPS;
	double m_targetFrameTime;
	double m_timeFrom;
	double m_timeTo;

};

}

#endif /* SRC_THREADBASE_H_ */
