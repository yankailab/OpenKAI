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
	m_baud = 115200;
	m_slaveAddr = 1;
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
	KISSm(pK, baud);
	KISSm(pK, slaveAddr);

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


		this->autoFPSto();
	}
}

bool _OrientalMotor::open(void)
{
	m_pMb = modbus_new_rtu(m_port.c_str(), m_baud, 'E', 8, 1);
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
		LOG_E("Error setting slave id: "+i2str(m_slaveAddr) );
		m_slaveAddr = -1;
		return false;
	}

	// send a 0x08 query for connection check
	uint8_t buf[256];
	buf[0] = m_slaveAddr;
	buf[1] = 0x08;
	buf[2] = 0;
	buf[3] = 0;
	buf[4] = 0x12;
	buf[5] = 0x34;

	if (modbus_send_raw_request(m_pMb, buf, 6) < 8)
	{
		LOG_E("Query 0x08h error");
		return false;
	}

	const uint32_t N_REG = 10;
	uint16_t reg[N_REG];
	int nRead = modbus_read_registers(m_pMb, 0, N_REG, reg);
	IF_F(nRead != N_REG);
	IF_F(reg[0] == 0);

	LOG_I(i2str(nRead) + " registers read");

//
//	memset(buf, 0, sizeof buf);
//	int nReceived = modbus_receive_confirmation(m_pMb, buf);
//	IF_Fl(nReceived < 0, "Error receiving data from the sensor: " + i2str(errno));
//	LOG_I("Received " + i2str(nReceived) + " bytes from the sensor");

	return true;
}

//bool _OrientalMotor::updateLidar(void)
//{
//	NULL_F(m_pMb);
//
//	const uint32_t N_REGISTERS = 15 + m_nDiv*2;
//	uint16_t reg[N_REGISTERS];
//
//	int nRead = modbus_read_input_registers(m_pMb, 1, N_REGISTERS, reg);
//	IF_F(nRead != N_REGISTERS);
//	IF_F(reg[0] == 0);
//
//	return true;
//}

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

	C_MSG(msg);

	return true;
}

}
