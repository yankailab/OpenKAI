/*
 * _Morphology.cpp
 *
 *  Created on: March 11, 2019
 *      Author: yankai
 */

#include "_Morphology.h"

#ifdef USE_OPENCV

namespace kai
{

_Morphology::_Morphology()
{
	m_type = vision_morphology;
	m_pV = NULL;
}

_Morphology::~_Morphology()
{
	close();
}

bool _Morphology::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	int i = 0;
	while (1)
	{
		Kiss* pM = pK->child(i++);
		if(pM->empty())break;

		IMG_MORPH m;
		m.init();
		pM->v("morphOp", &m.m_morphOp);
		pM->v("nItr", &m.m_nItr);
		pM->v("kShape", &m.m_kShape);
		pM->v("kW", &m.m_kW);
		pM->v("kH", &m.m_kH);
		pM->v("aX", &m.m_aX);
		pM->v("aY", &m.m_aY);
		m.updateKernel();

		m_vFilter.push_back(m);
	}

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->getInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Morphology::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Morphology::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Morphology::start(void)
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

void _Morphology::update(void)
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

void _Morphology::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	m_fIn.copy(*m_pV->BGR());

	Mat m1 = *m_fIn.m();
	Mat m2;
	Mat* pM1 = &m1;
	Mat* pM2 = &m2;
	Mat* pT;

	for(int i=0;i<m_vFilter.size();i++)
	{
		IMG_MORPH* pM = &m_vFilter[i];

		cv::morphologyEx(*pM1, *pM2,
				pM->m_morphOp,
				pM->m_kernel,
				cv::Point(pM->m_aX, pM->m_aY),
				pM->m_nItr);

		SWAP(pM1,pM2,pT);
	}

	m_fBGR.copy(*pM1);
}

}
#endif
