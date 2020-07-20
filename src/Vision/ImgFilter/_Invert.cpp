/*
 * _Invert.cpp
 *
 *  Created on: March 14, 2019
 *      Author: yankai
 */

#include "_Invert.h"

#ifdef USE_OPENCV

namespace kai
{

_Invert::_Invert()
{
	m_type = vision_invert;
	m_pV = NULL;
}

_Invert::~_Invert()
{
	close();
}

bool _Invert::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->getInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Invert::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Invert::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Invert::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Invert::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
			open();

		this->autoFPSfrom();

		if(m_bOpen)
		{
			if(m_fIn.tStamp() < m_pV->BGR()->tStamp())
				filter();
		}

		this->autoFPSto();
	}
}

void _Invert::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	Mat m;
	cv::bitwise_not(*m_pV->BGR()->m(),m);
	m_fBGR.copy(m);
}

}
#endif
