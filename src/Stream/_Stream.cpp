/*
 * _Stream.cpp
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

	m_pCamera = NULL;
	m_pFrame = new Frame();
	m_pGrayFrame = new Frame();
	m_pHSVframe = new Frame();
	m_showDepth = 0;

	m_bGray = false;
	m_bHSV = false;
}

_Stream::~_Stream()
{
	DEL(m_pCamera);
	DEL(m_pFrame);
	DEL(m_pGrayFrame);
	DEL(m_pHSVframe);
}

bool _Stream::init(void* pKiss)
{
	CHECK_F(this->_ThreadBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;
	F_INFO(pK->v("bShowDepth", &m_showDepth));
	F_INFO(pK->v("bGray", &m_bGray));
	F_INFO(pK->v("bHSV", &m_bHSV));

	string camClass;
	string camName;

	F_FATAL_F(pK->v("input", &camName));
	Kiss* pC = pK->o(camName);
	F_FATAL_F(pC->v("class", &camClass));

	if(camClass=="Camera")
	{
		m_pCamera = new Camera();
	}
	else if(camClass=="ZED")
	{
#ifdef USE_ZED
		m_pCamera = new ZED();
#else
		LOG(FATAL)<<"ZED is not support in this build, please compile OpenKAI with ZED support enabled";
		return false;
#endif
	}
	else
	{
		LOG(FATAL)<<"Unknown stream input";
		return false;
	}

	F_ERROR_F(m_pCamera->setup(pC));

	double FPS;
	if(pC->v("FPS", &FPS))
	{
		setTargetFPS(FPS);
	}

	m_bThreadON = false;
	return true;
}

bool _Stream::link(void)
{
	NULL_F(m_pKiss);
	//TODO:link variables

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
	//Open stream input
	if(m_pCamera->openCamera()==false)
	{
		LOG(FATAL)<<"Cannot open stream input:";
		return;
	}

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		//Update camera frame
		m_pFrame->update(m_pCamera->readFrame());

		//Update Gray frame
		if(m_bGray)
		{
			m_pGrayFrame->getGrayOf(m_pFrame);
		}

		//Update HSV frame
		if(m_bHSV)
		{
			m_pHSVframe->getHSVOf(m_pFrame);
		}

		this->autoFPSto();
	}

	m_pCamera->release();

}

Frame* _Stream::getBGRFrame(void)
{
	return m_pFrame;
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

bool _Stream::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);

	if(m_pFrame->empty())return false;

	if(m_showDepth==0)
	{
		pFrame->update(m_pFrame);
	}
	else
	{
		pFrame->update(m_pCamera->getDepthFrame());
	}

	putText(*pFrame->getCMat(), "Stream "+(*this->getName())+" FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

} /* namespace kai */
