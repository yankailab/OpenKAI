/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

_ActuatorBase::_ActuatorBase()
{
	m_bFeedback = true;
	m_nMinAxis = 1;
	m_bMoving = false;

	m_lastCmdType = actCmd_standby;
	m_tLastCmd = 0;
	m_tCmdTimeout = USEC_1SEC;

	m_pParent = NULL;
}

_ActuatorBase::~_ActuatorBase()
{
}

bool _ActuatorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
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
		pA->v("pRawRange", &a.m_p.m_vRawRange);

		pA->v("s", &a.m_s.m_v);
		pA->v("sTarget", &a.m_s.m_vTarget);
		pA->v("sErr", &a.m_s.m_vErr);
		pA->v("sRawRange", &a.m_s.m_vRawRange);

		pA->v("a", &a.m_a.m_v);
		pA->v("aTarget", &a.m_a.m_vTarget);
		pA->v("aErr", &a.m_a.m_vErr);
		pA->v("aRawRange", &a.m_a.m_vRawRange);

		pA->v("b", &a.m_b.m_v);
		pA->v("bTarget", &a.m_b.m_vTarget);
		pA->v("bErr", &a.m_b.m_vErr);
		pA->v("bRawRange", &a.m_b.m_vRawRange);

		pA->v("c", &a.m_c.m_v);
		pA->v("cTarget", &a.m_c.m_vTarget);
		pA->v("cErr", &a.m_c.m_vErr);
		pA->v("cRawRange", &a.m_c.m_vRawRange);

		m_vAxis.push_back(a);
	}

	IF_F(m_vAxis.size() < m_nMinAxis);

	string iName;

	iName = "";
	F_INFO(pK->v("_ActuatorBase", &iName));
	m_pParent = (_ActuatorBase*) (pK->getInst(iName));

	return true;
}

bool _ActuatorBase::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
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

void _ActuatorBase::setPtarget(int i, float nP)
{
	IF_(i<0 || i>=m_vAxis.size());

	ACTUATOR_AXIS* pA = &m_vAxis[i];
	pA->m_p.setTarget(nP);
}

void _ActuatorBase::setPtargetRaw(int i, float rawP)
{
	IF_(i<0 || i>=m_vAxis.size());

	ACTUATOR_AXIS* pA = &m_vAxis[i];
	pA->m_p.setTargetRaw(rawP);
}

void _ActuatorBase::setStarget(int i, float nS)
{
	IF_(i<0 || i>=m_vAxis.size());

	ACTUATOR_AXIS* pA = &m_vAxis[i];
	pA->m_s.setTarget(nS);
}

void _ActuatorBase::gotoOrigin(void)
{
	for(int i=0; i<m_vAxis.size(); i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];
		pA->gotoOrigin();
	}
}

void _ActuatorBase::setCmd(ACTUATOR_CMD_TYPE cType)
{
	m_lastCmdType = cType;
	m_tLastCmd = getTimeUsec();
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

float _ActuatorBase::getPraw(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), FLT_MAX);

	return m_vAxis[i].m_p.getRaw();
}

float _ActuatorBase::getSraw(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), FLT_MAX);

	return m_vAxis[i].m_s.getRaw();
}

void _ActuatorBase::draw(void)
{
	this->_ThreadBase::draw();

	for(int i=0; i<m_vAxis.size(); i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];

		addMsg("-----------------------", 1);
		addMsg(pA->m_name, 1);
		addMsg("p=" + f2str(pA->m_p.m_v) +
				", pT=" + f2str(pA->m_p.m_vTarget) +
				", pR=" + f2str(pA->m_p.getRaw()) +
				", pRange=[" + f2str(pA->m_p.m_vRawRange.x) + ", " + f2str(pA->m_p.m_vRawRange.y) + "]" +
				", pO=" + f2str(pA->m_pOrigin) +
				", pE=" + f2str(pA->m_p.m_vErr)
				, 1);

		addMsg("s=" + f2str(pA->m_s.m_v) +
				", sT=" + f2str(pA->m_s.m_vTarget) +
				", sR=" + f2str(pA->m_s.getRaw()) +
				", sRange=[" + f2str(pA->m_s.m_vRawRange.x) + ", " + f2str(pA->m_s.m_vRawRange.y) + "]" +
				", sE=" + f2str(pA->m_s.m_vErr)
				, 1);

		addMsg("a=" + f2str(pA->m_a.m_v) +
				", aT=" + f2str(pA->m_a.m_vTarget) +
				", aR=" + f2str(pA->m_a.getRaw()) +
				", aRange=[" + f2str(pA->m_a.m_vRawRange.x) + ", " + f2str(pA->m_a.m_vRawRange.y) + "]" +
				", aE=" + f2str(pA->m_a.m_vErr)
				, 1);

		addMsg("b=" + f2str(pA->m_b.m_v) +
				", bT=" + f2str(pA->m_b.m_vTarget) +
				", bR=" + f2str(pA->m_b.getRaw()) +
				", bRange=[" + f2str(pA->m_b.m_vRawRange.x) + ", " + f2str(pA->m_b.m_vRawRange.y) + "]" +
				", bE=" + f2str(pA->m_b.m_vErr)
				, 1);

		addMsg("c=" + f2str(pA->m_c.m_v) +
				", cT=" + f2str(pA->m_c.m_vTarget) +
				", cR=" + f2str(pA->m_c.getRaw()) +
				", cRange=[" + f2str(pA->m_c.m_vRawRange.x) + ", " + f2str(pA->m_c.m_vRawRange.y) + "]" +
				", cE=" + f2str(pA->m_c.m_vErr)
				, 1);
	}
}

}
