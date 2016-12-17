/*
 * _Stream.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "_Stream.h"

/*
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
		m_pCamera = new _Camera();
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

	F_ERROR_F(m_pCamera->init(pC));

	m_bThreadON = false;
	return true;
}

bool _Stream::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
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
	if(m_pCamera->open()==false)
	{
		LOG(FATAL)<<"Cannot open stream input";
		return;
	}

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		//Update camera frame
		m_pFrame->update(m_pCamera->getFrame());

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

bool _Stream::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Frame* pFrame = pWin->getFrame();

	if(m_showDepth==0)
	{
		pFrame->update(m_pFrame);
	}
	else
	{
		pFrame->update(m_pCamera->getDepthFrame());
	}

	this->_ThreadBase::draw();

	return true;
}

}
*/

