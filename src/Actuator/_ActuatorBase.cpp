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
	m_pTarget = 0;
	m_pParent = NULL;
}

_ActuatorBase::~_ActuatorBase()
{
}

bool _ActuatorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

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
		pA->v("pRaw", &a.m_p.m_vRaw);
		pA->v("pErr", &a.m_p.m_vErr);
		pA->v("pRawRange", &a.m_p.m_vRawRange);

		pA->v("s", &a.m_s.m_v);
		pA->v("sTarget", &a.m_s.m_vTarget);
		pA->v("sRaw", &a.m_s.m_vRaw);
		pA->v("sErr", &a.m_s.m_vErr);
		pA->v("sRawRange", &a.m_s.m_vRawRange);

		pA->v("a", &a.m_a.m_v);
		pA->v("aTarget", &a.m_a.m_vTarget);
		pA->v("aRaw", &a.m_a.m_vRaw);
		pA->v("aErr", &a.m_a.m_vErr);
		pA->v("aRawRange", &a.m_a.m_vRawRange);

		pA->v("b", &a.m_b.m_v);
		pA->v("bTarget", &a.m_b.m_vTarget);
		pA->v("bRaw", &a.m_b.m_vRaw);
		pA->v("bErr", &a.m_b.m_vErr);
		pA->v("bRawRange", &a.m_b.m_vRawRange);

		pA->v("c", &a.m_c.m_v);
		pA->v("cTarget", &a.m_c.m_vTarget);
		pA->v("cRaw", &a.m_c.m_vRaw);
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

bool _ActuatorBase::bComplete(void)
{
	for(ACTUATOR_AXIS a : m_vAxis)
	{
		IF_F(!a.m_p.bComplete());
	}

	return true;
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

	return m_vAxis[i].m_p.m_vRaw;
}

float _ActuatorBase::getSraw(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), FLT_MAX);

	return m_vAxis[i].m_s.m_vRaw;
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
				", pR=" + f2str(pA->m_p.m_vRaw) +
				", pRange=[" + f2str(pA->m_p.m_vRawRange.x) + ", " + f2str(pA->m_p.m_vRawRange.y) + "]" +
				", pO=" + f2str(pA->m_pOrigin) +
				", pE=" + f2str(pA->m_p.m_vErr)
				, 1);

		addMsg("s=" + f2str(pA->m_s.m_v) +
				", sT=" + f2str(pA->m_s.m_vTarget) +
				", sR=" + f2str(pA->m_s.m_vRaw) +
				", sRange=[" + f2str(pA->m_s.m_vRawRange.x) + ", " + f2str(pA->m_s.m_vRawRange.y) + "]" +
				", sE=" + f2str(pA->m_s.m_vErr)
				, 1);

		addMsg("a=" + f2str(pA->m_a.m_v) +
				", aT=" + f2str(pA->m_a.m_vTarget) +
				", aR=" + f2str(pA->m_a.m_vRaw) +
				", aRange=[" + f2str(pA->m_a.m_vRawRange.x) + ", " + f2str(pA->m_a.m_vRawRange.y) + "]" +
				", aE=" + f2str(pA->m_a.m_vErr)
				, 1);

		addMsg("b=" + f2str(pA->m_b.m_v) +
				", bT=" + f2str(pA->m_b.m_vTarget) +
				", bR=" + f2str(pA->m_b.m_vRaw) +
				", bRange=[" + f2str(pA->m_b.m_vRawRange.x) + ", " + f2str(pA->m_b.m_vRawRange.y) + "]" +
				", bE=" + f2str(pA->m_b.m_vErr)
				, 1);

		addMsg("c=" + f2str(pA->m_c.m_v) +
				", cT=" + f2str(pA->m_c.m_vTarget) +
				", cR=" + f2str(pA->m_c.m_vRaw) +
				", cRange=[" + f2str(pA->m_c.m_vRawRange.x) + ", " + f2str(pA->m_c.m_vRawRange.y) + "]" +
				", cE=" + f2str(pA->m_c.m_vErr)
				, 1);
	}
}

}
