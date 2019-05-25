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

	return true;


	// send a 0x08 query for connection check
//	uint8_t pB[256];
//	pB[0] = m_slaveAddr;
//	pB[1] = 0x08;
//	pB[2] = 0;
//	pB[3] = 0;
//	pB[4] = 0x12;
//	pB[5] = 0x34;
//	if (modbus_send_raw_request(m_pMb, pB, 6) < 8)
//	{
//		LOG_E("Query 0x08h error");
//		return false;
//	}
}

bool _Modbus::bOpen(void)
{
	NULL_F(m_pMb);

	return true;
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

modbus_t* _Modbus::getModbus(int iSlave)
{
	if(!m_pMb)return NULL;

	pthread_mutex_lock(&m_mutex);

	if (modbus_set_slave(m_pMb, iSlave) != 0)
	{
		pthread_mutex_unlock(&m_mutex);
		LOG_E("Error setting slave id:" + i2str(iSlave));
		return NULL;
	}

	return m_pMb;
}

void _Modbus::releaseModbus(void)
{
	IF_(!m_pMb);

    modbus_flush(m_pMb);
	pthread_mutex_unlock(&m_mutex);
}

bool _Modbus::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;

	return true;
}

}
