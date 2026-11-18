#include "_Modbus.h"

namespace kai
{

	_Modbus::_Modbus()
	{
	}

	_Modbus::~_Modbus()
	{
		stop();
		close();
	}

	bool _Modbus::loadConfig(void)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		// A reload must not delete a running reconnect worker.
		if (m_pT)
		{
			IF_F(!BASE::loadConfig());
		}
		else
		{
			IF_F(!_ModuleBase::loadConfig());
		}
		closeLocked();
		const json &j = *m_pJ;

		jKv(j, "bModbusDebug", m_bModbusDebug);
		jKv(j, "nErrReconnect", m_nErrReconnect);
		jKv(j, "rtuPort", m_rtuPort);
		jKv(j, "rtuParity", m_rtuParity);
		jKv(j, "rtuBaud", m_rtuBaud);
		jKv(j, "rtuDataBits", m_rtuDataBits);
		jKv(j, "rtuStopBits", m_rtuStopBits);
		jKv(j, "tcpAddr", m_tcpAddr);
		jKv(j, "tcpPort", m_tcpPort);
		jKv(j, "tIntervalUsec", m_tIntervalUsec);
		jKv(j, "tOutSec", m_tOutSec);
		jKv(j, "tOutUSec", m_tOutUSec);
		jKv(j, "modbusType", m_type);

