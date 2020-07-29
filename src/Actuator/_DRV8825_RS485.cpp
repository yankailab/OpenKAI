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

	m_vStepRange.init(0, 1e2);
	m_vSpeedRange.init(0, 500);
	m_vAccelRange.init(1, 300);
	m_vBrakeRange.init(1, 300);
	m_vCurrentRange.init(0, 1000);
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
	pK->v("vStepRange", &m_vStepRange);
	pK->v("vSpeedRange", &m_vSpeedRange);
	pK->v("vAccelRange", &m_vAccelRange);
	pK->v("vBrakeRange", &m_vBrakeRange);
	pK->v("vCurrentRange", &m_vCurrentRange);
	pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

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

	setDistPerRound(m_dpr);
	resetPos();
	setDist(m_vAxis[0].getPtarget());
	setSpeed(m_vAxis[0].getStarget());
	setAccel(m_vAxis[0].getAtarget());
	run();

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(m_bFeedback)
		{
			readStatus();
		}

		this->autoFPSto();
	}
}

int _DRV8825_RS485::check(void)
{
	NULL__(m_pMB,-1);
	IF__(!m_pMB->bOpen(),-1);

	return 0;
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

bool _DRV8825_RS485::setDist(float d)
{
	IF_F(check()<0);
	IF_F(d < -1.0 || d > 1.0);

	uint16_t pB[2];

	int32_t step = abs(d) * m_vStepRange.len() + m_vStepRange.x;
	pB[0] = HIGH16(step);
	pB[1] = LOW16(step);
	IF_F(m_pMB->writeRegisters(m_iSlave, 9, 2, pB) != 2);

	pB[0] = (d >= 0.0)?1:0;
	IF_F(m_pMB->writeRegisters(m_iSlave, 11, 1, pB) != 1);

	return true;
}

bool _DRV8825_RS485::setSpeed(float s)
{
	IF_F(check()<0);

	uint16_t b = constrain<float>(s, 0.0, 1.0) * m_vSpeedRange.len() + m_vSpeedRange.x;
	IF_F(m_pMB->writeRegisters(m_iSlave, 8, 1, &b) != 1);

	return true;
}

bool _DRV8825_RS485::setAccel(float a)
{
	IF_F(check()<0);

	uint16_t b = constrain<float>(a, 0.0, 1.0) * m_vAccelRange.len() + m_vAccelRange.x;
	IF_F(m_pMB->writeRegisters(m_iSlave, 2, 1, &b) != 1);

	return true;
}

bool _DRV8825_RS485::setBrake(float b)
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

void _DRV8825_RS485::readStatus(void)
{
	IF_(check()<0);
	IF_(!m_ieReadStatus.update(m_tStamp));

	uint16_t pB[2];
	int r = m_pMB->readRegisters(m_iSlave, 23, 1, pB);
	IF_(r != 1);

//	int p = MAKE32(pB[0], pB[1]);
	int16_t p = pB[0];
}

void _DRV8825_RS485::draw(void)
{
	this->_ActuatorBase::draw();
}

}
