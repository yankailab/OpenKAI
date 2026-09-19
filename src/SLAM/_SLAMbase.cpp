/*
 * _SLAMbase.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_SLAMbase.h"

namespace kai
{

	_SLAMbase::_SLAMbase()
	{
	}

	_SLAMbase::~_SLAMbase()
	{
	}

	bool _SLAMbase::init(const json &j)
	{
		IF_F(!this->_NavBase::init(j));

		jKv(j, "tScaleIMU", m_tScaleIMU);
		jKv(j, "tScalePC", m_tScalePC);

		return true;
	}

	bool _SLAMbase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_NavBase::link(j, pM));

		string n;

		n = "";
		jKv(j, "_IMUbase", n);
		m_pIMU = (_IMUbase *)(pM->findModule(n));

		return true;
	}

	bool _SLAMbase::check(void)
	{
		return this->_NavBase::check();
	}

	bool _SLAMbase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _SLAMbase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}

	bool _SLAMbase::bTracking(void)
	{
		return m_bTracking;
	}

	void _SLAMbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_NavBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		if (m_bTracking)
		{
			msg = "Tracking";
		}
		else
		{
			msg = "Tracking lost";
		}
		pC->addMsg(msg);

	}

}
