/*
 *  Created on: June 22, 2020
 *      Author: yankai
 */
#include "_ZLAC8015.h"

namespace kai
{

_ZLAC8015::_ZLAC8015()
{
	m_pMB = NULL;
	m_iSlave = 1;
	m_iMode = 3;   //speed control
	m_pA = NULL;

	m_ieReadStatus.init(50000);
}

_ZLAC8015::~_ZLAC8015()
{
}

bool _ZLAC8015::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iSlave", &m_iSlave);
	pK->v("iMode", &m_iMode);
	pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

	m_pA = &m_vAxis[0];

	string n;
	n = "";
	F_ERROR_F(pK->v("_Modbus", &n ));
	m_pMB = (_Modbus*) (pK->getInst( n ));
	IF_Fl(!m_pMB, n + " not found");
    
	return true;
}

bool _ZLAC8015::start(void)
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

int _ZLAC8015::check(void)
{
	NULL__(m_pMB,-1);
	IF__(!m_pMB->bOpen(),-1);
	NULL__(m_pA,-1);

	return 0;
}

void _ZLAC8015::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

        if(!m_bReady)
        {
            setup();
        }
        else
        {
            readStatus();
            updateMove();
        }

		this->autoFPSto();
	}
}

void _ZLAC8015::setup(void)
{
    IF_(!setMode());
    IF_(!setAccel());
    IF_(!setBrake());
    IF_(!m_pMB->writeRegister(m_iSlave, 0x2031, 0x08)); //power on the motor
    
    m_bReady = true;
}

void _ZLAC8015::updateMove(void)
{
	IF_(check()<0);

    setSpeed();
    
//    r = m_pMB->writeRegister(m_iSlave, 0x2031, 0x07); //power off
//    r = m_pMB->writeRegister(m_iSlave, 0x2031, 0x06); //clear alarm
}

bool _ZLAC8015::setMode(void)
{
	IF_F(check()<0);

	IF_F(m_pMB->writeRegister(m_iSlave, 0x2032, m_iMode) != 1);
//	int r = m_pMB->writeRegister(m_iSlave, 0x2032, m_iMode);
//    IF_F(r != 1);

	return true;
}

bool _ZLAC8015::setAccel(void)
{
	IF_F(check()<0);

	uint16_t v = m_pA->m_a.m_vTarget;
	IF_F(m_pMB->writeRegister(m_iSlave, 0x2037, v) != 1);

	return true;
}

bool _ZLAC8015::setBrake(void)
{
	IF_F(check()<0);

	uint16_t v = m_pA->m_b.m_vTarget;
	IF_F(m_pMB->writeRegister(m_iSlave, 0x2038, v) != 1);

	return true;
}

bool _ZLAC8015::setSpeed(void)
{
	IF_F(check()<0);

	int16_t v = m_pA->m_s.m_vTarget;
	IF_F(m_pMB->writeRegister(m_iSlave, 0x203A, v) != 1);

	return true;
}

bool _ZLAC8015::stop(void)
{
	IF_F(check()<0);

	IF_F(m_pMB->writeBit(m_iSlave, 3, true) != 1);
	return true;
}

bool _ZLAC8015::bComplete(void)
{
	IF_F(check()<0);

	uint16_t b;
	int r = m_pMB->readRegisters(m_iSlave, 12, 1, &b);
	IF_F(r != 1);

	return (b==1)?true:false;
}

bool _ZLAC8015::readStatus(void)
{
	IF_F(check()<0);
	IF_T(!m_ieReadStatus.update(m_tStamp));

	uint16_t pB[2];
	int r = m_pMB->readRegisters(m_iSlave, 0x202C, 1, pB);
	IF_F(r != 1);

//	int p = MAKE32(pB[0], pB[1]);
	int16_t p = pB[0];
	m_pA->m_p.m_v = p;

	return true;
}

void _ZLAC8015::draw(void)
{
	this->_ActuatorBase::draw();
}

}
