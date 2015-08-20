/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "ObjectDetector.h"

ObjectDetector::ObjectDetector() {
	// TODO Auto-generated constructor stub

}

ObjectDetector::~ObjectDetector() {
	// TODO Auto-generated destructor stub
}


bool ObjectDetector::init(void)
{
/*	m_logger.init(LOG_LEVEL,"ObjectDetector",(long)this);

	if(m_pCS->init(this, m_pFCS, m_pV)==false)
	{
		m_logger.addLog(0,LOG_ERROR,"C_CentralServer.init()");
		return false;
	}

	m_logger.addLog(0,LOG_OK,"init()");
*/

//	m_tSleep = TRD_INTERVAL_OBJ;

	return true;
}

bool ObjectDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if(retCode!=0)
	{
//		m_logger.addLog(retCode,LOG_ERROR,"start().pthread_create()");
		m_bThreadON = false;
		return false;
	}

//	m_pVController->start();

//	m_logger.addLog(0,LOG_OK,"start()");
	return true;
}

void ObjectDetector::update(void)
{
	int tThreadBegin;

	while(m_bThreadON)
	{
		tThreadBegin = time(NULL);

//		m_logger.addLog(0,LOG_OK,"update()");

		this->sleepThread(m_tSleep);		//sleepThread can be woke up by this->wakeupThread()
	}

}

void ObjectDetector::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

//	m_pVController->stop();

//	m_logger.addLog(0,LOG_OK,"stop()");
}

void ObjectDetector::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

