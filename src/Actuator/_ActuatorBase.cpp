/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

_ActuatorBase::_ActuatorBase()
{
	m_vNoriginPos.init(0.0);
	m_vNpos.init(-1.0);
	m_vNtargetPos.init(-1.0);
	m_vNposErr.init(0.01);
	m_vNspeed.init(0.0);
	m_vNtargetSpeed.init(0.0);

	m_vNoriginAngle.init(0.0);
	m_vNangle.init(-1.0);
	m_vNtargetAngle.init(-1.0);
	m_vNangleErr.init(0.01);
	m_vNrotSpeed.init(0.0);
	m_vNtargetRotSpeed.init(0.0);

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

	pK->v("vNoriginPos", &m_vNoriginPos);
	pK->v("vNpos", &m_vNpos);
	pK->v("vNtargetPos", &m_vNtargetPos);
	pK->v("vNposErr", &m_vNposErr);
	pK->v("vNtargetSpeed", &m_vNtargetSpeed);

	pK->v("vNoriginAngle", &m_vNoriginAngle);
	pK->v("vNangle", &m_vNangle);
	pK->v("vNtargetAngle", &m_vNtargetAngle);
	pK->v("vNangleErr", &m_vNangleErr);
	pK->v("vNtargetRotSpeed", &m_vNtargetSpeed);

	pK->v("bFeedback",&m_bFeedback);

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

void _ActuatorBase::pos(vFloat4& vPos)
{
	m_vNtargetPos.x = constrain(vPos.x, -1.0f, 1.0f);
	m_vNtargetPos.y = constrain(vPos.y, -1.0f, 1.0f);
	m_vNtargetPos.z = constrain(vPos.z, -1.0f, 1.0f);
	m_vNtargetPos.w = constrain(vPos.w, -1.0f, 1.0f);
}

void _ActuatorBase::speed(vFloat4& vSpeed)
{
	m_vNtargetSpeed.x = constrain(vSpeed.x, -1.0f, 1.0f);
	m_vNtargetSpeed.y = constrain(vSpeed.y, -1.0f, 1.0f);
	m_vNtargetSpeed.z = constrain(vSpeed.z, -1.0f, 1.0f);
	m_vNtargetSpeed.w = constrain(vSpeed.w, -1.0f, 1.0f);

	if((m_vNpos.x >= 1.0 && m_vNtargetSpeed.x > 0.5) || (m_vNpos.x <= 0.0 && m_vNtargetSpeed.x < 0.5))m_vNtargetSpeed.x = 0.5;
	if((m_vNpos.y >= 1.0 && m_vNtargetSpeed.y > 0.5) || (m_vNpos.y <= 0.0 && m_vNtargetSpeed.y < 0.5))m_vNtargetSpeed.y = 0.5;
	if((m_vNpos.z >= 1.0 && m_vNtargetSpeed.z > 0.5) || (m_vNpos.z <= 0.0 && m_vNtargetSpeed.z < 0.5))m_vNtargetSpeed.z = 0.5;
	if((m_vNpos.w >= 1.0 && m_vNtargetSpeed.w > 0.5) || (m_vNpos.w <= 0.0 && m_vNtargetSpeed.w < 0.5))m_vNtargetSpeed.w = 0.5;
}

void _ActuatorBase::angle(vFloat4& vAngle)
{
	m_vNtargetAngle.x = constrain(vAngle.x, -1.0f, 1.0f);
	m_vNtargetAngle.y = constrain(vAngle.y, -1.0f, 1.0f);
	m_vNtargetAngle.z = constrain(vAngle.z, -1.0f, 1.0f);
	m_vNtargetAngle.w = constrain(vAngle.w, -1.0f, 1.0f);
}

void _ActuatorBase::rotate(vFloat4& vRot)
{
	m_vNtargetRotSpeed.x = constrain(vRot.x, -1.0f, 1.0f);
	m_vNtargetRotSpeed.y = constrain(vRot.y, -1.0f, 1.0f);
	m_vNtargetRotSpeed.z = constrain(vRot.z, -1.0f, 1.0f);
	m_vNtargetRotSpeed.w = constrain(vRot.w, -1.0f, 1.0f);

	if((m_vNangle.x >= 1.0 && m_vNtargetRotSpeed.x > 0.5) || (m_vNangle.x <= 0.0 && m_vNtargetRotSpeed.x < 0.5))m_vNtargetRotSpeed.x = 0.5;
	if((m_vNangle.y >= 1.0 && m_vNtargetRotSpeed.y > 0.5) || (m_vNangle.y <= 0.0 && m_vNtargetRotSpeed.y < 0.5))m_vNtargetRotSpeed.y = 0.5;
	if((m_vNangle.z >= 1.0 && m_vNtargetRotSpeed.z > 0.5) || (m_vNangle.z <= 0.0 && m_vNtargetRotSpeed.z < 0.5))m_vNtargetRotSpeed.z = 0.5;
	if((m_vNangle.w >= 1.0 && m_vNtargetRotSpeed.w > 0.5) || (m_vNangle.w <= 0.0 && m_vNtargetRotSpeed.w < 0.5))m_vNtargetRotSpeed.w = 0.5;
}

void _ActuatorBase::gotoOrigin(void)
{
	pos(m_vNoriginPos);
}

void _ActuatorBase::setGlobalTarget(vFloat4& t)
{

}

bool _ActuatorBase::bComplete(void)
{
	if(m_vNtargetPos.x >= 0.0)
	{
		IF_F(!EAQ(m_vNpos.x, m_vNtargetPos.x, m_vNposErr.x));
	}

	if(m_vNtargetPos.y >= 0.0)
	{
		IF_F(!EAQ(m_vNpos.y, m_vNtargetPos.y, m_vNposErr.y));
	}

	if(m_vNtargetPos.z >= 0.0)
	{
		IF_F(!EAQ(m_vNpos.z, m_vNtargetPos.z, m_vNposErr.z));
	}

	if(m_vNtargetPos.w >= 0.0)
	{
		IF_F(!EAQ(m_vNpos.w, m_vNtargetPos.w, m_vNposErr.w));
	}

	return true;
}

vFloat4 _ActuatorBase::getPos(void)
{
	return m_vNpos;
}

vFloat4 _ActuatorBase::getSpeed(void)
{
	return m_vNspeed;
}

void _ActuatorBase::draw(void)
{
	this->_ThreadBase::draw();

	addMsg("-- Normalized state --",1);
	addMsg("vNpos = (" + f2str(m_vNpos.x) + ", " + f2str(m_vNpos.y) + ", " + f2str(m_vNpos.z) + ", " + f2str(m_vNpos.w) + ")", 1);
	addMsg("vNtargetPos = (" + f2str(m_vNtargetPos.x) + ", " + f2str(m_vNtargetPos.y) + ", " + f2str(m_vNtargetPos.z) + ", " + f2str(m_vNtargetPos.w) + ")", 1);
	addMsg("vNrot = (" + f2str(m_vNangle.x) + ", " + f2str(m_vNangle.y) + ", " + f2str(m_vNangle.z) + ", " + f2str(m_vNangle.w) + ")", 1);
	addMsg("vNtargetRot = (" + f2str(m_vNtargetAngle.x) + ", " + f2str(m_vNtargetAngle.y) + ", " + f2str(m_vNtargetAngle.z) + ", " + f2str(m_vNtargetAngle.w) + ")", 1);
	addMsg("vNspeed = (" + f2str(m_vNspeed.x) + ", " + f2str(m_vNspeed.y) + ", " + f2str(m_vNspeed.z) + ", " + f2str(m_vNspeed.w) + ")", 1);
	addMsg("vNtargetSpeed = (" + f2str(m_vNtargetSpeed.x) + ", " + f2str(m_vNtargetSpeed.y) + ", " + f2str(m_vNtargetSpeed.z) + ", " + f2str(m_vNtargetSpeed.w) + ")", 1);

}

}
