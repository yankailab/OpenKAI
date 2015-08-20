/*
 * ObjectLocalizer.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef SRC_OBJECTDETECTOR_H_
#define SRC_OBJECTDETECTOR_H_

#include "MultiThreadBase.h"

class ObjectDetector:public MultiThreadBase {
public:
	ObjectDetector();
	virtual ~ObjectDetector();

	bool init(void);
	bool start(void);
	bool complete(void);
	void stop(void);

	void waitForComplete(void);

private:
	pthread_t	m_threadID;
	bool		m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((ObjectDetector *)This)->update();
		return NULL;
	}
};

#endif /* SRC_OBJECTDETECTOR_H_ */
