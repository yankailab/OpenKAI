/*
 * _Thread.h
 *
 *  Created on: Feb 3, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__Thread_H_
#define OpenKAI_src_Base__Thread_H_

#include "BASE.h"
#include <atomic>

namespace kai
{
	enum THREAD_STATE
	{
		thread_stop = 0,
		thread_run = 1,
		thread_sleep = 2,
		thread_pause = 3
	};

	class _Thread : public BASE
	{
	public:
		_Thread();
		virtual ~_Thread();

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(void) override;
		virtual bool startThread(void *(*__start_routine)(void *), void *__restrict __arg);
		virtual void console(void *pConsole);

		bool bStopped(void);
		bool bRunning(void);
		bool bPaused(void);

		bool bRun(void);
		void run(void);
		void pause(void);
		void stop(void);
		void join(void); // request stop and wait without cancelling the worker

		bool bOnPause(void);
		bool bOnResume(void);
		void runAllLinkedThreads(void); // wake up all the other instances

		void sleepT(int64_t nsec);
		void skipSleep(void);
		void autoFPS(void);

		uint64_t getFPS(void);
		void setTargetFPS(int fps);
		uint64_t getTargetFPS(void);
		uint64_t getDtNs(void);
		uint64_t getTfromNs(void);
		uint64_t getTtoNs(void);

	protected:
		pthread_t m_threadID = 0;
		pthread_mutex_t m_wakeupMutex;
		pthread_cond_t m_wakeupSignal;

		std::atomic<THREAD_STATE> m_setState{thread_stop};
		std::atomic<THREAD_STATE> m_state{thread_stop};

		uint64_t m_tFromNs = 0;
		uint64_t m_tToNs = 0;
		uint64_t m_targetFPS = DEFAULT_FPS;
		uint64_t m_targetTns = NSEC_SEC / m_targetFPS;
		uint64_t m_dTns = 1;
		uint64_t m_FPS = 0;
		bool m_bSkipSleep = false;

		// linked
		vector<_Thread *> m_vRunThread;
	};

}
#endif
