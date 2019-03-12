/*
 * _Erode.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Erode.h"

namespace kai
{

_Erode::_Erode()
{
	m_type = vision_erode;
	m_pV = NULL;
}

_Erode::~_Erode()
{
	close();
}

bool _Erode::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss** ppM = pK->getChildItr();
	int i=0;
	while (ppM[i])
	{
		Kiss* pM = ppM[i++];

		IMG_ERODE e;
		e.init();
		pM->v("nItr", &e.m_nItr);
		pM->v("kShape", &e.m_kShape);
		pM->v("kW", &e.m_kW);
		pM->v("kH", &e.m_kH);
		pM->v("aX", &e.m_aX);
		pM->v("aY", &e.m_aY);
		e.updateKernel();

		m_vFilter.push_back(e);
	}

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Erode::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Erode::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Erode::start(void)
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

void _Erode::update(void)
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

void _Erode::filter(void)
{
	m_fIn.copy(*m_pV->BGR());

	Mat m1 = *m_fIn.m();
	Mat m2;
	Mat* pM1 = &m1;
	Mat* pM2 = &m2;
	Mat* pT;

	for(int i=0; i<m_vFilter.size(); i++)
	{
		IMG_ERODE* pI = &m_vFilter[i];

		cv::erode(*pM1, *pM2,
				pI->m_kernel,
				cv::Point(pI->m_aX, pI->m_aY),
				pI->m_nItr);

		SWAP(pM1,pM2,pT);
	}

	m_fBGR.copy(*pM1);
}

}
