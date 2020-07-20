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
	setDist(m_vNormTargetPos.x);
	setSpeed(m_vNormTargetSpeed.x);
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

	uint16_t b = constrain<float>(s, 0.0, 1.0) * m_vSpeedRange.len() + m_vStepRange.x;
	IF_F(m_pMB->writeRegisters(m_iSlave, 8, 1, &b) != 1);

	return true;
}

bool _DRV8825_RS485::setAccel(float a)
{

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

void _DRV8825_RS485::readStatus(void)
{
	IF_(check()<0);
	IF_(!m_ieReadStatus.update(m_tStamp));

//	uint16_t pB[18];
//	int nR = 6;
//	int r = m_pMB->readRegisters(m_iSlave, 204, nR, pB);
//	IF_(r != 6);
//
//	m_cState.m_step = MAKE32(pB[0], pB[1]);
//	m_cState.m_speed = MAKE32(pB[4], pB[5]);
//
//	//update actual unit to normalized value
//	m_vNormPos.x = (float)(m_cState.m_step - m_vStepRange.x) / (float)m_vStepRange.len();
//	m_vNormSpeed.x = (float)(m_cState.m_speed - m_vSpeedRange.x) / (float)m_vSpeedRange.len();
//
//	LOG_I("step: "+i2str(m_cState.m_step) +
//			", speed: " + i2str(m_cState.m_speed));
}

void _DRV8825_RS485::draw(void)
{
	this->_ActuatorBase::draw();

//	addMsg("-- Current state --",1);
//	addMsg("step: " + i2str(m_cState.m_step),1);
//	addMsg("speed: " + i2str(m_cState.m_speed),1);
//
//	addMsg("-- Target state --",1);
//	addMsg("step: " + i2str(m_tState.m_step),1);
//	addMsg("speed: " + i2str(m_tState.m_speed),1);
}

}
