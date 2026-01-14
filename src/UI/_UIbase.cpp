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

	bool _UIbase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		return true;
	}

	bool _UIbase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		vector<string> vB = j.value("vBASE", vector<string>{});
		m_vpB.clear();
		for (string n : vB)
		{
			BASE *pB = (BASE *)(pM->findModule(n));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return true;
	}

	bool _UIbase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _UIbase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();
		}
	}
}
