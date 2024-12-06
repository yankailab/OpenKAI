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

	int _UIbase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _UIbase::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		vector<string> vB;
		pK->a("vBASE", &vB);
		for (string p : vB)
		{
			BASE *pB = (BASE *)(pK->findModule(p));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return OK_OK;
	}

	int _UIbase::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
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
