/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

_ActuatorBase::_ActuatorBase()
{
	m_nMinAxis = 1;

    m_bPower = false;
    m_bReady = false;
	m_bFeedback = true;
	m_bMoving = false;

	m_lastCmdType = actCmd_standby;
	m_tLastCmd = 0;
	m_tCmdTimeout = USEC_1SEC;

	pthread_mutex_init(&m_mutex, NULL);
	m_pParent = NULL;
}

_ActuatorBase::~_ActuatorBase()
{
	pthread_mutex_destroy(&m_mutex);
}

bool _ActuatorBase::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tCmdTimeout", &m_tCmdTimeout);

	Kiss *pAxis = pK->child("axis");
	int i = 0;
	while (1)
	{
		Kiss *pA = pAxis->child(i++);
		if (pA->empty())
			break;

		ACTUATOR_AXIS a;
		a.init();
		pA->v("name", &a.m_name);
		pA->v("pOrigin", &a.m_pOrigin);

		pA->v("p", &a.m_p.m_v);
		pA->v("pTarget", &a.m_p.m_vTarget);
		pA->v("pErr", &a.m_p.m_vErr);
		pA->v("pRange", &a.m_p.m_vRange);

		pA->v("s", &a.m_s.m_v);
		pA->v("sTarget", &a.m_s.m_vTarget);
		pA->v("sErr", &a.m_s.m_vErr);
		pA->v("sRange", &a.m_s.m_vRange);

		pA->v("a", &a.m_a.m_v);
		pA->v("aTarget", &a.m_a.m_vTarget);
		pA->v("aErr", &a.m_a.m_vErr);
		pA->v("aRange", &a.m_a.m_vRange);

		pA->v("b", &a.m_b.m_v);
		pA->v("bTarget", &a.m_b.m_vTarget);
		pA->v("bErr", &a.m_b.m_vErr);
		pA->v("bRange", &a.m_b.m_vRange);

		pA->v("c", &a.m_c.m_v);
		pA->v("cTarget", &a.m_c.m_vTarget);
		pA->v("cErr", &a.m_c.m_vErr);
		pA->v("cRange", &a.m_c.m_vRange);

		m_vAxis.push_back(a);
	}

	IF_F(m_vAxis.size() < m_nMinAxis);

	string n;

	n = "";
	F_INFO(pK->v("_ActuatorBase", &n ));
	m_pParent = (_ActuatorBase*) (pK->getInst( n ));

	return true;
}

bool _ActuatorBase::power(bool bON)
{
    return true;    
}

bool _ActuatorBase::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ActuatorBase::update(void)
{
	m_tStamp = getTimeUsec();
}

bool _ActuatorBase::bCmdTimeout(void)
{
	uint64_t t = getTimeUsec();
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

void _ActuatorBase::setPtarget(int i, float p)
{
	IF_(i<0);
	IF_(i>=m_vAxis.size());

	ACTUATOR_AXIS* pA = &m_vAxis[i];
	pA->m_p.setTarget(p);

	m_lastCmdType = actCmd_pos;
	m_tLastCmd = m_tStamp;
}

void _ActuatorBase::setStarget(int i, float s)
{
	IF_(i<0);
	IF_(i>=m_vAxis.size());

	ACTUATOR_AXIS* pA = &m_vAxis[i];
	pA->m_s.setTarget(s);

	m_lastCmdType = actCmd_spd;
	m_tLastCmd = m_tStamp;
}

void _ActuatorBase::gotoOrigin(void)
{
	for(int i=0; i<m_vAxis.size(); i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];
		pA->gotoOrigin();
	}

	m_lastCmdType = actCmd_pos;
	m_tLastCmd = m_tStamp;
}

bool _ActuatorBase::bComplete(void)
{
	for(ACTUATOR_AXIS a : m_vAxis)
	{
		IF_F(!a.m_p.bComplete());
	}

	return true;
}

bool _ActuatorBase::bComplete(int i)
{
	IF_F(i<0 || i>=m_vAxis.size());

	ACTUATOR_AXIS* pA = &m_vAxis[i];
	return pA->m_p.bComplete();
}

float _ActuatorBase::getP(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_p.m_v;
}

float _ActuatorBase::getS(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_s.m_v;
}

float _ActuatorBase::getPtarget(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_p.m_vTarget;
}

float _ActuatorBase::getStarget(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_s.m_vTarget;
}

void _ActuatorBase::draw(void)
{
	this->_ModuleBase::draw();

	for(int i=0; i<m_vAxis.size(); i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];

		addMsg("-----------------------", 1);
		addMsg(pA->m_name, 1);
		addMsg("p=" + f2str(pA->m_p.m_v) +
				", pT=" + f2str(pA->m_p.m_vTarget) +
				", pRange=[" + f2str(pA->m_p.m_vRange.x) + ", " + f2str(pA->m_p.m_vRange.y) + "]" +
				", pO=" + f2str(pA->m_pOrigin) +
				", pE=" + f2str(pA->m_p.m_vErr)
				, 1);

		addMsg("s=" + f2str(pA->m_s.m_v) +
				", sT=" + f2str(pA->m_s.m_vTarget) +
				", sRange=[" + f2str(pA->m_s.m_vRange.x) + ", " + f2str(pA->m_s.m_vRange.y) + "]" +
				", sE=" + f2str(pA->m_s.m_vErr)
				, 1);

		addMsg("a=" + f2str(pA->m_a.m_v) +
				", aT=" + f2str(pA->m_a.m_vTarget) +
				", aRange=[" + f2str(pA->m_a.m_vRange.x) + ", " + f2str(pA->m_a.m_vRange.y) + "]" +
				", aE=" + f2str(pA->m_a.m_vErr)
				, 1);

		addMsg("b=" + f2str(pA->m_b.m_v) +
				", bT=" + f2str(pA->m_b.m_vTarget) +
				", bRange=[" + f2str(pA->m_b.m_vRange.x) + ", " + f2str(pA->m_b.m_vRange.y) + "]" +
				", bE=" + f2str(pA->m_b.m_vErr)
				, 1);

		addMsg("c=" + f2str(pA->m_c.m_v) +
				", cT=" + f2str(pA->m_c.m_vTarget) +
				", cRange=[" + f2str(pA->m_c.m_vRange.x) + ", " + f2str(pA->m_c.m_vRange.y) + "]" +
				", cE=" + f2str(pA->m_c.m_vErr)
				, 1);
	}
}

}
