/*
 * Window.cpp
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#include "_UIbase.h"

namespace kai
{

	_UIbase::_UIbase()
	{
	}

	_UIbase::~_UIbase()
	{
	}

	bool _UIbase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());

		return true;
	}

	bool _UIbase::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		vector<string> vB;
		jKv(j, "vBASE", vB);
		m_vpB.clear();
		for (string n : vB)
		{
			BASE *pB = (BASE *)(m_pM->findModule(n));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return true;
	}

	bool _UIbase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _UIbase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}
}
