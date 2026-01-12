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

	int _UIbase::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		return true;
	}

	int _UIbase::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));

		vector<string> vB;
		pK->a("vBASE", &vB);
		for (string p : vB)
		{
			BASE *pB = (BASE *)(pK->findModule(p));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return true;
	}

	int _UIbase::start(void)
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
