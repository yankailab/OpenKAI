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

	m_pOpen = -1.0;
	m_pClose = 1.0;
}

_StepperGripper::~_StepperGripper()
{
}

bool _StepperGripper::init(void* pKiss)
{
	IF_F(!this->_DRV8825_RS485::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("pOpen", &m_pOpen);
	pK->v("pClose", &m_pClose);
	pK->v("bOpen", &m_bOpen);

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
	while(!initPos());
	m_bState = true;

	while (m_bThreadON)
	{
		this->autoFPSfrom();

//		readStatus();
		updateMove();

		this->autoFPSto();
	}
}

void _StepperGripper::updateMove(void)
{
	IF_(check()<0);

//	IF_(!m_pA->m_p.bComplete());
//
//	if(EQUAL(m_pA->m_p.m_v, m_pOpen, m_pA->m_p.m_vErr))
//	{
//		m_bState = true;
//	}
//	else if(EQUAL(m_pA->m_p.m_v, m_pClose, m_pA->m_p.m_vErr))
//	{
//		m_bState = false;
//	}
//	else
//	{
//		initPos();
//		return;
//	}

	IF_(m_bState == m_bOpen);

//	setPtarget(0, m_bOpen?m_pOpen:m_pClose);
	IF_(!setMove(!m_bState));
	IF_(!run());
	this->sleepTime(100000);
	while(!bComplete())
		this->sleepTime(100000);
	m_bState = !m_bState;
}

bool _StepperGripper::setMove(bool bOpen)
{
	IF_F(check()<0);

	int32_t step = m_pOpen - m_pClose;
	if(!bOpen)step = -step;

	int32_t ds = abs(step);
	IF_F(step==0);

	uint16_t pB[2];
	pB[0] = HIGH16(ds);
	pB[1] = LOW16(ds);
	IF_F(m_pMB->writeRegisters(m_iSlave, 9, 2, pB) != 2);
	this->sleepTime(100000);

	pB[0] = (step > 0)?0:1;
	IF_F(m_pMB->writeRegisters(m_iSlave, 11, 1, pB) != 1);
	this->sleepTime(100000);

	return true;
}

void _StepperGripper::grip(bool bOpen)
{
	m_bOpen = bOpen;
}

bool _StepperGripper::bGrip(void)
{
	return m_bState;
}

void _StepperGripper::draw(void)
{
	this->_DRV8825_RS485::draw();

	addMsg("bOpen=" + i2str(m_bOpen) + ", bState=" + i2str(m_bState));

}

}
