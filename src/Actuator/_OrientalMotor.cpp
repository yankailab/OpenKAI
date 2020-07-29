/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_OrientalMotor.h"

namespace kai
{

_OrientalMotor::_OrientalMotor()
{
	m_pMB = NULL;
	m_iSlave = 1;
	m_iData = 0;

	m_vStepRange.init(-1e5, 1e5);
	m_vSpeedRange.init(-4e6, 4e6);
	m_vAccelRange.init(1, 1e9);
	m_vBrakeRange.init(1, 1e9);
	m_vCurrentRange.init(0, 1000);

	m_cState.init();
	m_tState.init();

	m_ieCheckAlarm.init(100000);
	m_ieSendCMD.init(50000);
	m_ieReadStatus.init(50000);
}

_OrientalMotor::~_OrientalMotor()
{
}

bool _OrientalMotor::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iSlave",&m_iSlave);
	pK->v("iData",&m_iData);
	pK->v("vStepRange", &m_vStepRange);
	pK->v("vSpeedRange", &m_vSpeedRange);
	pK->v("vAccelRange", &m_vAccelRange);
	pK->v("vBrakeRange", &m_vBrakeRange);
	pK->v("vCurrentRange", &m_vCurrentRange);

	pK->v("tIntCheckAlarm", &m_ieCheckAlarm.m_tInterval);
	pK->v("tIntSendCMD", &m_ieSendCMD.m_tInterval);
	pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

	ACTUATOR_AXIS* pA = &m_vAxis[0];
	pA->m_vRawPrange.init(m_vStepRange.x, m_vStepRange.y);
	pA->m_vRawSrange.init(m_vSpeedRange.x, m_vSpeedRange.y);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_Modbus", &iName));
	m_pMB = (_Modbus*) (pK->getInst(iName));
	IF_Fl(!m_pMB, iName + " not found");

	return true;
}

bool _OrientalMotor::start(void)
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

void _OrientalMotor::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(m_bFeedback)
		{
			checkAlarm();
			readStatus();
		}
		sendCMD();

		this->autoFPSto();
	}
}

int _OrientalMotor::check(void)
{
	NULL__(m_pMB,-1);
	IF__(!m_pMB->bOpen(),-1);

	return 0;
}

void _OrientalMotor::checkAlarm(void)
{
	IF_(check()<0);
	IF_(!m_ieCheckAlarm.update(m_tStamp));

	uint16_t pB[2];
	pB[0] = 1<<7;
	pB[1] = 0;
	m_pMB->writeRegisters(m_iSlave, 125, 1, pB);
}

void _OrientalMotor::sendCMD(void)
{
	IF_(check()<0);
	IF_(!m_ieSendCMD.update(m_tStamp));

	ACTUATOR_AXIS* pA = &m_vAxis[0];
	IF_(pA->getPtarget() < 0.0);

	//update normalized value to actual unit
	m_tState.m_step = pA->getPtarget() * m_vStepRange.len() + m_vStepRange.x;
	m_tState.m_step = constrain(m_tState.m_step, m_vStepRange.x, m_vStepRange.y);
	m_tState.m_speed = pA->getStarget() * m_vSpeedRange.len() + m_vSpeedRange.x;
	m_tState.m_speed = constrain(m_tState.m_speed, m_vSpeedRange.x, m_vSpeedRange.y);

	//create the command
	uint16_t pB[18];
	//88
	pB[0] = 0;
	pB[1] = m_iData;
	pB[2] = 0;
	pB[3] = 1;
	//92
	pB[4] = HIGH16(m_tState.m_step);
	pB[5] = LOW16(m_tState.m_step);
	pB[6] = HIGH16(m_tState.m_speed);
	pB[7] = LOW16(m_tState.m_speed);

	//96
	pB[8] = HIGH16(m_tState.m_accel);
	pB[9] = LOW16(m_tState.m_accel);
	pB[10] = HIGH16(m_tState.m_brake);
	pB[11] = LOW16(m_tState.m_brake);
	pB[12] = HIGH16(m_tState.m_current);
	pB[13] = LOW16(m_tState.m_current);
	pB[14] = 0;
	pB[15] = 1;
	pB[16] = 0;
	pB[17] = 0;

	if(m_pMB->writeRegisters(m_iSlave, 88, 18, pB) != 18)
	{
		m_ieSendCMD.reset();
	}
}

void _OrientalMotor::readStatus(void)
{
	IF_(check()<0);
	IF_(!m_ieReadStatus.update(m_tStamp));

	uint16_t pB[18];
	int nR = 6;
	int r = m_pMB->readRegisters(m_iSlave, 204, nR, pB);
	IF_(r != 6);

	m_cState.m_step = MAKE32(pB[0], pB[1]);
	m_cState.m_speed = MAKE32(pB[4], pB[5]);

	//update actual unit to normalized value
	ACTUATOR_AXIS* pA = &m_vAxis[0];
	IF_(pA->getPtarget() < 0.0);

	pA->setRawP(m_cState.m_step);
	pA->setRawS(m_cState.m_speed);

	LOG_I("step: "+i2str(m_cState.m_step) +
			", speed: " + i2str(m_cState.m_speed));
}

void _OrientalMotor::draw(void)
{
	this->_ActuatorBase::draw();

	addMsg("-- Current state --",1);
	addMsg("step: " + i2str(m_cState.m_step),1);
	addMsg("speed: " + i2str(m_cState.m_speed),1);

	addMsg("-- Target state --",1);
	addMsg("step: " + i2str(m_tState.m_step),1);
	addMsg("speed: " + i2str(m_tState.m_speed),1);
}

}
