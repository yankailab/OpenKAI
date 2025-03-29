#include "_Modbus.h"

namespace kai
{

	_Modbus::_Modbus()
	{
		pthread_mutex_init(&m_mutex, NULL);

		m_pMb = NULL;
		m_type = "RTU";

		m_rtuPort = "";
		m_rtuParity = 'E';
		m_rtuBaud = 115200;

		m_tcpAddr = "";
		m_tcpPort = 0;

		m_bOpen = false;
		m_tIntervalUsec = 10000;
		m_tOutSec = 1;
		m_tOutUSec = 0;
	}

	_Modbus::~_Modbus()
	{
		pthread_mutex_destroy(&m_mutex);
		if (m_pMb)
		{
			modbus_close(m_pMb);
			modbus_free(m_pMb);
		}
	}

	int _Modbus::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("type", &m_type);
		if (m_type != "RTU" && m_type != "TCP")
		{
			LOG_E("Modbus type wrong (RTU/TCP): " + m_type);
			return false;
		}

		pK->v("rtuPort", &m_rtuPort);
		pK->v("rtuParity", &m_rtuParity);
		pK->v("rtuBaud", &m_rtuBaud);

		pK->v("tcpAddr", &m_tcpAddr);
		pK->v("tcpPort", &m_tcpPort);

		pK->v("tIntervalUsec", &m_tIntervalUsec);
		pK->v("tOutSec", &m_tOutSec);
		pK->v("tOutUSec", &m_tOutUSec);

		return OK_OK;
	}

	bool _Modbus::open(void)
	{
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
			return false;
		}

		if (m_pMb == nullptr)
		{
			m_pMb = NULL;
			LOG_E("Cannot create the libmodbus context");
			return false;
		}

		modbus_set_debug(m_pMb, false);
		IF_F(modbus_set_response_timeout(m_pMb, m_tOutSec, m_tOutUSec) != 0);

		if (modbus_connect(m_pMb) != 0)
		{
			LOG_E("Modbus connection failed");
			modbus_free(m_pMb);
			m_pMb = NULL;
			return false;
		}

		m_bOpen = true;
		return true;
	}

	bool _Modbus::bOpen(void)
	{
		return m_bOpen;
	}

	int _Modbus::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Modbus::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_pMb)
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();
		}
	}

	int _Modbus::rawRequest(uint8_t *pB, int nB)
	{
		IF__(!m_pMb, -1);
		NULL__(pB, -1);

		pthread_mutex_lock(&m_mutex);

		int r = modbus_send_raw_request(m_pMb, pB, nB);
		modbus_flush(m_pMb);

		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 2
	int _Modbus::readInputBits(int iSlave, int addr, int nB, uint8_t *pB)
	{
		IF__(!m_pMb, -1);
		NULL__(pB, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_read_input_bits(m_pMb, addr, nB, pB);
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 3
	int _Modbus::readRegisters(int iSlave, int addr, int nRegister, uint16_t *pB)
	{
		IF__(!m_pMb, -1);
		NULL__(pB, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_read_registers(m_pMb, addr, nRegister, pB);
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 5
	int _Modbus::writeBit(int iSlave, int addr, bool bStatus)
	{
		IF__(!m_pMb, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_write_bit(m_pMb, addr, bStatus);
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 6
	int _Modbus::writeRegister(int iSlave, int addr, int v)
	{
		IF__(!m_pMb, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_write_register(m_pMb, addr, v);
		}

		modbus_flush(m_pMb);

		usleep(m_tIntervalUsec);

		pthread_mutex_unlock(&m_mutex);

		return r;
	}

	// Function code: 10
	int _Modbus::writeRegisters(int iSlave, int addr, int nRegister, uint16_t *pB)
	{
		IF__(!m_pMb, -1);
		NULL__(pB, -1);

		int r = -1;

		pthread_mutex_lock(&m_mutex);

		if (modbus_set_slave(m_pMb, iSlave) == 0)
		{
			r = modbus_write_registers(m_pMb, addr, nRegister, pB);
		}

		modbus_flush(m_pMb);
		pthread_mutex_unlock(&m_mutex);

		return r;
	}

}
