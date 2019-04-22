/*
 * _Depth2Gray.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Depth2Gray.h"

namespace kai
{

_Depth2Gray::_Depth2Gray()
{
	m_type = vision_depth2Gray;
	m_pV = NULL;
}

_Depth2Gray::~_Depth2Gray()
{
	close();
}

bool _Depth2Gray::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("_DepthVisionBase", &iName);
	m_pV = (_DepthVisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Depth2Gray::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Depth2Gray::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Depth2Gray::start(void)
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

void _Depth2Gray::update(void)
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

void _Depth2Gray::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	Mat mD = *m_pV->Depth()->m();
	Mat mGray;
	mD.convertTo(mGray,
				 255.0/(m_pV->m_vRange.y - m_pV->m_vRange.x),
				 0);
	m_fBGR.copy(mD);
}

}
