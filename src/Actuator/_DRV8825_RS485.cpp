/*
 *  Created on: June 22, 2020
 *      Author: yankai
 */
#include "_DRV8825_RS485.h"

namespace kai
{

_DRV8825_RS485::_DRV8825_RS485()
{
	m_pMB = NULL;
	m_iSlave = 1;
	m_iData = 0;
	m_dpr = 1;
	m_pA = NULL;

	m_ieReadStatus.init(50000);
}

_DRV8825_RS485::~_DRV8825_RS485()
{
}

bool _DRV8825_RS485::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iSlave", &m_iSlave);
	pK->v("iData", &m_iData);
	pK->v("dpr", &m_dpr);
	pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

	m_pA = &m_vAxis[0];

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_Modbus", &iName));
	m_pMB = (_Modbus*) (pK->getInst(iName));
	IF_Fl(!m_pMB, iName + " not found");

	return true;
}

bool _DRV8825_RS485::start(void)
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

void _DRV8825_RS485::update(void)
{
	while(check()<0)
		this->sleepTime(USEC_1SEC);

	reset();

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateMove();
		readStatus();

		this->autoFPSto();
	}
}

int _DRV8825_RS485::check(void)
{
	NULL__(m_pMB,-1);
	IF__(!m_pMB->bOpen(),-1);
	NULL__(m_pA,-1);

	return 0;
}

void _DRV8825_RS485::updateMove(void)
{
	IF_(check()<0);

	if(m_bMoving)
	{
		IF_(m_pA->getRawP() != m_pTargetMoving);
		m_bMoving = false;
	}

	m_pTargetMoving = m_pA->getRawPtarget();
	setDist(m_pTargetMoving);
	setSpeed();
	setAccel();
	run();
	m_bMoving = true;
}

bool _DRV8825_RS485::setDistPerRound(int32_t dpr)
{
	IF_F(check()<0);

	uint16_t pB[2];
	pB[0] = HIGH16(dpr);
	pB[1] = LOW16(dpr);
	IF_F(m_pMB->writeRegisters(m_iSlave, 4, 2, pB) != 2);

	return true;
}

bool _DRV8825_RS485::setDist(int32_t d)
{
	IF_F(check()<0);

	uint16_t pB[2];
	int32_t step = d - m_pA->getRawP();// m_pA->getRawPtarget() - m_pA->getRawP();
	int32_t ds = abs(step);
	pB[0] = HIGH16(ds);
	pB[1] = LOW16(ds);
	IF_F(m_pMB->writeRegisters(m_iSlave, 9, 2, pB) != 2);

	pB[0] = (step >= 0)?1:0;
	IF_F(m_pMB->writeRegisters(m_iSlave, 11, 1, pB) != 1);

	return true;
}

bool _DRV8825_RS485::setSpeed(void)
{
	IF_F(check()<0);

	uint16_t b = m_pA->getRawStarget();
	IF_F(m_pMB->writeRegisters(m_iSlave, 8, 1, &b) != 1);

	return true;
}

bool _DRV8825_RS485::setAccel(void)
{
	IF_F(check()<0);

	uint16_t b = m_pA->getRawAtarget();
	IF_F(m_pMB->writeRegisters(m_iSlave, 2, 1, &b) != 1);

	return true;
}

bool _DRV8825_RS485::setBrake(void)
{

	return true;
}

void _DRV8825_RS485::run(void)
{
	m_pMB->writeBit(m_iSlave, 7, true);
}

void _DRV8825_RS485::stop(void)
{
	m_pMB->writeBit(m_iSlave, 3, true);
}

void _DRV8825_RS485::resetPos(void)
{
	m_pMB->writeBit(m_iSlave, 10, true);
}

void _DRV8825_RS485::reset(void)
{
	setDistPerRound(m_dpr);
	resetPos();
//	setDist();
	setSpeed();
	setAccel();
	run();
}

void _DRV8825_RS485::readStatus(void)
{
	IF_(check()<0);
	IF_(!m_ieReadStatus.update(m_tStamp));

	uint16_t pB[2];
	int r = m_pMB->readRegisters(m_iSlave, 23, 1, pB);
	IF_(r != 1);

//	int p = MAKE32(pB[0], pB[1]);
	int16_t p = pB[0];
	m_pA->setRawP(p);
}

void _DRV8825_RS485::draw(void)
{
	this->_ActuatorBase::draw();
}

}
