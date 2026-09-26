/*
 * ThreadBase.cpp
 *
 *  Created on: Feb 3, 2021
 *      Author: yankai
 */

#include "_Thread.h"
#include "../UI/_Console.h"

namespace kai
{
	static int initMonotonicCond(pthread_cond_t *pCond)
	{
		pthread_condattr_t attr;

		int r = pthread_condattr_init(&attr);
		if (r != 0)
			return r;

		r = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
		if (r == 0)
		{
			r = pthread_cond_init(pCond, &attr);
		}

		pthread_condattr_destroy(&attr);
		return r;
	}

	_Thread::_Thread()
	{
		m_class = "_Thread";

		pthread_mutex_init(&m_wakeupMutex, NULL);

		initMonotonicCond(&m_wakeupSignal);
	}

	_Thread::~_Thread()
	{
		if (m_threadID != 0)
		{
			stop();
			pthread_cancel(m_threadID);
			pthread_join(m_threadID, NULL);
			m_threadID = 0;
		}

		pthread_mutex_destroy(&m_wakeupMutex);
		pthread_cond_destroy(&m_wakeupSignal);
	}

	bool _Thread::loadConfig(void)
	{
		IF_F(!this->BASE::loadConfig());
		json &j = *m_pJ;

		float FPS = DEFAULT_FPS;
		jKv(j, "FPS", FPS);
		setTargetFPS(FPS);

		return true;
	}

	bool _Thread::saveConfig(bool bExport)
	{
		if (!BASE::saveConfig(false))
		{
			return false;
		}

		(*m_pJ)["FPS"] = m_targetFPS;
		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _Thread::link(void)
	{
		IF_F(!this->BASE::link());
		const json &j = *m_pJ;

		vector<string> vRunT;
		jKv(j, "vRunThread", vRunT);
		m_vRunThread.clear();
		for (string s : vRunT)
		{
			_Thread *pT = (_Thread *)(m_pM->findModule(s));
			if (!pT)
			{
				LOG_I("Instance not found: " + s);
				continue;
			}

			m_vRunThread.push_back(pT);
		}

		return true;
	}

	bool _Thread::startThread(void *(*__start_routine)(void *),
							  void *__restrict __arg)
	{
		IF_F(m_threadID != 0);

		m_setState = thread_run;
		m_tFromNs = getTns();

		int r = pthread_create(&m_threadID, 0, __start_routine, __arg);
		IF_F(r != 0);

		m_state = thread_run;
		return true;
	}

	bool _Thread::bStopped(void)
	{
		return (m_state == thread_stop);
	}

	bool _Thread::bRunning(void)
	{
		return (m_state == thread_run);
	}

	bool _Thread::bPaused(void)
	{
		return (m_state == thread_pause);
	}

	bool _Thread::bRun(void)
	{
		return (m_setState != thread_stop);
	}

	void _Thread::run(void)
	{
		m_setState = thread_run;
		pthread_cond_signal(&m_wakeupSignal);
	}

	void _Thread::pause(void)
	{
		m_setState = thread_pause;
	}

	void _Thread::stop(void)
	{
		pthread_mutex_lock(&m_wakeupMutex);
		m_setState = thread_stop;
		pthread_cond_signal(&m_wakeupSignal);
		pthread_mutex_unlock(&m_wakeupMutex);
	}

	void _Thread::join(void)
	{
		stop();
		if (m_threadID != 0 && !pthread_equal(m_threadID, pthread_self()))
		{
			pthread_join(m_threadID, NULL);
			m_threadID = 0;
			m_state = thread_stop;
		}
	}

	bool _Thread::bOnPause(void)
	{
		IF_F(m_setState != thread_pause);
		IF_F(m_state == thread_pause);

		m_state = thread_pause;
		return true;
	}

	bool _Thread::bOnResume(void)
	{
		IF_F(m_state != thread_pause);
		m_state = thread_run;

		return true;
	}

	void _Thread::runAllLinkedThreads(void)
	{
		for (_Thread *pT : m_vRunThread)
			pT->run();
	}

	void _Thread::sleepT(int64_t nsec)
	{
		m_state = thread_sleep;

		if (nsec > 0)
		{
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);

			uint64_t dS = nsec / NSEC_SEC;
			ts.tv_sec += dS;
			ts.tv_nsec += nsec - (dS * NSEC_SEC);

			while (ts.tv_nsec >= NSEC_SEC)
			{
				ts.tv_sec++;
				ts.tv_nsec -= NSEC_SEC;
			}

			pthread_mutex_lock(&m_wakeupMutex);
			if (m_setState != thread_stop)
				pthread_cond_timedwait(&m_wakeupSignal, &m_wakeupMutex, &ts);
			pthread_mutex_unlock(&m_wakeupMutex);
		}
		else
		{
			pthread_mutex_lock(&m_wakeupMutex);
			if (m_setState != thread_stop)
				pthread_cond_wait(&m_wakeupSignal, &m_wakeupMutex);
			pthread_mutex_unlock(&m_wakeupMutex);
		}

		m_state = thread_run;
	}

	void _Thread::skipSleep(void)
	{
		m_bSkipSleep = true;
	}

	void _Thread::autoFPS(void)
	{
		m_tToNs = getTns();

		if (!m_bSkipSleep)
		{
			const uint64_t elapsedNs = m_tToNs - m_tFromNs;
			if (elapsedNs < m_targetTns)
			{
				const uint64_t sleepNs = m_targetTns - elapsedNs;
				if (sleepNs > NSEC_MSEC)
				{
					sleepT(sleepNs);
				}
			}
		}
		else
		{
			m_bSkipSleep = false;
		}

		if (m_setState == thread_pause)
		{
			m_FPS = 0;
			sleepT(0);
			m_tFromNs = getTns();
		}

		uint64_t tNow = getTns();
		m_dTns = tNow - m_tFromNs + 1;
		m_tFromNs = tNow;
		m_FPS = NSEC_SEC / m_dTns;
	}

	uint64_t _Thread::getFPS(void)
	{
		return m_FPS;
	}

	void _Thread::setTargetFPS(int fps)
	{
		IF_(fps <= 0);

		m_targetFPS = fps;
		m_targetTns = NSEC_SEC / m_targetFPS;
	}

	uint64_t _Thread::getTargetFPS(void)
	{
		return m_targetFPS;
	}

	uint64_t _Thread::getTfromNs(void)
	{
		return m_tFromNs;
	}

	uint64_t _Thread::getTtoNs(void)
	{
		return m_tToNs;
	}

	uint64_t _Thread::getDtNs(void)
	{
		return m_dTns;
	}

	void _Thread::console(void *pConsole)
	{
		NULL_(pConsole);

		string msg = "FPS: " + i2str(m_FPS);
		string t = " " + this->getName();

		_Console *pC = (_Console *)pConsole;
		pC->addMsg(t, COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
		pC->addMsg(msg, COLOR_PAIR(_Console_COL_FPS) | A_BOLD, _Console_X_FPS);
	}
}
