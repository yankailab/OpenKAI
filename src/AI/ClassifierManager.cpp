/*
 * ClassifierManager.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#include "ClassifierManager.h"

namespace kai
{

ClassifierManager::ClassifierManager()
{
	m_numObj = 0;


}

ClassifierManager::~ClassifierManager()
{
	// TODO Auto-generated destructor stub
}

bool ClassifierManager::init(JSON* pJson)
{
//	CHECK_FATAL(pJson->getVal("CAFFE_MODEL_FILE", &modelFile));

	return true;
}

bool ClassifierManager::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void ClassifierManager::update(void)
{
	m_tSleep = TRD_INTERVAL_CLASSIFIER_MANAGER;

	while (m_bThreadON)
	{
		this->updateTime();


		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}



void ClassifierManager::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);
}

void ClassifierManager::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

bool ClassifierManager::complete(void)
{
	return true;
}


} /* namespace kai */
