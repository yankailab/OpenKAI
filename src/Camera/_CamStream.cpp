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

	m_bThreadON = false;
	m_threadID = NULL;

	m_bGray = false;
	m_bHSV = false;
	m_cudaDeviceID = 0;


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

	CHECK_INFO(pJson->getVal("CAM_"+camName+"_CUDADEVICE_ID", &m_cudaDeviceID));
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
	int i;
	GpuMat* pNewInput;
	m_tSleep = TRD_INTERVAL_CAMSTREAM;
	cuda::setDevice(m_cudaDeviceID);

	while (m_bThreadON)
	{
		this->updateTime();

		pNewInput = m_pCamL->readFrame();

		if(this->mutexTrylock(CAMSTREAM_MUTEX_ORIGINAL))
		{
			m_pFrameL->updateFrameSwitch(pNewInput);
			this->mutexUnlock(CAMSTREAM_MUTEX_ORIGINAL);
		}
		else
		{
			continue;
		}

		if(m_bGray)
		{
			if(this->mutexTrylock(CAMSTREAM_MUTEX_GRAY))
			{
				m_pGrayL->switchFrame();
				m_pGrayL->getGrayOf(*m_pFrameProcess);
				this->mutexUnlock(CAMSTREAM_MUTEX_GRAY);
			}
		}

		if(m_bHSV)
		{
			if(this->mutexTrylock(CAMSTREAM_MUTEX_HSV))
			{
				m_pHSV->switchFrame();
				m_pHSV->getHSVOf(*m_pFrameProcess);
				this->mutexUnlock(CAMSTREAM_MUTEX_HSV);
			}
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
