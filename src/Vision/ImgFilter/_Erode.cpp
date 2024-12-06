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
		close();
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

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _Erode::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _Erode::close(void)
	{
		this->_VisionBase::close();
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
			if (!m_bOpen)
				open();

			m_pT->autoFPS();

			if (m_bOpen)
			{
				if (m_fIn.tStamp() < m_pV->getFrameRGB()->tStamp())
					filter();
			}

		}
	}

	void _Erode::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		m_fIn.copy(*m_pV->getFrameRGB());

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
