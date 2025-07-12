/*
 * _Contrast.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_Contrast.h"

namespace kai
{

	_Contrast::_Contrast()
	{
		m_type = vision_contrast;
		m_pV = nullptr;

		m_alpha = 1.0;
		m_beta = 0.0;
	}

	_Contrast::~_Contrast()
	{
	}

	int _Contrast::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("alpha", &m_alpha);
		pK->v("beta", &m_beta);

		return OK_OK;
	}

	int _Contrast::link(void)
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

	int _Contrast::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Contrast::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Contrast::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		IF_(pF->bEmpty());
		//		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat m;
		pF->m()->convertTo(m, -1, m_alpha, m_beta);
		m_fRGB.copy(m);
	}

}
