/*
 * _InRange.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_InRange.h"

namespace kai
{

_InRange::_InRange()
{
	m_type = vision_inRange;
	m_pV = NULL;
	m_rFrom = 0.0;
	m_rTo = 15.0;
}

_InRange::~_InRange()
{
	close();
}

bool _InRange::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,rFrom);
	KISSm(pK,rTo);

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _InRange::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _InRange::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _InRange::start(void)
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

void _InRange::update(void)
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

void _InRange::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	Mat m;
	cv::inRange(*m_pV->BGR()->m(), m_rFrom, m_rTo, m);
	m_fBGR.copy(m);
}

}
