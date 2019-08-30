/*
 * _Rotate.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Rotate.h"

namespace kai
{

_Rotate::_Rotate()
{
	m_type = vision_rotate;
	m_pV = NULL;
	m_code = 0;
}

_Rotate::~_Rotate()
{
	close();
}

bool _Rotate::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("code",&m_code);

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Rotate::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Rotate::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Rotate::start(void)
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

void _Rotate::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
			open();

		this->autoFPSfrom();

		if(m_bOpen)
		{
			if(m_fBGR.tStamp() < m_pV->BGR()->tStamp())
				filter();
		}

		this->autoFPSto();
	}
}

void _Rotate::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	Mat m;
	cv::rotate(*m_pV->BGR()->m(), m, m_code);
	m_fBGR.copy(m);

}

}
