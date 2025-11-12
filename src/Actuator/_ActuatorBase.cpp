/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

	_ActuatorBase::_ActuatorBase()
	{
		pthread_mutex_init(&m_mutex, NULL);
		m_pParent = NULL;
	}

	_ActuatorBase::~_ActuatorBase()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	int _ActuatorBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pKchannels = pK->child("channels");
		IF__(pKchannels->empty(), OK_OK);

		int i = 0;
		while (1)
		{
			Kiss *pKc = pKchannels->child(i++);
			if (pKc->empty())
				break;

			ACTUATOR_CHAN c;
			c.init();

			pKc->v("ID", &c.m_ID);
			pKc->v("tCmdTimeout", &c.m_tCmdTimeout);
			pKc->v("mode", (int *)&c.m_mode);
			pKc->v("bPower", &c.m_bPower);

			int bf;

			bf = 0;
			pKc->v("setPower", &bf);
			if (bf)
				c.m_bfSet.set(actuator_power);

			bf = 0;
			pKc->v("setID", &bf);
			if (bf)
				c.m_bfSet.set(actuator_setID);

			bf = 0;
			pKc->v("setMode", &bf);
			if (bf)
				c.m_bfSet.set(actuator_setMode);

			pKc->v("Porigin", &c.m_pOrigin);
			pKc->v("p", &c.m_p.m_v);
			pKc->v("pTarget", &c.m_p.m_vTarget);
			pKc->v("pErr", &c.m_p.m_vErr);
			pKc->v("pRange", &c.m_p.m_vRange);

			pKc->v("sCofactor", &c.m_s.m_vCofactor);
			pKc->v("s", &c.m_s.m_v);
			pKc->v("sTarget", &c.m_s.m_vTarget);
			pKc->v("sErr", &c.m_s.m_vErr);
			pKc->v("sRange", &c.m_s.m_vRange);

			pKc->v("a", &c.m_a.m_v);
			pKc->v("aTarget", &c.m_a.m_vTarget);
			pKc->v("aErr", &c.m_a.m_vErr);
			pKc->v("aRange", &c.m_a.m_vRange);

			pKc->v("b", &c.m_b.m_v);
			pKc->v("bTarget", &c.m_b.m_vTarget);
			pKc->v("bErr", &c.m_b.m_vErr);
			pKc->v("bRange", &c.m_b.m_vRange);

			pKc->v("c", &c.m_c.m_v);
			pKc->v("cTarget", &c.m_c.m_vTarget);
			pKc->v("cErr", &c.m_c.m_vErr);
			pKc->v("cRange", &c.m_c.m_vRange);

			m_vChan.push_back(c);
		}

		if (m_vChan.empty())
		{
			ACTUATOR_CHAN c;
			c.init();
			m_vChan.push_back(c);
		}

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

	bool _ActuatorBase::open(void)
	{
		return false;
	}

	// void _ActuatorBase::atomicFrom(void)
	// {
	// 	pthread_mutex_lock(&m_mutex);
	// }

	// void _ActuatorBase::atomicTo(void)
	// {
	// 	pthread_mutex_unlock(&m_mutex);
	// }

	ACTUATOR_CHAN *_ActuatorBase::getChan(int iChan)
	{
		IF_N(m_vChan.empty())

		return &m_vChan[iChan];
	}

	bool _ActuatorBase::bCmdTimeout(int iChan)
	{
		return getChan(iChan)->bCmdTimeout();
	}

	void _ActuatorBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		const int nD = 5;

		for (int i = 0; i < m_vChan.size(); i++)
		{
			ACTUATOR_CHAN *pChan = getChan(i);
			ACTUATOR_V *pV;

			pC->addMsg("-----------------------", 1);
			pC->addMsg("Chan " + i2str(i), 1);

			pV = pChan->pos();
			pC->addMsg("p=" + f2str(pV->m_v) +
						   ", pT=" + f2str(pV->m_vTarget) +
						   ", pRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
						   ", pO=" + f2str(pChan->m_pOrigin) +
						   ", pE=" + f2str(pV->m_vErr),
					   1);

			pV = pChan->speed();
			pC->addMsg("s=" + f2str(pV->m_v) +
						   ", sT=" + f2str(pV->m_vTarget) +
						   ", sRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
						   ", sE=" + f2str(pV->m_vErr),
					   1);

			pV = pChan->accel();
			pC->addMsg("a=" + f2str(pV->m_v) +
						   ", aT=" + f2str(pV->m_vTarget) +
						   ", aRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
						   ", aE=" + f2str(pV->m_vErr),
					   1);

			pV = pChan->brake();
			pC->addMsg("b=" + f2str(pV->m_v) +
						   ", bT=" + f2str(pV->m_vTarget) +
						   ", bRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
						   ", bE=" + f2str(pV->m_vErr),
					   1);

			pV = pChan->current();
			pC->addMsg("c=" + f2str(pV->m_v) +
						   ", cT=" + f2str(pV->m_vTarget) +
						   ", cRange=[" + f2str(pV->m_vRange.x, nD) + ", " + f2str(pV->m_vRange.y, nD) + "]" +
						   ", cE=" + f2str(pV->m_vErr),
					   1);
		}
	}

}
