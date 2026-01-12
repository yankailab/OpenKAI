/*
 * _ColorConvert.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_ColorConvert.h"

namespace kai
{

	_ColorConvert::_ColorConvert()
	{
		m_type = vision_colorConvert;
		m_pV = nullptr;
		m_code = COLOR_RGB2GRAY;
	}

	_ColorConvert::~_ColorConvert()
	{
	}

	int _ColorConvert::init(const json& j)
	{
		CHECK_(_VisionBase::init(j));

		= j.value("code", &m_code);

		return true;
	}

	int _ColorConvert::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_VisionBase::link(j, pM));

		string n;
		n = "";
		= j.value("_VisionBase", &n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL__(m_pV);

		return true;
	}

	int _ColorConvert::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _ColorConvert::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _ColorConvert::filter(void)
	{
		NULL_(m_pV);
		Frame* pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
//		IF_(m_fRGB.tStamp() >= pF->tStamp());

		m_fRGB.copy(pF->cvtColor(m_code));
	}

}
