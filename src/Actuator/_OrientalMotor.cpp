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

	m_vStepRange.x = -1e5;
	m_vStepRange.y = 1e5;
	m_vSpeedRange.x = -4e6;
	m_vSpeedRange.y = 4e6;
	m_vAccelRange.x = 1;
	m_vAccelRange.y = 1e9;
	m_vBrakeRange.x = 1;
	m_vBrakeRange.y = 1e9;
	m_vCurrentRange.x = 0;
	m_vCurrentRange.y = 1000;

	m_cState.init();
	m_tState.init();
}

_OrientalMotor::~_OrientalMotor()
{
}

bool _OrientalMotor::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, iSlave);
	KISSm(pK, iData);

	pK->v("stepFrom", &m_vStepRange.x);
	pK->v("stepTo", &m_vStepRange.y);
	pK->v("speedFrom", &m_vSpeedRange.x);
	pK->v("speedTo", &m_vSpeedRange.y);
	pK->v("accelFrom", &m_vAccelRange.x);
	pK->v("accelTo", &m_vAccelRange.y);
	pK->v("brakeFrom", &m_vBrakeRange.x);
	pK->v("brakeTo", &m_vBrakeRange.y);
	pK->v("currentFrom", &m_vCurrentRange.x);
	pK->v("currentTo", &m_vCurrentRange.y);

	pK->v("targetStep", &m_tState.m_step);
	pK->v("targetSpeed", &m_tState.m_speed);



	string iName;
	iName = "";
	F_ERROR_F(pK->v("Modbus", &iName));
	m_pMB = (_Modbus*) (pK->root()->getChildInst(iName));
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

		sendCMD();
		readStatus();

		this->autoFPSto();
	}
}

int _OrientalMotor::check(void)
{
	NULL__(m_pMB,-1);
	IF__(!m_pMB->bOpen(),-1);

	return 0;
}

void _OrientalMotor::sendCMD(void)
{
	IF_(check()<0);
//	IF_(m_tStampCmdSet <= m_tStampCmdSent);

//	int32_t pB[9];
//	pB[0] = m_iData;
//	pB[1] = 1;
//	pB[2] = m_tState.m_step;
//	pB[3] = m_tState.m_speed;
//	pB[4] = m_tState.m_accel;
//	pB[5] = m_tState.m_brake;
//	pB[6] = m_tState.m_current;
//	pB[7] = 1;
//	pB[8] = 0;

	uint16_t pB[18];
	int result;

	//88
	pB[0] = 0;		//7 8
	pB[1] = m_iData;//9 10
	pB[2] = 0;		//11 12
	pB[3] = 1;		//13 14

	//92
	pB[4] = (m_tState.m_step >> 16) & 0x0000ffff;	//15 16
	pB[5] = m_tState.m_step & 0x0000ffff;			//17 18
	pB[6] = (m_tState.m_speed >> 16) & 0x0000ffff;	//19 20
	pB[7] = m_tState.m_speed & 0x0000ffff;			//21 22

	//96
	pB[8] = (m_tState.m_accel >> 16) & 0x0000ffff;
	pB[9] = m_tState.m_accel & 0x0000ffff;
	pB[10] = (m_tState.m_brake >> 16) & 0x0000ffff;
	pB[11] = m_tState.m_brake & 0x0000ffff;
	pB[12] = (m_tState.m_current >> 16) & 0x0000ffff;
	pB[13] = m_tState.m_current & 0x0000ffff;
	pB[14] = 0;
	pB[15] = 1;
	pB[16] = 0;
	pB[17] = 0;

	modbus_t* pMB = m_pMB->getModbus(m_iSlave);
	NULL_(pMB);

	result = modbus_write_registers(pMB, 88, 18, pB);	//88 -> 0058h

	m_pMB->releaseModbus();

	LOG_I("write result: " + i2str(result));
}

void _OrientalMotor::readStatus(void)
{
	IF_(check()<0);

	uint16_t pB[18];

	modbus_t* pMB = m_pMB->getModbus(m_iSlave);
	NULL_(pMB);

	int nRead = modbus_read_registers(pMB, 204, 6, pB);	//88 -> 00CCh

	m_pMB->releaseModbus();

	IF_(nRead != 6);
//	m_cState.m_step = unpack_int32(&pB[0], false);
//	m_cState.m_speed = unpack_int32(&pB[2], false);

	m_cState.m_step = 0;
	m_cState.m_step |= pB[0];
	m_cState.m_step <<= 16;
	m_cState.m_step |= pB[1];

	LOG_I("step: "+i2str(m_cState.m_step));
//	LOG_I("speed: "+i2str(m_cState.m_speed));

}

bool _OrientalMotor::draw(void)
{
	IF_F(!this->_ActuatorBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	return true;
}

bool _OrientalMotor::console(int& iY)
{
	IF_F(!this->_ActuatorBase::console(iY));
	string msg;

	C_MSG("-- Current state --");
	C_MSG("step: " + i2str(m_cState.m_step));
	C_MSG("speed: " + i2str(m_cState.m_speed));

	C_MSG("-- Target state --");
	C_MSG("step: " + i2str(m_tState.m_step));
	C_MSG("speed: " + i2str(m_tState.m_speed));

	return true;
}

}
