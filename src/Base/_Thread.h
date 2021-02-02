/*
 * _Thread.h
 *
 *  Created on: Feb 3, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__Thread_H_
#define OpenKAI_src_Base__Thread_H_

#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"
#include "../UI/Console.h"

namespace kai
{

class _Thread: public BASE
{
public:
	_Thread();
	virtual ~_Thread();

	virtual bool init(void* pKiss);
    virtual bool start(void *(*__start_routine) (void *), void *__restrict __arg);
	virtual void draw(void);

    virtual bool bRun(void);
	virtual void goSleep(void);
	virtual void sleepTime(int64_t usec);
	virtual bool bSleeping(void);
	virtual void wakeUp(void);

	virtual float getFPS(void);
	virtual void setTargetFPS(int fps);
	virtual void autoFPSfrom(void);
	virtual void autoFPSto(void);

protected:
	pthread_t m_threadID;
	bool m_bThreadON;
	pthread_mutex_t m_wakeupMutex;
	pthread_cond_t m_wakeupSignal;

	uint64_t m_tStamp;
	uint64_t m_tFrom;
	uint64 m_tTo;
	float m_targetFPS;
	float m_targetFrameTime;
	float m_dTime;
	float m_FPS;
	bool	m_bGoSleep;
	bool	m_bSleeping;
};

}
#endif
