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
	_Thread::_Thread()
	{
		m_threadID = 0;
		m_dT = 1.0;
		m_FPS = 0;
		m_targetFPS = DEFAULT_FPS;
		m_targetTframe = SEC_2_USEC / m_targetFPS;
		m_tFrom = 0;
		m_tTo = 0;

		m_state = thread_stop;
		m_setState = thread_stop;
		m_bPaused = false;
		m_bSkipSleep = false;

		pthread_mutex_init(&m_wakeupMutex, NULL);
		pthread_cond_init(&m_wakeupSignal, NULL);
	}

	_Thread::~_Thread()
	{
		IF_(m_threadID == 0);
		pthread_cancel(m_threadID);
		//	pthread_join(m_threadID, NULL);
		m_threadID = 0;

		pthread_mutex_destroy(&m_wakeupMutex);
		pthread_cond_destroy(&m_wakeupSignal);
	}

	int _Thread::init(void *pKiss)
	{
		CHECK_(this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		float FPS = DEFAULT_FPS;
		pK->v("FPS", &FPS);
		setTargetFPS(FPS);

		return OK_OK;
	}

	int _Thread::link(void)
	{
		CHECK_(this->BASE::link());

		Kiss *pK = (Kiss *)m_pKiss;
		vector<string> vRunT;
		pK->a("vRunThread", &vRunT);

		m_vRunThread.clear();
		for (string s : vRunT)
		{
			_Thread *pT = (_Thread *)(pK->findModule(s));
			if (!pT)
			{
				LOG_I("Instance not found: " + s);
				continue;
			}

			m_vRunThread.push_back(pT);
		}

		return OK_OK;
	}

	int _Thread::start(void *(*__start_routine)(void *),
					   void *__restrict __arg)
	{
		IF__(m_threadID != 0, OK_ERR_DUPLICATE);

		m_setState = thread_run;
		m_tFrom = getApproxTbootUs();

		int r = pthread_create(&m_threadID, 0, __start_routine, __arg);
		IF__(r != 0, r);

		m_state = thread_run;
		return OK_OK;
	}

	bool _Thread::bAlive(void)
	{
		return (m_setState != thread_stop);
	}

	bool _Thread::bRun(void)
	{
		return (m_state == thread_run);
	}

	bool _Thread::bStop(void)
	{
		return (m_state == thread_stop);
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
		m_setState = thread_stop;
	}

	bool _Thread::bOnPause(void)
	{
		IF_F(m_bPaused);
		IF_F(m_setState != thread_pause);

		m_bPaused = true;
		return true;
	}

	bool _Thread::bOnResume(void)
	{
		IF_F(!m_bPaused);

		m_bPaused = false;
		return true;
	}

	void _Thread::runAll(void)
	{
		for (_Thread *pT : m_vRunThread)
			pT->run();
	}

	void _Thread::sleepT(int64_t usec)
	{
		m_state = thread_sleep;

		if (usec > 0)
		{
			struct timeval tNow;
			gettimeofday(&tNow, NULL);

			int64_t nsec = (tNow.tv_usec + usec) * 1000;
			int64_t sec = nsec / NSEC_1SEC;

			struct timespec tTimeout;
			tTimeout.tv_sec = tNow.tv_sec + sec;
			tTimeout.tv_nsec = nsec - sec * NSEC_1SEC; // % NSEC_1SEC;

			pthread_mutex_lock(&m_wakeupMutex);
			pthread_cond_timedwait(&m_wakeupSignal, &m_wakeupMutex, &tTimeout);
			pthread_mutex_unlock(&m_wakeupMutex);
		}
		else
		{
			pthread_mutex_lock(&m_wakeupMutex);
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
		m_tTo = getApproxTbootUs();

		if (!m_bSkipSleep)
		{
			int uSleep = (int)(m_targetTframe - (m_tTo - m_tFrom));
			if (uSleep > 1000)
			{
				sleepT(uSleep);
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
		}

		uint64_t tNow = getApproxTbootUs();
		m_dT = (float)(tNow - m_tFrom + 1);
		m_tFrom = tNow;
		m_FPS = SEC_2_USEC / m_dT;
	}

	float _Thread::getFPS(void)
	{
		return m_FPS;
	}

	void _Thread::setTargetFPS(float fps)
	{
		IF_(fps <= 0);

		m_targetFPS = fps;
		m_targetTframe = SEC_2_USEC / m_targetFPS;
	}

	float _Thread::getTargetFPS(void)
	{
		return m_targetFPS;
	}

	uint64_t _Thread::getTfrom(void)
	{
		return m_tFrom;
	}

	uint64_t _Thread::getTto(void)
	{
		return m_tTo;
	}

	float _Thread::getDt(void)
	{
		return m_dT;
	}

	void _Thread::console(void *pConsole)
	{
		NULL_(pConsole);

		string msg = "FPS: " + f2str(m_FPS, 2);
		string t = " " + this->getName();

		_Console *pC = (_Console *)pConsole;
		pC->addMsg(t, COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
		pC->addMsg(msg, COLOR_PAIR(_Console_COL_FPS) | A_BOLD, _Console_X_FPS);
	}

}
