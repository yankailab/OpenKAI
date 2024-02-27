/*
 * _FrameCutOut.cpp
 *
 *  Created on: Nov 16, 2017
 *      Author: yankai
 */

#include "_FrameCutOut.h"

namespace kai
{

	_FrameCutOut::_FrameCutOut()
	{
		m_progress = 0.0;
	}

	_FrameCutOut::~_FrameCutOut()
	{
	}

	bool _FrameCutOut::init(void *pKiss)
	{
		IF_F(!this->_FileBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	bool _FrameCutOut::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _FrameCutOut::update(void)
	{
		srand(time(NULL));
	}

}
