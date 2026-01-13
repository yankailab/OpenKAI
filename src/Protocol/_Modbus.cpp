#include "_Modbus.h"

namespace kai
{

	_Modbus::_Modbus()
	{
		pthread_mutex_init(&m_mutex, NULL);

		m_pMb = nullptr;
		m_bOpen = false;
	}

	_Modbus::~_Modbus()
	{
		pthread_mutex_destroy(&m_mutex);
		close();

		IF_(m_pMb == nullptr);
		modbus_free(m_pMb);
		m_pMb = nullptr;
	}

	bool _Modbus::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		m_bModbusDebug = j.value("bModbusDebug", false);
		m_nErrReconnect = j.value("nErrReconnect", 0);

		m_rtuPort = j.value("rtuPort", "");
		m_rtuParity = j.value("rtuParity", 'E');
		m_rtuBaud = j.value("rtuBaud", 115200);

		m_tcpAddr = j.value("tcpAddr", "");
		m_tcpPort = j.value("tcpPort", 0);

		m_tIntervalUsec = j.value("tIntervalUsec", 10000);
		m_tOutSec = j.value("tOutSec", 1);
		m_tOutUSec = j.value("tOutUSec", 0);

		m_type = j.value("type", "RTU");
		if (m_type == "RTU")
		{
			m_pMb = modbus_new_rtu(m_rtuPort.c_str(), m_rtuBaud, *m_rtuParity.c_str(), 8, 1);
		}
		else if (m_type == "TCP")
		{
			m_pMb = modbus_new_tcp(m_tcpAddr.c_str(), m_tcpPort);
		}
		else
		{
			LOG_E("Modbus type wrong (RTU/TCP): " + m_type);
			return false;
		}

		if (m_pMb == nullptr)
		{
			LOG_E("Cannot create the libmodbus context: " + string(modbus_strerror(errno)));
			return false;
		}

		modbus_set_debug(m_pMb, m_bModbusDebug);

		if (modbus_set_response_timeout(m_pMb, m_tOutSec, m_tOutUSec) != 0)
		{
			LOG_E("Modbus set response timeout failed: " + string(modbus_strerror(errno)));
			close();
			return false;
		}

		return true;
	}

	bool _Modbus::open(void)
	{
		if (modbus_connect(m_pMb) != 0)
		{
			LOG_E("Modbus connection failed: " + string(modbus_strerror(errno)));
			close();
			return false;
		}

		m_bOpen = true;
		m_iErr = 0;
		return true;
	}

	bool _Modbus::bOpen(void)
	{
		return m_bOpen;
	}

	void _Modbus::close(void)
	{
		m_bOpen = false;
		IF_(m_pMb == nullptr);

		modbus_close(m_pMb);
	}

	bool _Modbus::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _Modbus::check(void)
	{
		NULL_F(m_pMb);
		IF_F(!bOpen());

		return this->_ModuleBase::check();
	}

	void _Modbus::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!bOpen())
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			if (m_iErr > m_nErrReconnect)
			{
				pthread_mutex_lock(&m_mutex);
				close();
				pthread_mutex_unlock(&m_mutex);

				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			m_pT->autoFPS();
		}
	}

	// Function code: 2
	int _Modbus::readInputBits(int iSlave, int addr, int nB, uint8_t *pB)
	{
		IF__(!check(), -1);
		NULL__(pB, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_read_input_bits(m_pMb, addr, nB, pB);
			if (r == -1)
			{
				m_iErr++;
				LOG_I("Modbus_read_input_bits (2) failed: " + string(modbus_strerror(errno)));
			}
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 3
	int _Modbus::readRegisters(int iSlave, int addr, int nRegister, uint16_t *pB)
	{
		IF__(!check(), -1);
		NULL__(pB, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_read_registers(m_pMb, addr, nRegister, pB);
			if (r == -1)
			{
				m_iErr++;
				LOG_I("Modbus_read_registers (3) failed: " + string(modbus_strerror(errno)));
			}
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 5
	int _Modbus::writeBit(int iSlave, int addr, bool bStatus)
	{
		IF__(!check(), -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_write_bit(m_pMb, addr, bStatus);
			if (r == -1)
			{
				m_iErr++;
				LOG_I("Modbus_write_bit (5) failed: " + string(modbus_strerror(errno)));
			}
		}

		modbus_flush(m_pMb);
		usleep(m_tIntervalUsec);

		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 6
	int _Modbus::writeRegister(int iSlave, int addr, int v)
	{
		IF__(!check(), -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_write_register(m_pMb, addr, v);
			if (r == -1)
			{
				m_iErr++;
				LOG_I("Modbus_write_register (6) failed: " + string(modbus_strerror(errno)));
			}
		}

		modbus_flush(m_pMb);
		usleep(m_tIntervalUsec);

		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 10
	int _Modbus::writeRegisters(int iSlave, int addr, int nRegister, uint16_t *pB)
	{
		IF__(!check(), -1);
		NULL__(pB, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_write_registers(m_pMb, addr, nRegister, pB);
			if (r == -1)
			{
				m_iErr++;
				LOG_I("Modbus_write_registers (10) failed: " + string(modbus_strerror(errno)));
			}
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	int _Modbus::sendRawRequest(uint8_t *pB, int nB)
	{
		IF__(!check(), -1);
		NULL__(pB, -1);

		pthread_mutex_lock(&m_mutex);

		int r = modbus_send_raw_request(m_pMb, pB, nB);
		if (r == -1)
		{
			m_iErr++;
			LOG_I("Modbus_send_raw_request failed: " + string(modbus_strerror(errno)));
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	int _Modbus::sendRawRequest(int iSlave, uint8_t *pB, int nB)
	{
		IF__(!check(), -1);
		NULL__(pB, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_send_raw_request(m_pMb, pB, nB);
			if (r == -1)
			{
				m_iErr++;
				LOG_I("Modbus_send_raw_request failed: " + string(modbus_strerror(errno)));
			}
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

}
