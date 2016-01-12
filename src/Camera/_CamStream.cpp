/*
 * CamStream.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "_CamStream.h"

namespace kai
{

_CamStream::_CamStream()
{
	_ThreadBase();

	m_camName = "";
	m_pCamL = new CamInput();
	m_pCamR = new CamInput();
	m_pFrameL = new CamFrame();
	m_pFrameR = new CamFrame();
	m_pHSV = new CamFrame();
	m_pGrayL = new CamFrame();
	m_pGrayR = new CamFrame();
	m_pDepth = new CamFrame();
//	m_pBGRAL = new CamFrame();
	m_pFrameProcess = &m_pFrameL;

	m_pSparseFlow = new CamSparseFlow();
	m_pStereo = new CamStereo();

	m_bStereoCam = false;
	m_bSparseFlow = false;
	m_bHSV = false;
	m_bGray = false;

	m_bThreadON = false;
	m_threadID = NULL;

	m_pDenseFlow = NULL;
	m_pOD = NULL;
	m_pFD = NULL;
	m_pSegNet = NULL;

}

_CamStream::~_CamStream()
{
	RELEASE(m_pCamL);
	RELEASE(m_pCamR);
	RELEASE(m_pFrameL);
	RELEASE(m_pFrameR);
	RELEASE(m_pHSV);
	RELEASE(m_pGrayL);
	RELEASE(m_pGrayR);
	RELEASE(m_pDepth);
//	RELEASE(m_pBGRAL);

	RELEASE(m_pSparseFlow);
	RELEASE(m_pStereo);

}

bool _CamStream::init(JSON* pJson, string camName)
{
	if(!pJson)return false;

	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_NAME", &m_camName));
	CHECK_ERROR(m_pCamL->setup(pJson, camName));

	m_pFrameProcess = &m_pFrameL;

	m_pSparseFlow->init();
	m_pStereo->init();

	m_bThreadON = false;

	return true;
}

bool _CamStream::start(void)
{
	//Open camera
	CHECK_ERROR(m_pCamL->openCamera());

//	if(m_pCamR->m_camDeviceID != m_pCamL->m_camDeviceID)
//	{
//		CHECK_ERROR(m_pCamR->openCamera());
//		m_bStereoCam = true;
//
////		m_pDepth->m_uFrame = Mat(CV_8U));
//	}

	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _CamStream::update(void)
{
	m_tSleep = TRD_INTERVAL_CAMSTREAM;

	while (m_bThreadON)
	{
		this->updateTime();

		//If the copyTo is called by other thread After switch changes and Before the readFrame complete
		//the previous frame might be used thus causes fluctuation.
		//We use frameID = the time the frame or Mat is created to avoid such case.
//		m_pFrameL->switchFrame();
		m_pFrameL->updateFrameSwitch(m_pCamL->readFrame());

		//Image processing
		if(m_bStereoCam)
		{
//			m_pFrameR->switchFrame();
//			m_pCamR->readFrame(m_pFrameR);
			m_pFrameR->updateFrameSwitch(m_pCamR->readFrame());

			//TODO
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

//		m_pBGRAL->switchFrame();
//		(*m_pFrameProcess)->getBGRA(m_pBGRAL);

		//Notification to other threads
		if(m_pDenseFlow)
		{
			m_pDenseFlow->updateFrame(m_pGrayL);
		}

		if(m_pOD)
		{
			m_pOD->updateFrame(m_pFrameL, m_pGrayL);
		}

		if(m_pFD)
		{
			m_pFD->updateFrame(m_pFrameL, m_pGrayL);
		}

		if(m_pSegNet)
		{
			m_pSegNet->updateFrame(m_pFrameL);
		}

		if(m_bSparseFlow)
		{
			m_pSparseFlow->detect(m_pGrayL);
		}

		if(m_tSleep>0)
		{
			//sleepThread can be woke up by this->wakeupThread()
			this->sleepThread(0,m_tSleep);
		}
	}

}

bool _CamStream::complete(void)
{
	m_pCamL->m_camera.release();
	if(m_bStereoCam)
	{
		m_pCamR->m_camera.release();
	}

	return true;
}

} /* namespace kai */
