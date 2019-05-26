#include "_Modbus.h"

namespace kai
{

_Modbus::_Modbus()
{
	pthread_mutex_init(&m_mutex, NULL);

	m_pMb = NULL;
	m_port = "";
	m_parity = 'E';
	m_baud = 115200;
	m_bOpen = false;
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

bool _Modbus::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK, port);
	KISSm(pK, parity);
	KISSm(pK, baud);


	return true;
}

bool _Modbus::open(void)
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

	m_bOpen = true;
	return true;
}

bool _Modbus::bOpen(void)
{
	return m_bOpen;
}

bool _Modbus::start(void)
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

void _Modbus::update(void)
{
	while (m_bThreadON)
	{
		if(!m_pMb)
		{
			if(!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		this->autoFPSto();
	}
}

int _Modbus::rawRequest(uint8_t* pB, int nB)
{
	IF__(!m_pMb,-1);
	NULL__(pB,-1);

	pthread_mutex_lock(&m_mutex);

	int r = modbus_send_raw_request(m_pMb, pB, nB);
    modbus_flush(m_pMb);

	pthread_mutex_unlock(&m_mutex);

	return r;
}

int _Modbus::writeRegisters(int iSlave, int addr, int nRegister, uint16_t* pB)
{
	IF__(!m_pMb,-1);
	NULL__(pB,-1);

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

int _Modbus::readRegisters(int iSlave, int addr, int nRegister, uint16_t* pB)
{
	IF__(!m_pMb,-1);
	NULL__(pB,-1);

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

bool _Modbus::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;

	return true;
}

}
