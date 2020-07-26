/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

_ActuatorBase::_ActuatorBase()
{
	m_nAxis = 0;
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

	Kiss *pAxis = pK->child("Axis");
	int i = 0;
	while (1)
	{
		Kiss *pA = pAxis->child(i++);
		if (pA->empty())
			break;

		ACTUATOR_AXIS aa;
		aa.init();
		pA->v("nP", &aa.m_nP);
		pA->v("nTargetP", &aa.m_nPtarget);
		pA->v("nOriginP", &aa.m_nPorigin);
		pA->v("nPerr", &aa.m_nPerr);
		pA->v("nSpeed", &aa.m_nS);
		pA->v("nTargetSpeed", &aa.m_nStarget);
		pA->v("vRangeP", &aa.m_vPrange);
		pA->v("vRangeSpeed", &aa.m_vSrange);
		m_vAxis.push_back(aa);
	}

	IF_F(m_vAxis.size() < m_nAxis);

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
	for(ACTUATOR_AXIS aa : m_vAxis)
	{
		IF_F(!aa.bComplete());
	}

	return true;
}

float _ActuatorBase::getP(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_nP;
}

float _ActuatorBase::getS(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].m_nS;
}

float _ActuatorBase::getRawP(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].getRawP();
}

float _ActuatorBase::getRawS(int i)
{
	IF__(i<0 || i>=m_vAxis.size(), -1);

	return m_vAxis[i].getRawS();
}


void _ActuatorBase::draw(void)
{
	this->_ThreadBase::draw();

	addMsg("-- State --",1);

	for(int i=0; i<m_vAxis.size(); i++)
	{
		ACTUATOR_AXIS* pA = &m_vAxis[i];

		addMsg("Axis " + i2str(i), 1);
		addMsg("nP = " + f2str(pA->m_nP) + ", nTargetP = " + f2str(pA->m_nPtarget) + ", nOriginP = " + f2str(pA->m_nPorigin) + ", nPerr" + f2str(pA->m_nPerr), 1);
		addMsg("nS = " + f2str(pA->m_nS) + ", nTargetS = " + f2str(pA->m_nStarget), 1);
		addMsg("p = " + f2str(pA->m_p) + ", vRangeP = [" + f2str(pA->m_vPrange.x) + ", " + f2str(pA->m_vPrange.y) + "]", 1);
		addMsg("s = " + f2str(pA->m_s) + ", vRangeS = [" + f2str(pA->m_vSrange.x) + ", " + f2str(pA->m_vSrange.y) + "]", 1);
		addMsg("-----------------------", 1);
	}
}

}
