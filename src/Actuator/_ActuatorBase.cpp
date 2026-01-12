/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

	_ActuatorBase::_ActuatorBase()
	{
		m_p.init();
		m_s.init();
		m_a.init();
		m_b.init();
		m_c.init();

		m_tLastCmd = 0;
		m_ieCheckAlarm.init(100000);
		m_ieReadStatus.init(50000);
		m_ieSendCMD.init(50000);

		m_bfStatus.clearAll();
		m_bfSet.clearAll();

		m_pParent = nullptr;
	}

	_ActuatorBase::~_ActuatorBase()
	{
	}

	bool _ActuatorBase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		m_ID = j.value("ID", 0);

		m_origin = j.value("pOrigin", 0);
		m_p.m_v = j.value("p", 0);
		m_p.m_vTarget = j.value("pTarget", 0);
		m_p.m_vErr = j.value("pErr", 0);
		m_p.m_vRange = j.value("pRange", vector<float>{-FLT_MAX, FLT_MAX});

		m_s.m_v = j.value("s", 0);
		m_s.m_vTarget = j.value("sTarget", 0);
		m_s.m_vErr = j.value("sErr", 0);
		m_s.m_vRange = j.value("sRange", vector<float>{-FLT_MAX, FLT_MAX});

		m_a.m_v = j.value("a", 0);
		m_a.m_vTarget = j.value("aTarget", 0);
		m_a.m_vErr = j.value("aErr", 0);
		m_a.m_vRange = j.value("aRange", vector<float>{-FLT_MAX, FLT_MAX});

		m_b.m_v = j.value("b", 0);
		m_b.m_vTarget = j.value("bTarget", 0);
		m_b.m_vErr = j.value("bErr", 0);
		m_b.m_vRange = j.value("bRange", vector<float>{-FLT_MAX, FLT_MAX});

		m_c.m_v = j.value("c", 0);
		m_c.m_vTarget = j.value("cTarget", 0);
		m_c.m_vErr = j.value("cErr", 0);
		m_c.m_vRange = j.value("cRange", vector<float>{-FLT_MAX, FLT_MAX});

		m_tCmdTimeout = j.value("tCmdTimeout", 0);
		m_ieCheckAlarm.m_tInterval = j.value("tIntCheckAlarm", 100000);
		m_ieReadStatus.m_tInterval = j.value("tIntReadStatus", 50000);
		m_ieSendCMD.m_tInterval = j.value("tIntSendCMD", 50000);

		int bf;

		bf = j.value("bfSetPower", 0);
		if (bf)
			m_bfSet.set(actuator_power);

		bf = j.value("bfSetID", 0);
		if (bf)
			m_bfSet.set(actuator_setID);

		bf = j.value("bfSetMode", 0);
		if (bf)
			m_bfSet.set(actuator_setMode);

		bf = j.value("bfMove", 0);
		if (bf)
			m_bfSet.set(actuator_move);

		return true;
	}

	bool _ActuatorBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = j.value("_ActuatorBase", "");
		m_pParent = (_ActuatorBase *)(pM->findModule(n));

		return true;
	}

	bool _ActuatorBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _ActuatorBase::update(void)
	{
		IF_(!check());
	}

	int _ActuatorBase::getID(void)
	{
		return m_ID;
	}

	void _ActuatorBase::power(bool bON)
	{
		if (bON)
			m_bfSet.set(actuator_power);
		else
			m_bfSet.clear(actuator_power);
	}

	void _ActuatorBase::move(bool bMove)
	{
		if (bMove)
			m_bfSet.set(actuator_move);
		else
			m_bfSet.clear(actuator_move);
	}

	void _ActuatorBase::gotoOrigin(void)
	{
		m_p.setTarget(m_origin);
		m_bfSet.set(actuator_gotoOrigin);
	}

	void _ActuatorBase::clearAlarm(void)
	{
		m_bfSet.set(actuator_clearAlarm);
	}

	bool _ActuatorBase::bComplete(void)
	{
		IF_F(!m_p.bComplete());

		return true;
	}

	ACTUATOR_V *_ActuatorBase::pos(void)
	{
		return &m_p;
	}

	ACTUATOR_V *_ActuatorBase::speed(void)
	{
		return &m_s;
	}

	ACTUATOR_V *_ActuatorBase::accel(void)
	{
		return &m_a;
	}

	ACTUATOR_V *_ActuatorBase::brake(void)
	{
		return &m_b;
	}

	ACTUATOR_V *_ActuatorBase::current(void)
	{
		return &m_c;
	}

	void _ActuatorBase::setBitFlag(ACTUATOR_BF_SET bf)
	{
		m_bfSet.set(bf);
	}

	void _ActuatorBase::setLastCmdTime(void)
	{
		m_tLastCmd = getTbootUs();
	}

	bool _ActuatorBase::bCmdTimeout(void)
	{
		uint64_t t = getTbootUs();
		IF_F(t - m_tLastCmd < m_tCmdTimeout);

		return true;
	}

	void _ActuatorBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		const int nD = 5;

		pC->addMsg("ID: " + i2str(m_ID), 1);

		ACTUATOR_V *pV;

		pV = pos();
		pC->addMsg("p=" + f2str(pV->m_v) +
					   ", pT=" + f2str(pV->m_vTarget) +
					   ", pRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
					   ", pOrigin=" + f2str(m_origin) +
					   ", pE=" + f2str(pV->m_vErr),
				   1);

		pV = speed();
		pC->addMsg("s=" + f2str(pV->m_v) +
					   ", sT=" + f2str(pV->m_vTarget) +
					   ", sRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
					   ", sE=" + f2str(pV->m_vErr),
				   1);

		pV = accel();
		pC->addMsg("a=" + f2str(pV->m_v) +
					   ", aT=" + f2str(pV->m_vTarget) +
					   ", aRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
					   ", aE=" + f2str(pV->m_vErr),
				   1);

		pV = brake();
		pC->addMsg("b=" + f2str(pV->m_v) +
					   ", bT=" + f2str(pV->m_vTarget) +
					   ", bRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
					   ", bE=" + f2str(pV->m_vErr),
				   1);

		pV = current();
		pC->addMsg("c=" + f2str(pV->m_v) +
					   ", cT=" + f2str(pV->m_vTarget) +
					   ", cRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
					   ", cE=" + f2str(pV->m_vErr),
				   1);
	}

}
