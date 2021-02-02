/*
 * ThreadBase.cpp
 *
 *  Created on: Feb 3, 2021
 *      Author: yankai
 */

#include "_Thread.h"

namespace kai
{

_Thread::_Thread()
{
	m_bThreadON = false;
	m_threadID = 0;
	m_tStamp = 0;
	m_dTime = 1.0;
	m_FPS = 0;
	m_targetFPS = DEFAULT_FPS;
	m_targetFrameTime = USEC_1SEC / m_targetFPS;
	m_tFrom = 0;
	m_tTo = 0;
	m_bGoSleep = false;
	m_bSleeping = false;

	pthread_mutex_init(&m_wakeupMutex, NULL);
	pthread_cond_init(&m_wakeupSignal, NULL);
}

_Thread::~_Thread()
{
	m_bThreadON = false;
	IF_(m_threadID==0);
	pthread_cancel(m_threadID);
//	pthread_join(m_threadID, NULL);
	m_threadID = 0;

	pthread_mutex_destroy(&m_wakeupMutex);
	pthread_cond_destroy(&m_wakeupSignal);
}

bool _Thread::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	int FPS = DEFAULT_FPS;
	pK->v("FPS", &FPS);
	setTargetFPS(FPS);

	return true;
}

bool _Thread::start(void *(*__start_routine) (void *),
                    void *__restrict __arg)
{
	m_bThreadON = true;
	int r = pthread_create(&m_threadID, 0, __start_routine, __arg);
	if (r != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Thread::sleepTime(int64_t usec)
{
	if(usec>0)
	{
		struct timeval now;
		struct timespec timeout;

		gettimeofday(&now, NULL);
		int64_t nsec = (now.tv_usec + usec) * 1000;
		timeout.tv_sec = now.tv_sec + nsec / NSEC_1SEC;
		timeout.tv_nsec = nsec % NSEC_1SEC;

		m_bSleeping = true;
		pthread_mutex_lock(&m_wakeupMutex);
		pthread_cond_timedwait(&m_wakeupSignal, &m_wakeupMutex, &timeout);
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

bool _Thread::bRun(void)
{
    return m_bThreadON;
}

void _Thread::goSleep(void)
{
	m_bGoSleep = true;
}

bool _Thread::bSleeping(void)
{
	return m_bSleeping;
}

void _Thread::wakeUp(void)
{
	m_bGoSleep = false;
	pthread_cond_signal(&m_wakeupSignal);
}

float _Thread::getFPS(void)
{
	return m_FPS;
}

void _Thread::setTargetFPS(int fps)
{
	IF_(fps<=0);

	m_targetFPS = fps;
	m_targetFrameTime = USEC_1SEC / m_targetFPS;
}

void _Thread::autoFPSfrom(void)
{
	m_tFrom = getTimeUsec();
	m_dTime = (float)(m_tFrom - m_tStamp + 1);
	m_tStamp = m_tFrom;
	m_FPS = USEC_1SEC / m_dTime;
}

void _Thread::autoFPSto(void)
{
	m_tTo = getTimeUsec();

	int uSleep = (int) (m_targetFrameTime - (m_tTo - m_tFrom));
	if (uSleep > 1000)
	{
		this->sleepTime(uSleep);
	}

	if(m_bGoSleep)
	{
		m_FPS = 0;
		this->sleepTime(0);
	}
}

void _Thread::draw(void)
{
	this->BASE::draw();

	string msg = "FPS: " + i2str((int)m_FPS);

	if(m_pConsole)
	{
		Console* pC = (Console*)m_pConsole;
		pC->addMsg(msg, COLOR_PAIR(CONSOLE_COL_FPS)|A_BOLD, CONSOLE_X_FPS);
	}

#ifdef USE_OPENCV
	if(checkWindow())
	{
		Window* pWin = (Window*)this->m_pWindow;
		pWin->addMsg(msg, 1);
	}
#endif
}

}
