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
	}

	_Contrast::~_Contrast()
	{
	}

	bool _Contrast::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		m_alpha = j.value("alpha", 1.0);
		m_beta = j.value("beta", 0.0);

		return true;
	}

	bool _Contrast::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = j.value("_VisionBase", "");
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Contrast::start(void)
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
