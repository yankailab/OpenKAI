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
	BASE();

	m_bThreadON = false;
	m_threadID = 0;
	m_timeStamp = 0;
	m_dTime = 1.0;
	m_defaultFPS = DEFAULT_FPS;
	m_targetFPS = m_defaultFPS;
	m_targetFrameTime = USEC_1SEC / m_targetFPS;
	m_timeFrom = 0;
	m_timeTo = 0;

	pthread_mutex_init(&m_wakeupMutex, NULL);
	pthread_cond_init(&m_wakeupSignal, NULL);

}

_ThreadBase::~_ThreadBase()
{
	pthread_mutex_destroy(&m_wakeupMutex);
	pthread_cond_destroy(&m_wakeupSignal);
}

bool _ThreadBase::init(void* pKiss)
{
	CHECK_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	int FPS = DEFAULT_FPS;
	F_INFO(pK->v("FPS", &FPS));
	setTargetFPS(FPS);

	return true;
}

bool _ThreadBase::link(void)
{
	CHECK_F(!this->BASE::link());
	return true;
}

bool _ThreadBase::start(void)
{
	return true;
}

void _ThreadBase::sleepThread(int64_t usec)
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

void _ThreadBase::wakeupThread(void)
{
	pthread_cond_signal(&m_wakeupSignal);
}

void _ThreadBase::updateTime(void)
{
	uint64_t newTime = get_time_usec();
	m_dTime = newTime - m_timeStamp;
	m_timeStamp = newTime;
}

double _ThreadBase::getFrameRate(void)
{
	return USEC_1SEC / m_dTime;
}

void _ThreadBase::setTargetFPS(int fps)
{
	CHECK_(fps<=0);

	m_targetFPS = fps;
	m_targetFrameTime = USEC_1SEC / m_targetFPS;
	m_dTime = m_targetFrameTime;
}

void _ThreadBase::autoFPSfrom(void)
{
	m_timeFrom = get_time_usec();
}

void _ThreadBase::autoFPSto(void)
{
	m_timeTo = get_time_usec();

	int uSleep = (int) (m_targetFrameTime - (m_timeTo - m_timeFrom));
	if (uSleep > 1000)
	{
		this->sleepThread(uSleep);
	}

	this->updateTime();
}

void _ThreadBase::complete(void)
{
	m_bThreadON = false;
	this->wakeupThread();
}

void _ThreadBase::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

bool _ThreadBase::draw(void)
{
	CHECK_F(!this->BASE::draw());

	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	pWin->tabReset();

	putText(*pMat,
			*this->getName() + " FPS: " + i2str(getFrameRate()),
			*pWin->getTextPos(), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pWin->lineNext();

	return true;
}

}
