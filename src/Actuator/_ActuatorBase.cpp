/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

_ActuatorBase::_ActuatorBase()
{
	m_vNormOriginPos.init(0.0);
	m_vNormPos.init(-1.0);
	m_vNormTargetPos.init(-1.0);
	m_vNormPosErr.init(0.01);

	m_vNormSpeed.init(0.0);
	m_vNormTargetSpeed.init(0.0);

	m_vNormOriginRot.init(0.0);
	m_vNormRot.init(-1.0);
	m_vNormTargetRot.init(-1.0);
	m_vNormRotErr.init(0.01);

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

	pK->v("vNormOriginPos", &m_vNormOriginPos);
	pK->v("vNormPos", &m_vNormPos);
	pK->v("vNormTargetPos", &m_vNormTargetPos);
	pK->v("vNormPosErr", &m_vNormPosErr);

	pK->v("vNormTargetSpeed", &m_vNormTargetSpeed);

	pK->v("vNormOriginRot", &m_vNormOriginRot);
	pK->v("vNormRot", &m_vNormRot);
	pK->v("vNormTargetRot", &m_vNormTargetRot);
	pK->v("vNormRotErr", &m_vNormRotErr);

	pK->v("bFeedback",&m_bFeedback);

	string iName;

	iName = "";
	F_INFO(pK->v("_ActuatorBase", &iName));
	m_pParent = (_ActuatorBase*) (pK->root()->getChildInst(iName));

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

void _ActuatorBase::pos(vFloat4& vPos)
{
	m_vNormTargetPos.x = (vPos.x >= 0.0)?constrain(vPos.x, 0.0f, 1.0f):-1.0;
	m_vNormTargetPos.y = (vPos.y >= 0.0)?constrain(vPos.y, 0.0f, 1.0f):-1.0;
	m_vNormTargetPos.z = (vPos.z >= 0.0)?constrain(vPos.z, 0.0f, 1.0f):-1.0;
	m_vNormTargetPos.w = (vPos.w >= 0.0)?constrain(vPos.w, 0.0f, 1.0f):-1.0;
}

void _ActuatorBase::speed(vFloat4& vSpeed)
{
	m_vNormTargetSpeed.x = (vSpeed.x >= 0.0)?constrain(vSpeed.x, 0.0f, 1.0f):-1.0;
	m_vNormTargetSpeed.y = (vSpeed.y >= 0.0)?constrain(vSpeed.y, 0.0f, 1.0f):-1.0;
	m_vNormTargetSpeed.z = (vSpeed.z >= 0.0)?constrain(vSpeed.z, 0.0f, 1.0f):-1.0;
	m_vNormTargetSpeed.w = (vSpeed.w >= 0.0)?constrain(vSpeed.w, 0.0f, 1.0f):-1.0;
}

void _ActuatorBase::rot(vFloat4& vRot)
{
	m_vNormTargetRot.x = (vRot.x >= 0.0)?constrain(vRot.x, 0.0f, 1.0f):-1.0;
	m_vNormTargetRot.y = (vRot.y >= 0.0)?constrain(vRot.y, 0.0f, 1.0f):-1.0;
	m_vNormTargetRot.z = (vRot.z >= 0.0)?constrain(vRot.z, 0.0f, 1.0f):-1.0;
	m_vNormTargetRot.w = (vRot.w >= 0.0)?constrain(vRot.w, 0.0f, 1.0f):-1.0;
}

void _ActuatorBase::gotoOrigin(void)
{
	pos(m_vNormOriginPos);
}

void _ActuatorBase::setGlobalTarget(vFloat4& t)
{

}

bool _ActuatorBase::bComplete(void)
{
	if(m_vNormTargetPos.x >= 0.0)
	{
		IF_F(!EAQ(m_vNormPos.x, m_vNormTargetPos.x, m_vNormPosErr.x));
	}

	if(m_vNormTargetPos.y >= 0.0)
	{
		IF_F(!EAQ(m_vNormPos.y, m_vNormTargetPos.y, m_vNormPosErr.y));
	}

	if(m_vNormTargetPos.z >= 0.0)
	{
		IF_F(!EAQ(m_vNormPos.z, m_vNormTargetPos.z, m_vNormPosErr.z));
	}

	if(m_vNormTargetPos.w >= 0.0)
	{
		IF_F(!EAQ(m_vNormPos.w, m_vNormTargetPos.w, m_vNormPosErr.w));
	}

	return true;
}

vFloat4 _ActuatorBase::getPos(void)
{
	return m_vNormPos;
}

vFloat4 _ActuatorBase::getSpeed(void)
{
	return m_vNormSpeed;
}

void _ActuatorBase::draw(void)
{
	this->_ThreadBase::draw();

	addMsg("-- Normalized state --",1);
	addMsg("vNormPos = (" + f2str(m_vNormPos.x) + ", " + f2str(m_vNormPos.y) + ", " + f2str(m_vNormPos.z) + ", " + f2str(m_vNormPos.w) + ")", 1);
	addMsg("vNormTargetPos = (" + f2str(m_vNormTargetPos.x) + ", " + f2str(m_vNormTargetPos.y) + ", " + f2str(m_vNormTargetPos.z) + ", " + f2str(m_vNormTargetPos.w) + ")", 1);
	addMsg("vNormRot = (" + f2str(m_vNormRot.x) + ", " + f2str(m_vNormRot.y) + ", " + f2str(m_vNormRot.z) + ", " + f2str(m_vNormRot.w) + ")", 1);
	addMsg("vNormTargetRot = (" + f2str(m_vNormTargetRot.x) + ", " + f2str(m_vNormTargetRot.y) + ", " + f2str(m_vNormTargetRot.z) + ", " + f2str(m_vNormTargetRot.w) + ")", 1);
	addMsg("vNormSpeed = (" + f2str(m_vNormSpeed.x) + ", " + f2str(m_vNormSpeed.y) + ", " + f2str(m_vNormSpeed.z) + ", " + f2str(m_vNormSpeed.w) + ")", 1);
	addMsg("vNormTargetSpeed = (" + f2str(m_vNormTargetSpeed.x) + ", " + f2str(m_vNormTargetSpeed.y) + ", " + f2str(m_vNormTargetSpeed.z) + ", " + f2str(m_vNormTargetSpeed.w) + ")", 1);

}

}
