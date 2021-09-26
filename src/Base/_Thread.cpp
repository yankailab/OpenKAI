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
		m_tWakeUp = 0;

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

		vector<string> vWakeup;
		pK->a("vWakeup", &vWakeup);
		for (int i = 0; i < vWakeup.size(); i++)
		{
			_Thread *pT = (_Thread *)(pK->getInst(vWakeup[i]));
			IF_CONT(!pT);
			m_vWakeUp.push_back(pT);
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
		m_tWakeUp = m_tFrom;
		m_bGoSleep = false;
		pthread_cond_signal(&m_wakeupSignal);
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
		uint64_t tNow = getApproxTbootUs();
		m_dT = (float)(tNow - m_tFrom + 1);
		m_tFrom = tNow;
		m_FPS = SEC_2_USEC / m_dT;
	}

	void _Thread::autoFPSto(void)
	{
		//Pipeline wakeup
		if (!m_vWakeUp.empty())
		{
			for (int i = 0; i < m_vWakeUp.size(); i++)
				m_vWakeUp[i]->wakeUp();
		}

		m_tTo = getApproxTbootUs();

		int uSleep = (int)(m_targetTframe - (m_tTo - m_tFrom));
		if (uSleep > 1000)
		{
			sleepT(uSleep);
		}

		if (m_bGoSleep)
		{
			IF_(m_tFrom <= m_tWakeUp);
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
#ifdef WITH_UI
		NULL_(pConsole);

		string msg = "FPS: " + f2str(m_FPS, 2);
		string t = " " + *this->getName();

		_Console *pC = (_Console *)pConsole;
		pC->addMsg(t, COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
		pC->addMsg(msg, COLOR_PAIR(_Console_COL_FPS) | A_BOLD, _Console_X_FPS);
#endif
	}

}
