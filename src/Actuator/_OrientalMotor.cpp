/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_OrientalMotor.h"

namespace kai
{

_OrientalMotor::_OrientalMotor()
{
	m_pMb = NULL;
	m_port = "";
	m_parity = 'E';
	m_baud = 115200;
	m_slaveAddr = 1;
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
	if (m_pMb)
	{
		modbus_close(m_pMb);
		modbus_free(m_pMb);
	}
}

bool _OrientalMotor::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, port);
	KISSm(pK, parity);
	KISSm(pK, baud);
	KISSm(pK, slaveAddr);
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
		if (!m_pMb)
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

//		sendCMD();
		readStatus();

		this->autoFPSto();
	}
}

bool _OrientalMotor::open(void)
{
	m_pMb = modbus_new_rtu(m_port.c_str(), m_baud, *m_parity.c_str(), 8, 1);
	if (m_pMb == nullptr)
	{
		m_pMb = NULL;
		LOG_E("Cannot create the libmodbus context");
		return false;
	}

	modbus_set_debug(m_pMb, false);

	if (modbus_connect(m_pMb) != 0)
	{
		LOG_E("Modbus connection failed");
		modbus_free(m_pMb);
		m_pMb = NULL;
		return false;
	}

	if (modbus_set_slave(m_pMb, m_slaveAddr) != 0)
	{
		LOG_E("Error setting slave id");
		m_slaveAddr = -1;
		return false;
	}

	// send a 0x08 query for connection check
	uint8_t pB[256];
	pB[0] = m_slaveAddr;
	pB[1] = 0x08;
	pB[2] = 0;
	pB[3] = 0;
	pB[4] = 0x12;
	pB[5] = 0x34;
	if (modbus_send_raw_request(m_pMb, pB, 6) < 8)
	{
		LOG_E("Query 0x08h error");
		return false;
	}

	return true;
}

int _OrientalMotor::check(void)
{
	NULL__(m_pMb,-1);

	return 0;
}

void _OrientalMotor::sendCMD(void)
{
	IF_(check()<0);
	IF_(m_tStampCmdSet <= m_tStampCmdSent);

	int32_t pB[9];
	pB[0] = m_iData;
	pB[1] = 1;
	pB[2] = m_tState.m_step;
	pB[3] = m_tState.m_speed;
	pB[4] = m_tState.m_accel;
	pB[5] = m_tState.m_brake;
	pB[6] = m_tState.m_current;
	pB[7] = 1;
	pB[8] = 0;

	modbus_write_registers(m_pMb, 88, 18, (uint16_t*)pB);	//88 -> 0058h
}

void _OrientalMotor::readStatus(void)
{
	IF_(check()<0);

	uint16_t pB[18];
	int nRead = modbus_read_registers(m_pMb, 204, 6, pB);	//88 -> 00CCh
	IF_(nRead != 6);
	LOG_I(i2str(nRead) + " registers read");

	m_cState.m_step = unpack_int32(&pB[0], false);
	m_cState.m_speed = unpack_int32(&pB[2], false);
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
