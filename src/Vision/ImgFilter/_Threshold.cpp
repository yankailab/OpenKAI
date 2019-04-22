/*
 * _Threshold.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Threshold.h"

namespace kai
{

_Threshold::_Threshold()
{
	m_type = vision_threshold;
	m_pV = NULL;
}

_Threshold::~_Threshold()
{
	close();
}

bool _Threshold::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss** ppM = pK->getChildItr();
	int i=0;
	while (ppM[i])
	{
		Kiss* pM = ppM[i++];

		IMG_THRESHOLD t;
		t.init();
		pM->v("type", &t.m_type);
		pM->v("vMax", &t.m_vMax);
		pM->v("bAutoThr", &t.m_bAutoThr);
		pM->v("thr", &t.m_thr);
		pM->v("method", &t.m_method);
		pM->v("thrType", &t.m_thrType);
		pM->v("blockSize", &t.m_blockSize);
		pM->v("C", &t.m_C);

		m_vFilter.push_back(t);
	}

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Threshold::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _Threshold::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _Threshold::start(void)
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

void _Threshold::update(void)
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

void _Threshold::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	m_fIn.copy(m_pV->BGR()->cvtColor(COLOR_RGB2GRAY));

	Mat m1 = *m_fIn.m();
	Mat m2;
	Mat* pM1 = &m1;
	Mat* pM2 = &m2;
	Mat* pT;

	for(int i=0; i<m_vFilter.size(); i++)
	{
		IMG_THRESHOLD* pI = &m_vFilter[i];

		if(pI->m_type == img_thr_adaptive)
		{
			cv::adaptiveThreshold(*pM1, *pM2,
					pI->m_vMax,
					pI->m_method,
					pI->m_thrType,
					pI->m_blockSize,
					pI->m_C);
		}
		else if(pI->m_type == img_thr)
		{
			if(pI->m_bAutoThr)
			{
				cv::threshold(*pM1, *pM2,
						0,
						255,
						pI->m_thrType|THRESH_OTSU);
			}
			else
			{
				cv::threshold(*pM1, *pM2,
						pI->m_thr,
						pI->m_vMax,
						pI->m_thrType);
			}
		}

		SWAP(pM1,pM2,pT);
	}

	m_fBGR.copy(*pM1);
}

}
