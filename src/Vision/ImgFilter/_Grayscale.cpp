/*
 * _Grayscale.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Grayscale.h"

#ifdef USE_OPENCV

namespace kai
{

_Grayscale::_Grayscale()
{
	m_type = vision_grayscale;
	m_pV = NULL;
}

_Grayscale::~_Grayscale()
{
	close();
}

bool _Grayscale::init(void* pKiss)
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

bool _Grayscale::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Grayscale::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Grayscale::start(void)
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

void _Grayscale::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
			open();

		this->autoFPSfrom();

		if(m_bOpen)
		{
			if(m_fBGR.tStamp() < m_pV->BGR()->tStamp())
			{
				filter();
			}
		}

		this->autoFPSto();
	}
}

void _Grayscale::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	m_fBGR.copy(m_pV->BGR()->cvtColor(COLOR_RGB2GRAY));
}

}
#endif
