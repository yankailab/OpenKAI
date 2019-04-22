/*
 * _Resize.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Resize.h"

namespace kai
{

_Resize::_Resize()
{
	m_type = vision_resize;
	m_pV = NULL;
}

_Resize::~_Resize()
{
	close();
}

bool _Resize::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Resize::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Resize::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Resize::start(void)
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

void _Resize::update(void)
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

void _Resize::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	m_fBGR.copy(m_pV->BGR()->resize(m_w, m_h));
}

}
