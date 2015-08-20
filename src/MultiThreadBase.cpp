/*
 * MultiThreadBase.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#include "MultiThreadBase.h"

MultiThreadBase::MultiThreadBase() {
	// TODO Auto-generated constructor stub
	pthread_mutex_init(&m_wakeupMutex, NULL);
	pthread_cond_init(&m_wakeupSignal, NULL);

}

MultiThreadBase::~MultiThreadBase() {
	// TODO Auto-generated destructor stub
	pthread_mutex_destroy(&m_wakeupMutex);
	pthread_cond_destroy(&m_wakeupSignal);
}

void MultiThreadBase::sleepThread(int32_t sec)
{
	struct timeval	now;
	struct timespec timeout;

	gettimeofday(&now,NULL);
	timeout.tv_sec = now.tv_sec + sec;
	timeout.tv_nsec = 0;//(m_now.tv_usec+usec) * 1000; //if no response request, rest for 100ms at top
	pthread_mutex_lock(&m_wakeupMutex);
	pthread_cond_timedwait(&m_wakeupSignal, &m_wakeupMutex, &timeout);
	pthread_mutex_unlock(&m_wakeupMutex);

}

void MultiThreadBase::wakeupThread(void)
{
	m_tSleep = 0;
	pthread_cond_signal(&m_wakeupSignal);
}
