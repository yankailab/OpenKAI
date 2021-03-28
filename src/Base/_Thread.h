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
#include "../UI/_WindowCV.h"
#include "../UI/_Console.h"

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
	virtual bool bGoSleep(void);
	virtual void sleepT (int64_t usec);
	virtual bool bSleeping(void);
	virtual void wakeUp(void);

	virtual float getFPS(void);
	virtual void setTargetFPS(float fps);
	virtual float getTargetFPS(void);
	virtual void autoFPSfrom(void);
	virtual void autoFPSto(void);
    virtual float getDt(void);

	virtual uint64_t getTfrom(void);
	virtual uint64_t getTto(void);

protected:
	pthread_t m_threadID;
	bool m_bThreadON;
	pthread_mutex_t m_wakeupMutex;
	pthread_cond_t m_wakeupSignal;

	uint64_t m_tFrom;
	uint64_t m_tTo;
	float m_targetFPS;
	float m_targetTframe;
	float m_dT;
	float m_FPS;
	bool m_bGoSleep;
	bool m_bSleeping;
    
    //Pipeline
    vector<_Thread*> m_vWakeUp;
    uint64_t m_tWakeUp;
};

}
#endif
