/*
 * _Contrast.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Contrast.h"

#ifdef USE_OPENCV

namespace kai
{

_Contrast::_Contrast()
{
	m_type = vision_contrast;
	m_pV = NULL;

	m_alpha = 1.0;
	m_beta = 0.0;
}

_Contrast::~_Contrast()
{
	close();
}

bool _Contrast::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v<double>("alpha",&m_alpha);
	pK->v<double>("beta",&m_beta);

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->getInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Contrast::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Contrast::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Contrast::start(void)
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

void _Contrast::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
			open();

		this->autoFPSfrom();

		if(m_bOpen)
		{
			filter();
		}

		this->autoFPSto();
	}
}

void _Contrast::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	Mat m;
	m_pV->BGR()->m()->convertTo(m, -1, m_alpha, m_beta);
	m_fBGR.copy(m);
}

}
#endif
