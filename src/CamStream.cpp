/*
 * CamStream.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "CamStream.h"

namespace kai
{

CamStream::CamStream()
{
	// TODO Auto-generated constructor stub

}

CamStream::~CamStream()
{
	// TODO Auto-generated destructor stub
}

bool CamStream::init(void)
{



	m_pMonitor->setWindowName("Front camera");
	m_pMonitor->init();

	m_pFrameProcess = &m_pFrameL;


	return true;
}

bool CamStream::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode << " in ObjectDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO) << "ObjectDetector.start()";

	return true;
}

void CamStream::update(void)
{
	int tThreadBegin;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		m_pFrameL->switchFrame();
		m_pCamL->readFrame(m_pFrameL);

		m_pFrameR->switchFrame();
		m_pCamR->readFrame(m_pFrameR);

		m_pHSV->switchFrame();
		(*m_pFrameProcess)->getHSV(m_pHSV);

		m_pMarkerDetect->detect(m_pHSV);

//		g_pAP->markerLock(&g_markerDet,g_pVehicle);
//		g_numObj = g_objDet.detect(g_frontRGB.m_uFrame/*.getMat(ACCESS_READ)*/,&g_pObj);

		m_pMonitor->addFrame(m_pFrameL,0,0);
		m_pMonitor->show();

//		this->sleepThread(m_tSleep,0);//sleepThread can be woke up by this->wakeupThread()
	}

}

bool CamStream::complete(void)
{

}

void CamStream::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);


	LOG(INFO) << "CamStream.stop()";
}

void CamStream::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

} /* namespace kai */
