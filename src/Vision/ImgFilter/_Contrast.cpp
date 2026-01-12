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

	int _Contrast::init(const json& j)
	{
		CHECK_(_VisionBase::init(j));

		= j.value("alpha", &m_alpha);
		= j.value("beta", &m_beta);

		return true;
	}

	int _Contrast::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_VisionBase::link(j, pM));

		string n;
		n = "";
		= j.value("_VisionBase", &n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL__(m_pV);

		return true;
	}

	int _Contrast::start(void)
	{
		NULL_F(m_pT);
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
		NULL_(pF);
		IF_(pF->bEmpty());
		//		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat m;
		pF->m()->convertTo(m, -1, m_alpha, m_beta);
		m_fRGB.copy(m);
	}

}
