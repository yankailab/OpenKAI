/*
 * ThreadBase.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_THREADBASE_H_
#define SRC_THREADBASE_H_

#include "platform.h"

class ThreadBase {
public:
	ThreadBase();
	virtual ~ThreadBase();

	void sleepThread(int32_t sec, int32_t nsec);
	void wakeupThread(void);

public:
	pthread_mutex_t	m_wakeupMutex;
	pthread_cond_t	m_wakeupSignal;
	int m_tSleep;
};

#endif /* SRC_THREADBASE_H_ */
