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

	bool _D2RGB::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		return true;
	}

	bool _D2RGB::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jVar(j, "_RGBDbase", n);
		m_pV = (_RGBDbase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	Frame *_D2RGB::getFrameRGB(void)
	{
		NULL_N(m_pV);
		return m_pV->getFrameD();
	}
}
