/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorBase.h"

namespace kai
{

_ActuatorBase::_ActuatorBase()
{
	m_nCurrentPos = -1.0;
	m_nTargetPos = -1.0;
	m_nCurrentSpeed = 0.0;
	m_nTargetSpeed = 0.0;
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

void _ActuatorBase::move(float nSpeed)
{
	m_nTargetSpeed = constrain(nSpeed, 0.0f, 1.0f);
	m_tStampCmdSet = getTimeUsec();
}

void _ActuatorBase::moveTo(float nPos, float nSpeed)
{
	m_nTargetPos = constrain(nPos, 0.0f, 1.0f);
	m_nTargetSpeed = constrain(nSpeed, 0.0f, 1.0f);
	m_tStampCmdSet = getTimeUsec();
}

void _ActuatorBase::moveToOrigin(void)
{
}

bool _ActuatorBase::bComplete(void)
{
	return EAQ(m_nCurrentPos, m_nTargetPos, 1e-2);
}

float _ActuatorBase::pos(void)
{
	return m_nCurrentPos;
}

float _ActuatorBase::speed(void)
{
	return m_nCurrentSpeed;
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
	C_MSG("Current pos: " + f2str(m_nCurrentPos));
	C_MSG("Target pos: " + f2str(m_nTargetPos));
	C_MSG("Speed: " + f2str(m_nTargetSpeed));

	return true;
}

}
