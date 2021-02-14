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
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _StepperGripper::update(void)
{
	while(!initPos());
	m_bState = true;

	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateMove();

		m_pT->autoFPSto();
	}
}

void _StepperGripper::updateMove(void)
{
	IF_(check()<0);

	IF_(m_bState == m_bOpen);
	IF_(!setMove(!m_bState));
	IF_(!run());
	while(!bComplete());
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
	m_pT->sleepT (m_cmdInt);

	pB[0] = (step > 0)?0:1;
	IF_F(m_pMB->writeRegisters(m_iSlave, 11, 1, pB) != 1);
	m_pT->sleepT (m_cmdInt);

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
