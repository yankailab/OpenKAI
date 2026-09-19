/*
 * _NavBase.cpp
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#include "_NavBase.h"

namespace kai
{

	_NavBase::_NavBase()
	{
	}

	_NavBase::~_NavBase()
	{
	}

	bool _NavBase::init(const json &j)
	{
		IF_F(!this->_ReferenceFrame::init(j));

//		jKv(j, "scale", m_scale);

		return true;
	}

	float _NavBase::confidence(void)
	{
		return m_confidence;
	}

	void _NavBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ReferenceFrame::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("confidence=" + f2str(m_confidence));
	}

}
