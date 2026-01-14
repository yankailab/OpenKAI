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
	}

	_ColorConvert::~_ColorConvert()
	{
	}

	bool _ColorConvert::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		m_code = j.value("code", COLOR_RGB2GRAY);

		return true;
	}

	bool _ColorConvert::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = j.value("_VisionBase", "");
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _ColorConvert::start(void)
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
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
		//		IF_(m_fRGB.tStamp() >= pF->tStamp());

		m_fRGB.copy(pF->cvtColor(m_code));
	}

}
