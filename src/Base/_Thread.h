/*
 * _Thread.h
 *
 *  Created on: Feb 3, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__Thread_H_
#define OpenKAI_src_Base__Thread_H_

#include "BASE.h"
#include "../Module/Kiss.h"

namespace kai
{
	enum THREAD_STATE
	{
		thread_stop = 0,
		thread_run = 1,
		thread_sleep = 2,
		thread_pause = 3,
		thread_resume = 4
	};

	class _Thread : public BASE
	{
	public:
		_Thread();
		virtual ~_Thread();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void *(*__start_routine)(void *), void *__restrict __arg);
		virtual void console(void *pConsole);

		bool bAlive(void);
		bool bRun(void);
		bool bStop(void);

		void run(void);
		void pause(void);
		void stop(void);

		bool bOnPause(void);
		bool bOnResume(void);
		void runAll(void);	   // wake up all the other instances

		void sleepT(int64_t usec);
		void autoFPSfrom(void);
		void autoFPSto(void);
		float getFPS(void);
		void setTargetFPS(float fps);
		float getTargetFPS(void);
		float getDt(void);
		uint64_t getTfrom(void);
		uint64_t getTto(void);

	protected:
		pthread_t m_threadID;
		pthread_mutex_t m_wakeupMutex;
		pthread_cond_t m_wakeupSignal;

		THREAD_STATE m_setState;
		THREAD_STATE m_state;
		bool m_bPaused;

		uint64_t m_tFrom;
		uint64_t m_tTo;
		float m_targetFPS;
		float m_targetTframe;
		float m_dT;
		float m_FPS;

		// linked
		vector<_Thread *> m_vRunThread;
	};

}
#endif