		if (!validConfigLocked())
		{
			m_bConnectRequested = false;
			return false;
		}
		return true;
	}

	bool _Modbus::saveConfig(bool bExport)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["bModbusDebug"] = m_bModbusDebug;
		j["nErrReconnect"] = m_nErrReconnect;
		j["rtuPort"] = m_rtuPort;
		j["rtuParity"] = m_rtuParity;
		j["rtuBaud"] = m_rtuBaud;
		j["rtuDataBits"] = m_rtuDataBits;
		j["rtuStopBits"] = m_rtuStopBits;
		j["tcpAddr"] = m_tcpAddr;
		j["tcpPort"] = m_tcpPort;
		j["tIntervalUsec"] = m_tIntervalUsec;
		j["tOutSec"] = m_tOutSec;
		j["tOutUSec"] = m_tOutUSec;
		j["modbusType"] = m_type;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Modbus::validConfigLocked(void)
	{
		IF_Le_F(m_type != "RTU" && m_type != "TCP", "Modbus type must be RTU or TCP");
		IF_Le_F(m_nErrReconnect < 0, "Modbus error threshold must be nonnegative");
		IF_Le_F(m_tOutUSec >= 1000000 || (m_tOutSec == 0 && m_tOutUSec == 0),
			"Invalid Modbus response timeout");
		if (m_type == "RTU")
		{
			IF_Le_F(m_rtuPort.empty() || m_rtuBaud <= 0, "Invalid Modbus RTU port or baud rate");
			IF_Le_F(m_rtuParity != "N" && m_rtuParity != "E" && m_rtuParity != "O",
				"Modbus RTU parity must be N, E or O");
			IF_Le_F(m_rtuDataBits < 5 || m_rtuDataBits > 8, "Invalid Modbus RTU data bits");
			IF_Le_F(m_rtuStopBits != 1 && m_rtuStopBits != 2, "Invalid Modbus RTU stop bits");
		}
		else
		{
			IF_Le_F(m_tcpAddr.empty() || m_tcpPort < 1 || m_tcpPort > 65535,
				"Invalid Modbus TCP address or port");
		}
		return true;
	}

	bool _Modbus::open(void)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bConnectRequested = true;
		return openLocked();
	}

	bool _Modbus::openLocked(void)
	{
		IF__(m_bOpened, true);
		IF_F(!validConfigLocked());

		if (m_type == "RTU")
			m_pMb = modbus_new_rtu(m_rtuPort.c_str(), m_rtuBaud, m_rtuParity[0],
				m_rtuDataBits, m_rtuStopBits);
		else
			m_pMb = modbus_new_tcp(m_tcpAddr.c_str(), m_tcpPort);

		if (!m_pMb)
		{
			LOG_E("Cannot create the libmodbus context: " + string(modbus_strerror(errno)));
			return false;
		}
		modbus_set_debug(m_pMb, m_bModbusDebug);
		if (modbus_set_response_timeout(m_pMb, m_tOutSec, m_tOutUSec) != 0)
		{
			LOG_E("Modbus set response timeout failed: " + string(modbus_strerror(errno)));
			closeLocked();
			return false;
		}
		if (modbus_connect(m_pMb) != 0)
		{
			LOG_E("Modbus connection failed: " + string(modbus_strerror(errno)));
			closeLocked();
			return false;
		}
		m_bOpened = true;
		m_iErr = 0;
		return true;
	}

	bool _Modbus::bOpen(void)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_bOpened;
	}

	void _Modbus::close(void)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bConnectRequested = false;
		closeLocked();
	}

	void _Modbus::closeLocked(void)
	{
		if (m_pMb)
		{
			modbus_close(m_pMb);
			modbus_free(m_pMb);
			m_pMb = nullptr;
		}
		m_bOpened = false;
		m_iErr = 0;
	}

	bool _Modbus::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Modbus::stop(void)
	{
		// Do not hold m_mutex while waiting for a worker that also needs it.
		if (m_pT) m_pT->join();
	}

	bool _Modbus::check(void)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return checkLocked();
	}

	bool _Modbus::checkLocked(void)
	{
		return m_pMb && m_bOpened && _ModuleBase::check();
	}

	void _Modbus::update(void)
	{
		while (m_pT->bRun())
		{
			bool retry = false;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_bConnectRequested)
				{
					if (m_bOpened && m_iErr > m_nErrReconnect)
					{
						closeLocked();
						retry = true;
					}
					else if (!m_bOpened)
						retry = !openLocked();
				}
			}
			if (retry)
				m_pT->sleepT(NSEC_SEC);
			else
				m_pT->autoFPS();
		}
	}

	void _Modbus::setType(const string &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_type = value;
	}

	string _Modbus::getType(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_type;
	}

	void _Modbus::setRtuPort(const string &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_rtuPort = value;
	}

	string _Modbus::getRtuPort(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_rtuPort;
	}

	void _Modbus::setRtuParity(const string &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_rtuParity = value;
	}

	string _Modbus::getRtuParity(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_rtuParity;
	}

	void _Modbus::setRtuBaud(int value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_rtuBaud = value;
	}

	int _Modbus::getRtuBaud(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_rtuBaud;
	}

	void _Modbus::setRtuDataBits(int value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_rtuDataBits = value;
	}

	int _Modbus::getRtuDataBits(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_rtuDataBits;
	}

	void _Modbus::setRtuStopBits(int value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_rtuStopBits = value;
	}

	int _Modbus::getRtuStopBits(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_rtuStopBits;
	}

	void _Modbus::setTcpAddr(const string &value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tcpAddr = value;
	}

	string _Modbus::getTcpAddr(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_tcpAddr;
	}

	void _Modbus::setTcpPort(int value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tcpPort = value;
	}

	int _Modbus::getTcpPort(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_tcpPort;
	}

	void _Modbus::setModbusDebug(bool value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bModbusDebug = value;
	}

	bool _Modbus::getModbusDebug(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_bModbusDebug;
	}

	void _Modbus::setErrReconnect(int value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_nErrReconnect = value;
	}

	int _Modbus::getErrReconnect(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_nErrReconnect;
	}

	void _Modbus::setIntervalUsec(uint64_t value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tIntervalUsec = value;
	}

	uint64_t _Modbus::getIntervalUsec(void) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_tIntervalUsec;
	}

	void _Modbus::setResponseTimeout(uint32_t sec, uint32_t usec)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tOutSec = sec;
		m_tOutUSec = usec;
	}

	void _Modbus::getResponseTimeout(uint32_t &sec, uint32_t &usec) const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		sec = m_tOutSec;
		usec = m_tOutUSec;
	}

	// Function code: 2
	int _Modbus::readInputBits(int iSlave, int addr, int nB, uint8_t *pB)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF__(!checkLocked(), -1);
		NULL__(pB, -1);

		int r = -1;

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

		return r;
	}

	// Function code: 3
	int _Modbus::readRegisters(int iSlave, int addr, int nRegister, uint16_t *pB)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF__(!checkLocked(), -1);
		NULL__(pB, -1);

		int r = -1;

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

		return r;
	}

	// Function code: 5
	int _Modbus::writeBit(int iSlave, int addr, bool bStatus)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF__(!checkLocked(), -1);

		int r = -1;

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

		return r;
	}

	// Function code: 6
	int _Modbus::writeRegister(int iSlave, int addr, int v)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF__(!checkLocked(), -1);

		int r = -1;

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

		return r;
	}

	// Function code: 10
	int _Modbus::writeRegisters(int iSlave, int addr, int nRegister, uint16_t *pB)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF__(!checkLocked(), -1);
		NULL__(pB, -1);

		int r = -1;

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

		return r;
	}

	int _Modbus::sendRawRequest(uint8_t *pB, int nB)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF__(!checkLocked(), -1);
		NULL__(pB, -1);

		int r = modbus_send_raw_request(m_pMb, pB, nB);
		if (r == -1)
		{
			m_iErr++;
			LOG_I("Modbus_send_raw_request failed: " + string(modbus_strerror(errno)));
		}

		modbus_flush(m_pMb);

		return r;
	}

	int _Modbus::sendRawRequest(int iSlave, uint8_t *pB, int nB)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		IF__(!checkLocked(), -1);
		NULL__(pB, -1);

		int r = -1;

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

		return r;
	}

}
