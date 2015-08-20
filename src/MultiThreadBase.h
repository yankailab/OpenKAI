/*
 * MultiThreadBase.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_MULTITHREADBASE_H_
#define SRC_MULTITHREADBASE_H_

#include "platform.h"

class MultiThreadBase {
public:
	MultiThreadBase();
	virtual ~MultiThreadBase();

	void sleepThread(int32_t sec);
	void wakeupThread(void);

public:
	pthread_mutex_t	m_wakeupMutex;
	pthread_cond_t	m_wakeupSignal;
	int m_tSleep;
};

#endif /* SRC_MULTITHREADBASE_H_ */
