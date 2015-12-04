/*
 * ThreadBase.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_THREADBASE_H_
#define SRC_THREADBASE_H_

#include "platform.h"
#include "../Utility/util.h"

namespace kai
{

class _ThreadBase {
public:
	_ThreadBase();
	virtual ~_ThreadBase();

	void stop(void);
	bool complete(void);
	void waitForComplete(void);

	void sleepThread(int32_t sec, int32_t nsec);
	void wakeupThread(void);
	void updateTime(void);

public:
	pthread_t	m_threadID;
	bool 		m_bThreadON;

	uint64_t			m_timeStamp;
	pthread_mutex_t	m_wakeupMutex;
	pthread_cond_t	m_wakeupSignal;
	int 				m_tSleep;

};

}

#endif /* SRC_THREADBASE_H_ */
