/*
 *  Created on: June 22, 2020
 *      Author: yankai
 */
#include "_StepperGripper.h"

namespace kai
{

_StepperGripper::_StepperGripper()
{
	m_bState = false;
	m_bOpen = true;
}

_StepperGripper::~_StepperGripper()
{
}

bool _StepperGripper::init(void* pKiss)
{
	IF_F(!this->_DRV8825_RS485::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _StepperGripper::start(void)
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

void _StepperGripper::update(void)
{
	while(check()<0)
		this->sleepTime(USEC_1SEC);

//	setDistPerRound(m_dpr);
//	setDist(m_vNormTargetPos.x);
//	setSpeed(m_vNormTargetSpeed.x);
//	run();

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateGripper();

		if(m_bFeedback)
			readStatus();

		this->autoFPSto();
	}
}

void _StepperGripper::updateGripper(void)
{
	IF_(m_bState == m_bOpen);

	setDistPerRound(m_dpr);
	setDist((m_bOpen)?-1.0:1.0);
	setSpeed(1.0);
	run();

	m_bState = m_bOpen;
}

void _StepperGripper::grip(bool bOpen)
{
	m_bOpen = bOpen;
}

void _StepperGripper::draw(void)
{
	this->_DRV8825_RS485::draw();

	addMsg("bOpen=" + i2str(m_bOpen) + ", bState=" + i2str(m_bState));

}

}
