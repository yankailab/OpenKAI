/*
 * _Resize.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Resize.h"

namespace kai
{

	_Resize::_Resize()
	{
		m_type = vision_resize;
		m_pV = nullptr;
	}

	_Resize::~_Resize()
	{
	}

	int _Resize::init(const json& j)
	{
		CHECK_(_VisionBase::init(j));

		return true;
	}

	int _Resize::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_VisionBase::link(j, pM));

		string n;
		n = "";
		= j.value("_VisionBase", &n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL__(m_pV);

		return true;
	}

	int _Resize::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Resize::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _Resize::filter(void)
	{
		NULL_(m_pV);
		Frame* pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
		IF_(m_fRGB.tStamp() >= pF->tStamp());

		m_fRGB.copy(pF->resize(m_vSizeRGB.x, m_vSizeRGB.y));
	}

}
