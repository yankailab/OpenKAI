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
	m_camName = "";
	m_pCamL = new CamInput();
	m_pCamR = new CamInput();
	m_pFrameL = new CamFrame();
	m_pFrameR = new CamFrame();
	m_pHSV = new CamFrame();
	m_pGrayL = new CamFrame();
	m_pGrayR = new CamFrame();
	m_pDepth = new CamFrame();
	m_pMonitor = new CamMonitor();
	m_pFrameProcess = &m_pFrameL;

	m_pMarkerDetect = new CamMarkerDetect();
	m_pDenseFlow = new CamDenseFlow();
	m_pSparseFlow = new CamSparseFlow();
	m_pStereo = new CamStereo();

	m_bStereoCam = false;
	m_bMarkerDetect = false;
	m_bDenseFlow = false;
	m_bSparseFlow = false;
	m_bHSV = false;
	m_bGray = false;
	m_bShowWindow = false;

	m_bThreadON = false;
	m_threadID = NULL;

}

CamStream::~CamStream()
{
	RELEASE(m_pCamL);
	RELEASE(m_pCamR);
	RELEASE(m_pFrameL);
	RELEASE(m_pFrameR);
	RELEASE(m_pHSV);
	RELEASE(m_pGrayL);
	RELEASE(m_pGrayR);
	RELEASE(m_pDepth);
	RELEASE(m_pMonitor);

	RELEASE(m_pMarkerDetect);
	RELEASE(m_pDenseFlow);
	RELEASE(m_pSparseFlow);
	RELEASE(m_pStereo);

}

bool CamStream::init(void)
{
	m_pFrameL->init();
	m_pFrameR->init();
	m_pHSV->init();
	m_pMonitor->init();
	m_pGrayL->init();
	m_pGrayR->init();
	m_pDepth->init();
	m_pFrameProcess = &m_pFrameL;

	m_pMarkerDetect->init();
	m_pDenseFlow->init();
	m_pSparseFlow->init();
	m_pStereo->init();

	m_bThreadON = false;

	return true;
}

bool CamStream::openWindow(void)
{
	if(m_bShowWindow)
	{
		return true;
	}

	if(m_camName=="")
	{
		return false;
	}
	namedWindow(m_camName);
	m_pMonitor->setWindowName(m_camName);
	m_bShowWindow = true;

	return true;

}

void CamStream::closeWindow(void)
{
	m_bShowWindow = false;
}

bool CamStream::start(void)
{
	//Open camera
	CHECK_ERROR(m_pCamL->openCamera());
	m_pCamL->setSize();

	if(m_pCamR->m_camDeviceID != m_pCamL->m_camDeviceID)
	{
		CHECK_ERROR(m_pCamR->openCamera());
		m_pCamR->setSize();

		m_bStereoCam = true;

//		m_pDepth->m_uFrame = Mat(CV_8U));
	}


	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode << " in CamStream::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO) << "CamStream.start()";

	return true;
}

void CamStream::update(void)
{
	int tThreadBegin;
	m_tSleep = TRD_INTERVAL_CAMSTREAM;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		m_pFrameL->switchFrame();
		m_pCamL->readFrame(m_pFrameL);

		if(m_bStereoCam)
		{
			m_pFrameR->switchFrame();
			m_pCamR->readFrame(m_pFrameR);

			m_pGrayL->switchFrame();
			m_pFrameL->getGray(m_pGrayL);
			m_pGrayR->switchFrame();
			m_pFrameR->getGray(m_pGrayR);

			m_pStereo->detect(m_pGrayL,m_pGrayR,m_pDepth);
		}
		else if(m_bGray)
		{
			m_pGrayL->switchFrame();
			(*m_pFrameProcess)->getGray(m_pGrayL);
		}

		if(m_bHSV)
		{
			m_pHSV->switchFrame();
			(*m_pFrameProcess)->getHSV(m_pHSV);
		}


		if(m_bMarkerDetect)
		{
			m_pMarkerDetect->detect(m_pHSV,m_pFrameL,true);
		}

		if(m_bDenseFlow)
		{
			m_pDenseFlow->detect(m_pGrayL);
		}

		if(m_bSparseFlow)
		{
			m_pSparseFlow->detect(m_pGrayL);
		}

		if(m_bShowWindow)
		{
//			m_pMonitor->addFrame(m_pFrameL,0,0);
//			m_pMonitor->addFrame(m_pFrameR,0,0);
			m_pMonitor->addFrame(m_pDepth,0,0);

//			m_pMonitor->show();
		}

		if(m_tSleep>0)
		{
			//sleepThread can be woke up by this->wakeupThread()
			this->sleepThread(0,m_tSleep);
		}
	}

}

bool CamStream::complete(void)
{
	m_pCamL->m_camera.release();
	if(m_bStereoCam)
	{
		m_pCamR->m_camera.release();
	}

	return true;
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
