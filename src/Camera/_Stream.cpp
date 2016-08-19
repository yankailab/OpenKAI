/*
 * CamStream.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "_Stream.h"

namespace kai
{

_Stream::_Stream()
{
	_ThreadBase();

	m_camName = "";
	m_pCamInput = new Camera();
	m_pCamFrame = new Frame();
	m_pGrayFrame = new Frame();
	m_pHSVframe = new Frame();

	m_bGray = false;
	m_bHSV = false;
}

_Stream::~_Stream()
{
	RELEASE(m_pCamInput);
	RELEASE(m_pCamFrame);
	RELEASE(m_pGrayFrame);
	RELEASE(m_pHSVframe);
}

bool _Stream::init(JSON* pJson, string camName)
{
	if(!pJson)return false;

	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("CAM_"+camName+"_FPS", &FPS));
	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_NAME", &m_camName));
	CHECK_ERROR(m_pCamInput->setup(pJson, camName));

	m_bThreadON = false;

	this->setTargetFPS(FPS);

	return true;
}

bool _Stream::start(void)
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

void _Stream::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		//Update camera frame
		m_pCamFrame->update(m_pCamInput->readFrame());

		//Update Gray frame
		if(m_bGray)
		{
			m_pGrayFrame->getGrayOf(m_pCamFrame);
		}

		//Update HSV frame
		if(m_bHSV)
		{
			m_pHSVframe->getHSVOf(m_pCamFrame);
		}

		this->autoFPSto();
	}

}

bool _Stream::complete(void)
{
	m_pCamInput->m_camera.release();

	return true;
}

Frame* _Stream::getFrame(void)
{
	return m_pCamFrame;
}

Frame* _Stream::getGrayFrame(void)
{
	return m_pGrayFrame;
}

Frame* _Stream::getHSVFrame(void)
{
	return m_pHSVframe;
}

Camera* _Stream::getCameraInput(void)
{
	return m_pCamInput;
}

} /* namespace kai */
