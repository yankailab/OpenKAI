/*
 * Window.cpp
 *
 *  Created on: July 10, 2023
 *      Author: Kai Yan
 */

#include "_WebUIswarm.h"

namespace kai
{

	_WebUIswarm::_WebUIswarm()
	{
	}

	_WebUIswarm::~_WebUIswarm()
	{
	}

	bool _WebUIswarm::init(void *pKiss)
	{
		IF_F(!this->_WebUIbase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

//		pK->v("rootDir", &m_rootDir);

		return true;
	}

	bool _WebUIswarm::link(void)
	{
		IF_F(!this->_WebUIbase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		return true;
	}

	bool _WebUIswarm::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _WebUIswarm::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			m_pT->autoFPSto();
		}
	}
}
