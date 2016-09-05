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
	m_pCamera = NULL;//new Camera();
	m_pCamFrame = new Frame();
	m_pGrayFrame = new Frame();
	m_pHSVframe = new Frame();
	m_showDepth = 0;

	m_bGray = false;
	m_bHSV = false;
}

_Stream::~_Stream()
{
	RELEASE(m_pCamera);
	RELEASE(m_pCamFrame);
	RELEASE(m_pGrayFrame);
	RELEASE(m_pHSVframe);
}

bool _Stream::init(JSON* pJson, string camName)
{
	if(!pJson)return false;

	double FPS = DEFAULT_FPS;
	int camType;
	m_camName = camName;

	CHECK_INFO(pJson->getVal("CAM_"+camName+"_FPS", &FPS));
	CHECK_INFO(pJson->getVal("CAM_"+camName+"_SHOWDEPTH", &m_showDepth));
	CHECK_FATAL(pJson->getVal("CAM_"+camName+"_TYPE", &camType));

	switch (camType)
	{
	case CAM_GENERAL:
		m_pCamera = new Camera();
		break;
	case CAM_ZED:
#ifdef USE_ZED
		m_pCamera = new ZED();
#else
		LOG(FATAL)<<"ZED is not support in this build, please compile OpenKAI with ZED support enabled.";
		return false;
#endif
		break;
	default:
		return false;
		break;
	}

	CHECK_ERROR(m_pCamera->setup(pJson, camName));

	m_bThreadON = false;
	this->setTargetFPS(FPS);

	return true;
}

bool _Stream::start(void)
{
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
	//Open camera
	if(m_pCamera->openCamera()==false)return;

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		//Update camera frame
		m_pCamFrame->update(m_pCamera->readFrame());

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
	m_pCamera->release();

	return true;
}

Frame* _Stream::getBGRFrame(void)
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

CamBase* _Stream::getCameraInput(void)
{
	return m_pCamera;
}

bool _Stream::draw(Frame* pFrame, iVector4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	if(m_pCamFrame->empty())return false;

	if(m_showDepth==0)
	{
		pFrame->update(m_pCamFrame);
	}
	else
	{
		pFrame->update(m_pCamera->getDepthFrame());
	}

	putText(*pFrame->getCMat(), "Camera FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

} /* namespace kai */
