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
		close();
	}

	int _D2RGB::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _D2RGB::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_RGBDbase", &n);
		m_pV = (_RGBDbase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _D2RGB::open(void)
	{
		NULL_F(m_pV);
		return m_pV->isOpened();
	}

	void _D2RGB::close(void)
	{
		NULL_(m_pV);
		m_pV->close();
	}

	Frame *_D2RGB::getFrameRGB(void)
	{
		NULL_N(m_pV);
		return m_pV->getFrameD();
	}
}
