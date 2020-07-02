/*
 * _Mask.cpp
 *
 *  Created on: July 2, 2020
 *      Author: yankai
 */

#include "_Mask.h"

#ifdef USE_OPENCV

namespace kai
{

_Mask::_Mask()
{
	m_pV = NULL;
	m_pVmask = NULL;
}

_Mask::~_Mask()
{
	close();
}

bool _Mask::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	iName = "";
	pK->v("_VisionBaseMask", &iName);
	m_pVmask = (_VisionBase*) (pK->parent()->getInst(iName));
	IF_Fl(!m_pVmask, iName + ": not found");

	return true;
}

bool _Mask::open(void)
{
	NULL_F(m_pV);
	NULL_F(m_pVmask);

	m_bOpen = m_pV->isOpened();
	IF_F(!m_bOpen);

	m_bOpen = m_pVmask->isOpened();

	return m_bOpen;
}

void _Mask::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Mask::start(void)
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

void _Mask::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
			open();

		this->autoFPSfrom();

		if(m_bOpen)
		{
			if(m_fIn.tStamp() < m_pV->BGR()->tStamp())
			{
				filter();
			}
		}

		this->autoFPSto();
	}
}

void _Mask::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());
	IF_(m_pVmask->BGR()->bEmpty());

	m_fIn.copy(*m_pV->BGR());
	m_fMask.copy(*m_pVmask->BGR());

	Mat mV = *m_fIn.m();
	Mat mM = *m_fMask.m();
	Mat mBg;
	mBg.zeros(mV.rows, mV.cols, mV.type());

	mV.copyTo(mBg, mM);
	m_fBGR.copy(mBg);
}

}
#endif
