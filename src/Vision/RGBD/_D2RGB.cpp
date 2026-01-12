/*
 * _D2RGB.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_D2RGB.h"

namespace kai
{

	_D2RGB::_D2RGB()
	{
		m_type = vision_D2RGB;
		m_pV = nullptr;
	}

	_D2RGB::~_D2RGB()
	{
	}

	int _D2RGB::init(const json& j)
	{
		CHECK_(_VisionBase::init(j));

		return true;
	}

	int _D2RGB::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_VisionBase::link(j, pM));

		string n;
		n = "";
		= j.value("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pM->findModule(n));
		NULL__(m_pV);

		return true;
	}

	Frame *_D2RGB::getFrameRGB(void)
	{
		NULL_N(m_pV);
		return m_pV->getFrameD();
	}
}
