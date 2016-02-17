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
	m_pCamInput = new CamInput();
	m_pCamFrames = new FrameGroup();
	m_pGrayFrames = new FrameGroup();
	m_pHSVframes = new FrameGroup();

	m_bThreadON = false;
	m_threadID = NULL;

	m_bGray = false;
	m_bHSV = false;

}

_CamStream::~_CamStream()
{
	RELEASE(m_pCamInput);
	RELEASE(m_pCamFrames);
	RELEASE(m_pGrayFrames);
	RELEASE(m_pHSVframes);
}

bool _CamStream::init(JSON* pJson, string camName)
{
	if(!pJson)return false;

//	CHECK_INFO(pJson->getVal("CAM_"+camName+"_CUDADEVICE_ID", &m_cudaDeviceID));
	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_NAME", &m_camName));
	CHECK_ERROR(m_pCamInput->setup(pJson, camName));

	CHECK_FATAL(m_pCamFrames->init(2));
	CHECK_FATAL(m_pGrayFrames->init(2));
	CHECK_FATAL(m_pHSVframes->init(2));

	m_bThreadON = false;

	return true;
}

bool _CamStream::start(void)
{
	//Open camera
	CHECK_ERROR(m_pCamInput->openCamera());

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
	GpuMat* pNewInput;
	CamFrame* pFrame;
	CamFrame* pGray;
	CamFrame* pHSV;

	m_tSleep = TRD_INTERVAL_CAMSTREAM;

	while (m_bThreadON)
	{
		this->updateTime();

		//Get new input frame
		pNewInput = m_pCamInput->readFrame();

		//Update camera frame
		m_pCamFrames->updateFrameIndex();
		pFrame = m_pCamFrames->getLastFrame();
		pFrame->update(pNewInput);

		//Update Gray frame
		if(m_bGray)
		{
			m_pGrayFrames->updateFrameIndex();
			pGray = m_pGrayFrames->getLastFrame();
			pGray->getGrayOf(pFrame);
		}

		//Update HSV frame
		if(m_bHSV)
		{
			m_pHSVframes->updateFrameIndex();
			pHSV = m_pHSVframes->getLastFrame();
			pHSV->getHSVOf(pFrame);
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
	m_pCamInput->m_camera.release();

	return true;
}

CamFrame* _CamStream::getLastFrame(void)
{
	return m_pCamFrames->getLastFrame();
}

CamFrame* _CamStream::getLastGrayFrame(void)
{
	return m_pGrayFrames->getLastFrame();
}

CamFrame* _CamStream::getLastHSVFrame(void)
{
	return m_pHSVframes->getLastFrame();
}

FrameGroup* _CamStream::getFrameGroup(void)
{
	return m_pCamFrames;
}

FrameGroup* _CamStream::getGrayFrameGroup(void)
{
	return m_pGrayFrames;
}

FrameGroup* _CamStream::getHSVFrameGroup(void)
{
	return m_pHSVframes;
}


} /* namespace kai */
