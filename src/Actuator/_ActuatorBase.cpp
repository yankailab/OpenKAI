/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

	_ActuatorBase::_ActuatorBase()
	{
		m_ID = 0;
		m_origin = 0;
		m_p.init();
		m_s.init();
		m_a.init();
		m_b.init();
		m_c.init();

		m_tLastCmd = 0;
		m_tCmdTimeout = 0;
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

	int _ActuatorBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("ID", &m_ID);
		
		pK->v("pOrigin", &m_origin);
		pK->v("p", &m_p.m_v);
		pK->v("pTarget", &m_p.m_vTarget);
		pK->v("pErr", &m_p.m_vErr);
		pK->v("pRange", &m_p.m_vRange);

		pK->v("s", &m_s.m_v);
		pK->v("sTarget", &m_s.m_vTarget);
		pK->v("sErr", &m_s.m_vErr);
		pK->v("sRange", &m_s.m_vRange);

		pK->v("a", &m_a.m_v);
		pK->v("aTarget", &m_a.m_vTarget);
		pK->v("aErr", &m_a.m_vErr);
		pK->v("aRange", &m_a.m_vRange);

		pK->v("b", &m_b.m_v);
		pK->v("bTarget", &m_b.m_vTarget);
		pK->v("bErr", &m_b.m_vErr);
		pK->v("bRange", &m_b.m_vRange);

		pK->v("c", &m_c.m_v);
		pK->v("cTarget", &m_c.m_vTarget);
		pK->v("cErr", &m_c.m_vErr);
		pK->v("cRange", &m_c.m_vRange);

		pK->v("tCmdTimeout", &m_tCmdTimeout);
		pK->v("tIntCheckAlarm", &m_ieCheckAlarm.m_tInterval);
		pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);
		pK->v("tIntSendCMD", &m_ieSendCMD.m_tInterval);

		int bf;

		bf = 0;
		pK->v("bfSetPower", &bf);
		if (bf)
			m_bfSet.set(actuator_power);

		bf = 0;
		pK->v("bfSetID", &bf);
		if (bf)
			m_bfSet.set(actuator_setID);

		bf = 0;
		pK->v("bfSetMode", &bf);
		if (bf)
			m_bfSet.set(actuator_setMode);

		bf = 0;
		pK->v("bfMove", &bf);
		if (bf)
			m_bfSet.set(actuator_move);

		return OK_OK;
	}

	int _ActuatorBase::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		n = "";
		pK->v("_ActuatorBase", &n);
		m_pParent = (_ActuatorBase *)(pK->findModule(n));

		return OK_OK;
	}

	int _ActuatorBase::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _ActuatorBase::update(void)
	{
		IF_(check() != OK_OK);
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
