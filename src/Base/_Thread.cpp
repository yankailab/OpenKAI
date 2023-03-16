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
		m_bThreadON = false;
		m_threadID = 0;
		m_dT = 1.0;
		m_FPS = 0;
		m_targetFPS = DEFAULT_FPS;
		m_targetTframe = SEC_2_USEC / m_targetFPS;
		m_tFrom = 0;
		m_tTo = 0;
		m_bGoSleep = false;
		m_bSleeping = false;
		m_bWakeUp = false;

		pthread_mutex_init(&m_wakeupMutex, NULL);
		pthread_cond_init(&m_wakeupSignal, NULL);
	}

	_Thread::~_Thread()
	{
		m_bThreadON = false;
		IF_(m_threadID == 0);
		pthread_cancel(m_threadID);
		//	pthread_join(m_threadID, NULL);
		m_threadID = 0;

		pthread_mutex_destroy(&m_wakeupMutex);
		pthread_cond_destroy(&m_wakeupSignal);
	}

	bool _Thread::init(void *pKiss)
	{
		IF_F(!this->BASE::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		float FPS = DEFAULT_FPS;
		pK->v("FPS", &FPS);
		setTargetFPS(FPS);

		return true;
	}

    bool _Thread::link(void)
    {
        IF_F(!this->BASE::link());

        Kiss *pK = (Kiss *)m_pKiss;
		vector<string> vWakeUp;
		pK->a("vTwakeUp", &vWakeUp);
		for (string s : vWakeUp)
		{
			_Thread *pT = (_Thread *)(pK->getInst(s));
            NULL_d_F(pT, LOG_E("Instance not found: " + s));
			m_vTwakeUp.push_back(pT);
		}

        return true;
    }

	bool _Thread::start(void *(*__start_routine)(void *),
						void *__restrict __arg)
	{
		IF_F(m_bThreadON);

		m_tFrom = getApproxTbootUs();

		m_bThreadON = true;
		int r = pthread_create(&m_threadID, 0, __start_routine, __arg);
		if (r != 0)
		{
			m_bThreadON = false;
			return false;
		}

		return true;
	}

	bool _Thread::bRun(void)
	{
		return m_bThreadON;
	}

	void _Thread::goSleep(void)
	{
		m_bGoSleep = true;
	}

	bool _Thread::bGoSleep(void)
	{
		return m_bGoSleep;
	}

	void _Thread::sleepT(int64_t usec)
	{
		if (usec > 0)
		{
			struct timeval tNow;
			gettimeofday(&tNow, NULL);

			int64_t nsec = (tNow.tv_usec + usec) * 1000;
			int64_t sec = nsec / NSEC_1SEC;

			struct timespec tTimeout;
			tTimeout.tv_sec = tNow.tv_sec + sec;
			tTimeout.tv_nsec = nsec - sec * NSEC_1SEC; // % NSEC_1SEC;

			m_bSleeping = true;
			pthread_mutex_lock(&m_wakeupMutex);
			pthread_cond_timedwait(&m_wakeupSignal, &m_wakeupMutex, &tTimeout);
			pthread_mutex_unlock(&m_wakeupMutex);
		}
		else
		{
			m_bSleeping = true;
			pthread_mutex_lock(&m_wakeupMutex);
			pthread_cond_wait(&m_wakeupSignal, &m_wakeupMutex);
			pthread_mutex_unlock(&m_wakeupMutex);
		}

		m_bSleeping = false;
	}

	bool _Thread::bSleeping(void)
	{
		return m_bSleeping;
	}

	void _Thread::wakeUp(void)
	{
		m_bWakeUp = true;
		m_bGoSleep = false;
		pthread_cond_signal(&m_wakeupSignal);
	}

	void _Thread::wakeUpAll(void)
	{
		for(_Thread* pT : m_vTwakeUp)
			pT->wakeUp();
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

	void _Thread::autoFPSfrom(void)
	{
		m_bWakeUp = false;
		uint64_t tNow = getApproxTbootUs();
		m_dT = (float)(tNow - m_tFrom + 1);
		m_tFrom = tNow;
		m_FPS = SEC_2_USEC / m_dT;
	}

	void _Thread::autoFPSto(void)
	{
		m_tTo = getApproxTbootUs();

		IF_d_(m_bWakeUp, m_bWakeUp = false);

		int uSleep = (int)(m_targetTframe - (m_tTo - m_tFrom));
		if (uSleep > 1000)
		{
			sleepT(uSleep);
		}

		if (m_bGoSleep)
		{
			IF_d_(m_bWakeUp, m_bWakeUp = false);
			m_FPS = 0;
			sleepT(0);
		}
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
		string t = " " + *this->getName();

		_Console *pC = (_Console *)pConsole;
		pC->addMsg(t, COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
		pC->addMsg(msg, COLOR_PAIR(_Console_COL_FPS) | A_BOLD, _Console_X_FPS);
	}

}
