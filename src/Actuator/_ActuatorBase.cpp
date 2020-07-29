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
	m_bFeedback = false;
	m_pParent = NULL;
}

_ActuatorBase::~_ActuatorBase()
{
}

bool _ActuatorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("bFeedback",&m_bFeedback);

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
		pA->v("p", &a.m_p);
		pA->v("pTarget", &a.m_pTarget);
		pA->v("pOrigin", &a.m_pOrigin);
		pA->v("pErr", &a.m_pEerr);
		pA->v("vRawPrange", &a.m_vRawPrange);
		pA->v("s", &a.m_s);
		pA->v("sTarget", &a.m_sTarget);
		pA->v("vRawSrange", &a.m_vRawSrange);
		pA->v("aTarget", &a.m_aTarget);
		pA->v("vRawArange", &a.m_vRawArange);
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
	pA->setPtarget(nP);
}

void _ActuatorBase::setStarget(int i, float nS)
{
	IF_(i<0 || i>=m_vAxis.size());

	ACTUATOR_AXIS* pA = &m_vAxis[i];
	pA->setStarget(nS);
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
		IF_F(!a.bComplete());
	}

	return true;
}

float _ActuatorBase::getP(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_p;
}

float _ActuatorBase::getS(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_s;
}

float _ActuatorBase::getRawP(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), FLT_MAX);

	return m_vAxis[i].getRawP();
}

float _ActuatorBase::getRawS(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), FLT_MAX);

	return m_vAxis[i].getRawS();
}

void _ActuatorBase::draw(void)
{
	this->_ThreadBase::draw();

	addMsg("-- State --",1);

	for(int i=0; i<m_vAxis.size(); i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];

		addMsg(pA->m_name, 1);
		addMsg("nP=" + f2str(pA->m_p) + ", nPtarget=" + f2str(pA->m_pTarget) + ", nPorigin=" + f2str(pA->m_pOrigin) + ", nPerr=" + f2str(pA->m_pEerr), 1);
		addMsg("nS=" + f2str(pA->m_s) + ", nStarget=" + f2str(pA->m_sTarget), 1);
		addMsg("p=" + f2str(pA->m_rawP) + ", vPrange=[" + f2str(pA->m_vRawPrange.x) + ", " + f2str(pA->m_vRawPrange.y) + "]", 1);
		addMsg("s=" + f2str(pA->m_rawS) + ", vSrange=[" + f2str(pA->m_vRawSrange.x) + ", " + f2str(pA->m_vRawSrange.y) + "]", 1);
		addMsg("-----------------------", 1);
	}
}

}
