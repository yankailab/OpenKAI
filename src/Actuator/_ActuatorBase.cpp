/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

	_ActuatorBase::_ActuatorBase()
	{
		m_bPower = false;
		m_bReady = false;
		m_bFeedback = true;
		m_bMoving = false;

		m_lastCmdType = actCmd_standby;
		m_tLastCmd = 0;
		m_tCmdTimeout = SEC_2_USEC;

		pthread_mutex_init(&m_mutex, NULL);
		m_pParent = NULL;

		m_bf.clearAll();

		m_pOrigin = 0.0;
		m_p.init();
		m_sDir = 1.0;
		m_s.init();
		m_a.init();
		m_b.init();
		m_c.init();
	}

	_ActuatorBase::~_ActuatorBase()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	int _ActuatorBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("tCmdTimeout", &m_tCmdTimeout);

		pK->v("Porigin", &m_pOrigin);
		pK->v("p", &m_p.m_v);
		pK->v("pTarget", &m_p.m_vTarget);
		pK->v("pErr", &m_p.m_vErr);
		pK->v("pRange", &m_p.m_vRange);

		pK->v("sDir", &m_sDir);
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

	bool _ActuatorBase::power(bool bON)
	{
		return true;
	}

	void _ActuatorBase::setStop(void)
	{
		m_bf.set(ACT_BF_STOP);
	}

	int _ActuatorBase::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _ActuatorBase::update(void)
	{
		IF_(check() != OK_OK);
		//	m_pT->setTstamp(getApproxTbootUs());
	}

	bool _ActuatorBase::bCmdTimeout(void)
	{
		uint64_t t = getApproxTbootUs();
		IF_F(t - m_tLastCmd < m_tCmdTimeout);

		return true;
	}

	bool _ActuatorBase::open(void)
	{
		return false;
	}

	void _ActuatorBase::atomicFrom(void)
	{
		pthread_mutex_lock(&m_mutex);
	}

	void _ActuatorBase::atomicTo(void)
	{
		pthread_mutex_unlock(&m_mutex);
	}

	void _ActuatorBase::setPtarget(float p, bool bNormalized)
	{
		if (!bNormalized)
			m_p.setTarget(p);
		else
			m_p.setNormalizedTarget(p);

		m_lastCmdType = actCmd_pos;
		m_tLastCmd = m_pT->getTfrom();
	}

	void _ActuatorBase::setStarget(float s, bool bNormalized)
	{
		s *= m_sDir;

		if (!bNormalized)
			m_s.setTarget(s);
		else
			m_s.setNormalizedTarget(s);

		m_lastCmdType = actCmd_spd;
		m_tLastCmd = m_pT->getTfrom();
	}

	void _ActuatorBase::setAtarget(float a, bool bNormalized)
	{
		if (!bNormalized)
			m_a.setTarget(a);
		else
			m_a.setNormalizedTarget(a);

		m_lastCmdType = actCmd_accel;
		m_tLastCmd = m_pT->getTfrom();
	}

	void _ActuatorBase::setBtarget(float b, bool bNormalized)
	{
		if (!bNormalized)
			m_b.setTarget(b);
		else
			m_b.setNormalizedTarget(b);

		m_lastCmdType = actCmd_brake;
		m_tLastCmd = m_pT->getTfrom();
	}

	float _ActuatorBase::getPtarget(void)
	{
		return m_p.m_vTarget;
	}

	float _ActuatorBase::getStarget(void)
	{
		return m_s.m_vTarget;
	}

	float _ActuatorBase::getAtarget(void)
	{
		return m_a.m_vTarget;
	}

	float _ActuatorBase::getBtarget(void)
	{
		return m_b.m_vTarget;
	}

	float _ActuatorBase::getCtarget(void)
	{
		return m_c.m_vTarget;
	}

	float _ActuatorBase::getP(void)
	{
		return m_p.m_v;
	}

	float _ActuatorBase::getS(void)
	{
		return m_s.m_v;
	}

	float _ActuatorBase::getA(void)
	{
		return m_a.m_v;
	}

	float _ActuatorBase::getB(void)
	{
		return m_b.m_v;
	}

	float _ActuatorBase::getC(void)
	{
		return m_c.m_v;
	}

	void _ActuatorBase::setP(float p)
	{
		m_p.m_v = p;
	}

	void _ActuatorBase::setS(float s)
	{
		m_s.m_v = s;
	}

	void _ActuatorBase::setA(float a)
	{
		m_a.m_v = a;
	}

	void _ActuatorBase::setB(float b)
	{
		m_b.m_v = b;
	}

	void _ActuatorBase::setC(float c)
	{
		m_c.m_v = c;
	}

	void _ActuatorBase::gotoOrigin(void)
	{
		gotoOrigin();

		m_lastCmdType = actCmd_pos;
		m_tLastCmd = m_pT->getTfrom();
	}

	bool _ActuatorBase::bComplete(void)
	{
		IF_F(!m_p.bComplete());

		return true;
	}

	void _ActuatorBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("-----------------------", 1);
		pC->addMsg("p=" + f2str(m_p.m_v) +
					   ", pT=" + f2str(m_p.m_vTarget) +
					   ", pRange=[" + f2str(m_p.m_vRange.x) + ", " + f2str(m_p.m_vRange.y) + "]" +
					   ", pO=" + f2str(m_pOrigin) +
					   ", pE=" + f2str(m_p.m_vErr),
				   1);

		pC->addMsg("s=" + f2str(m_s.m_v) +
					   ", sT=" + f2str(m_s.m_vTarget) +
					   ", sRange=[" + f2str(m_s.m_vRange.x) + ", " + f2str(m_s.m_vRange.y) + "]" +
					   ", sE=" + f2str(m_s.m_vErr),
				   1);

		pC->addMsg("a=" + f2str(m_a.m_v) +
					   ", aT=" + f2str(m_a.m_vTarget) +
					   ", aRange=[" + f2str(m_a.m_vRange.x) + ", " + f2str(m_a.m_vRange.y) + "]" +
					   ", aE=" + f2str(m_a.m_vErr),
				   1);

		pC->addMsg("b=" + f2str(m_b.m_v) +
					   ", bT=" + f2str(m_b.m_vTarget) +
					   ", bRange=[" + f2str(m_b.m_vRange.x) + ", " + f2str(m_b.m_vRange.y) + "]" +
					   ", bE=" + f2str(m_b.m_vErr),
				   1);

		pC->addMsg("c=" + f2str(m_c.m_v) +
					   ", cT=" + f2str(m_c.m_vTarget) +
					   ", cRange=[" + f2str(m_c.m_vRange.x) + ", " + f2str(m_c.m_vRange.y) + "]" +
					   ", cE=" + f2str(m_c.m_vErr),
				   1);
	}

}
