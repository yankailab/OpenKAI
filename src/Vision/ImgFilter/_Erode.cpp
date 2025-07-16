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
		m_pV = nullptr;
	}

	_Erode::~_Erode()
	{
	}

	int _Erode::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		int i = 0;
		while (1)
		{
			Kiss *pM = pK->child(i++);
			if (pM->empty())
				break;

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

		return OK_OK;
	}

	int _Erode::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _Erode::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Erode::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Erode::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
		IF_(m_fIn.tStamp() >= pF->tStamp());

		m_fIn.copy(*pF);

		Mat m1 = *m_fIn.m();
		Mat m2;
		Mat *pM1 = &m1;
		Mat *pM2 = &m2;
		Mat *pT;

		for (int i = 0; i < m_vFilter.size(); i++)
		{
			IMG_ERODE *pI = &m_vFilter[i];

			cv::erode(*pM1, *pM2,
					  pI->m_kernel,
					  cv::Point(pI->m_aX, pI->m_aY),
					  pI->m_nItr);

			SWAP(pM1, pM2, pT);
		}

		m_fRGB.copy(*pM1);
	}

}
