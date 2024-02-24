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
#include "../Utility/BitFlag.h"

namespace kai
{
	enum THREAD_BF
	{
		thread_onWakeup = 0,
		thread_onGoSleep = 1,
	};

	enum THREAD_STATE
	{
		thread_stop = 0,
		thread_run = 1,
		thread_sleep = 2,
	};

	class _Thread : public BASE
	{
	public:
		_Thread();
		virtual ~_Thread();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void *(*__start_routine)(void *), void *__restrict __arg);
		virtual void console(void *pConsole);

		virtual void run(void);
		virtual void sleep(void);
		virtual void stop(void);

		virtual bool bThread(void);
		virtual bool bRun(void);
		virtual bool bSleep(void);
		virtual bool bStop(void);

		virtual bool bOnWakeUp(void);
		virtual bool bOnGoSleep(void);
		virtual void setOnWakeUp(void);

		virtual void sleepT(int64_t usec);
		void wakeUpAll(void);	   // wake up all the other instances

		virtual void autoFPSfrom(void);
		virtual void autoFPSto(void);
		virtual float getFPS(void);
		virtual void setTargetFPS(float fps);
		virtual float getTargetFPS(void);
		virtual float getDt(void);
		virtual uint64_t getTfrom(void);
		virtual uint64_t getTto(void);

	protected:
		pthread_t m_threadID;
		pthread_mutex_t m_wakeupMutex;
		pthread_cond_t m_wakeupSignal;
		THREAD_STATE m_setState;
		THREAD_STATE m_state;
		BIT_FLAG m_bf;

		uint64_t m_tFrom;
		uint64_t m_tTo;
		float m_targetFPS;
		float m_targetTframe;
		float m_dT;
		float m_FPS;

		// linked
		vector<_Thread *> m_vTwakeUp;
	};

}
#endif
