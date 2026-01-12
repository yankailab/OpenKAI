/*
 * _Invert.cpp
 *
 *  Created on: March 14, 2019
 *      Author: yankai
 */

#include "_Invert.h"

namespace kai
{

	_Invert::_Invert()
	{
		m_type = vision_invert;
		m_pV = nullptr;
	}

	_Invert::~_Invert()
	{
	}

	int _Invert::init(const json& j)
	{
		CHECK_(_VisionBase::init(j));

		return true;
	}

	int _Invert::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_VisionBase::link(j, pM));

		string n;
		n = "";
		= j.value("_VisionBase", &n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL__(m_pV);

		return true;
	}

	int _Invert::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Invert::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Invert::filter(void)
	{
		NULL_(m_pV);
		Frame* pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat m;
		cv::bitwise_not(*pF->m(), m);
		m_fRGB.copy(m);
	}

}
