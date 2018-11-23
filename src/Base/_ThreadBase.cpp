/*
 * ThreadBase.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#include "_ThreadBase.h"

namespace kai
{

_ThreadBase::_ThreadBase()
{
	m_bThreadON = false;
	m_threadID = 0;
	m_tStamp = 0;
	m_dTime = 1.0;
	m_FPS = 0;
	m_targetFPS = DEFAULT_FPS;
	m_targetFrameTime = USEC_1SEC / m_targetFPS;
	m_timeFrom = 0;
	m_timeTo = 0;
	m_bGoSleep = false;
	m_bSleeping = false;
	m_pWakeUp = NULL;
	m_threadMode = T_THREAD;
	m_bRealTime = true;

	pthread_mutex_init(&m_wakeupMutex, NULL);
	pthread_cond_init(&m_wakeupSignal, NULL);
}

_ThreadBase::~_ThreadBase()
{
	m_bThreadON = false;
	IF_(m_threadID==0);
	pthread_cancel(m_threadID);
//	pthread_join(m_threadID, NULL);
	m_threadID = 0;

	pthread_mutex_destroy(&m_wakeupMutex);
	pthread_cond_destroy(&m_wakeupSignal);
}

bool _ThreadBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, threadMode);
	KISSm(pK, bRealTime);

	int FPS = DEFAULT_FPS;
	pK->v("FPS", &FPS);
	setTargetFPS(FPS);

	string iName = "";
	pK->v("_wakeUp", &iName);
	m_pWakeUp = (_ThreadBase*) (pK->root()->getChildInst(iName));

	return true;
}

bool _ThreadBase::start(void)
{
	IF_F(m_threadMode != T_THREAD);
	return true;
}

void _ThreadBase::sleepTime(int64_t usec)
{
	if(usec>0)
	{
		struct timeval now;
		struct timespec timeout;

		gettimeofday(&now, NULL);
		int64_t nsec = (now.tv_usec + usec) * 1000;
		timeout.tv_sec = now.tv_sec + nsec / NSEC_1SEC;
		timeout.tv_nsec = nsec % NSEC_1SEC;

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
}

void _ThreadBase::goSleep(void)
{
	m_bGoSleep = true;
}

bool _ThreadBase::bSleeping(void)
{
	return m_bSleeping;
}

void _ThreadBase::wakeUp(void)
{
	m_bGoSleep = false;
	m_bSleeping = false;
	pthread_cond_signal(&m_wakeupSignal);
}

void _ThreadBase::wakeUpLinked(void)
{
	NULL_(m_pWakeUp);

	m_pWakeUp->wakeUp();
}

void _ThreadBase::updateTime(void)
{
	uint64_t newTime = getTimeUsec();
	m_dTime = newTime - m_tStamp;
	m_tStamp = newTime;
	m_FPS = USEC_1SEC / m_dTime;
}

double _ThreadBase::getFrameRate(void)
{
	return m_FPS;
}

void _ThreadBase::setTargetFPS(int fps)
{
	IF_(fps<=0);

	m_targetFPS = fps;
	m_targetFrameTime = USEC_1SEC / m_targetFPS;
}

void _ThreadBase::autoFPSfrom(void)
{
	m_timeFrom = getTimeUsec();
}

void _ThreadBase::autoFPSto(void)
{
	m_timeTo = getTimeUsec();
	this->updateTime();

	int uSleep = (int) (m_targetFrameTime - (m_timeTo - m_timeFrom));
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

bool _ThreadBase::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	pWin->tabReset();

	string msg = *this->getName();
	msg += " FPS: " + i2str(m_FPS);

	pWin->addMsg(&msg);

	return true;
}

bool _ThreadBase::console(int& iY)
{
	IF_F(!this->BASE::console(iY));

	string msg = "FPS: " + i2str(m_FPS);
	COL_FPS;
	mvaddstr(iY, CLI_X_FPS, msg.c_str());

	return true;
}

}
