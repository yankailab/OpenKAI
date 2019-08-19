/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

_ActuatorBase::_ActuatorBase()
{
	m_vNormPos.init(-1.0);
	m_vNormTargetPos.init(-1.0);
	m_vNormSpeed.init(0.0);
	m_vNormTargetSpeed.init(0.0);
	m_vNormPosErr.init(0.01);

	m_tStampCmdSet = 0;
	m_tStampCmdSent = 0;

	m_pParent = NULL;
}

_ActuatorBase::~_ActuatorBase()
{
}

bool _ActuatorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vNormPos", &m_vNormPos);
	pK->v("vNormTargetPos", &m_vNormTargetPos);
	pK->v("vNormTargetSpeed", &m_vNormTargetSpeed);
	pK->v("vNormPosErr", &m_vNormPosErr);

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

void _ActuatorBase::move(vFloat3& vSpeed)
{
	m_vNormTargetSpeed.x = (vSpeed.x >= 0.0)?constrain(vSpeed.x, 0.0f, 1.0f):-1.0;
	m_vNormTargetSpeed.y = (vSpeed.y >= 0.0)?constrain(vSpeed.y, 0.0f, 1.0f):-1.0;
	m_vNormTargetSpeed.z = (vSpeed.z >= 0.0)?constrain(vSpeed.z, 0.0f, 1.0f):-1.0;

	m_tStampCmdSet = getTimeUsec();
}

void _ActuatorBase::moveTo(vFloat3& vPos, vFloat3& vSpeed)
{
	m_vNormTargetPos.x = (vPos.x >= 0.0)?constrain(vPos.x, 0.0f, 1.0f):-1.0;
	m_vNormTargetPos.y = (vPos.y >= 0.0)?constrain(vPos.y, 0.0f, 1.0f):-1.0;
	m_vNormTargetPos.z = (vPos.z >= 0.0)?constrain(vPos.z, 0.0f, 1.0f):-1.0;

	m_vNormTargetSpeed.x = (vSpeed.x >= 0.0)?constrain(vSpeed.x, 0.0f, 1.0f):-1.0;
	m_vNormTargetSpeed.y = (vSpeed.y >= 0.0)?constrain(vSpeed.y, 0.0f, 1.0f):-1.0;
	m_vNormTargetSpeed.z = (vSpeed.z >= 0.0)?constrain(vSpeed.z, 0.0f, 1.0f):-1.0;

	m_tStampCmdSet = getTimeUsec();
}

void _ActuatorBase::moveToOrigin(void)
{
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

	return true;
}

float _ActuatorBase::pos(void)
{
	return m_vNormPos.x;
}

float _ActuatorBase::speed(void)
{
	return m_vNormSpeed.x;
}

bool _ActuatorBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	return true;
}

bool _ActuatorBase::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));
	string msg;

	C_MSG("-- Normalized state --");
	C_MSG("Current pos: (" + f2str(m_vNormPos.x) + ", " + f2str(m_vNormPos.y) + ", " + f2str(m_vNormPos.z) + ")");
	C_MSG("Target pos: (" + f2str(m_vNormTargetPos.x) + ", " + f2str(m_vNormTargetPos.y) + ", " + f2str(m_vNormTargetPos.z) + ")");
	C_MSG("Speed: (" + f2str(m_vNormSpeed.x) + ", " + f2str(m_vNormSpeed.y) + ", " + f2str(m_vNormSpeed.z) + ")");

	return true;
}

}
