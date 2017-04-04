/*
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */
#include "_LeddarVu.h"

namespace kai
{

_LeddarVu::_LeddarVu()
{
	m_pMb = NULL;
	m_nDiv = 0;
	m_dAngle = 0;
	m_minDist = 0.1;
	m_maxDist = 100.0;
	m_angleV = 0.0;
	m_angleH = 0.0;

	m_portName = "";
	m_baud = 115200;

	m_slaveAddr = -1;
	m_bUse0x41 = false;
}

_LeddarVu::~_LeddarVu()
{
	if (m_pMb)
	{
		modbus_close(m_pMb);
		modbus_free(m_pMb);
	}
}

bool _LeddarVu::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("minDist", &m_minDist));
	F_INFO(pK->v("maxDist", &m_maxDist));

	Kiss* pI;
	pI = pK->o("input");
	IF_F(pI->empty());
	F_INFO(pI->v("portName", &m_portName));
	F_INFO(pI->v("baud", &m_baud));
	F_INFO(pI->v("slaveAddr", &m_slaveAddr));
	F_INFO(pI->v("bUse0x41", &m_bUse0x41));

	return true;
}

bool _LeddarVu::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _LeddarVu::start(void)
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

void _LeddarVu::update(void)
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

		updateLidar();

		this->autoFPSto();
	}
}

bool _LeddarVu::open(void)
{
	m_pMb = modbus_new_rtu(m_portName.c_str(), m_baud, 'N', 8, 1);
	if (m_pMb == nullptr)
	{
		LOG_E("Unable to create the libmodbus context");
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

	// We send a MODBUS_SERVER_ID request to determine if we are talking to a Leddar sensor.
	uint8_t buf[256];
	buf[0] = 1;
	buf[1] = 0x11;	// MODBUS_SERVER_ID

	if (modbus_send_raw_request(m_pMb, buf, 2) < 2)
	{
		LOG_E("Error sending MODBUS_SERVER_ID request");
		return false;
	}



	memset(buf, 0, sizeof buf);
	int nReceived = modbus_receive_confirmation(m_pMb, buf);
	if (nReceived >= 0)
	{
		LOG_I("Received "<< nReceived <<" bytes from the sensor");

		if (nReceived >= 155)
		{
			if ((buf[0] == 1) && (buf[1] == 0x11))
			{
				if ((buf[2 + 150] == 9) && (buf[2 + 151] == 0))
				{
					printf("Talking to a M16\n");
				}
				else if ((buf[2 + 150] == 7) && (buf[2 + 151] == 0))
				{
					printf("Talking to an evaluation kit\n");
				}
				else if ((buf[2 + 50] == 10) && (buf[2 + 51] == 0))
				{
					printf("Talking to a LeddarOne (please use the LeddarOne sample)\n");
				}
				else
				{
					for (int i = 0; i < nReceived; ++i)
					{
						printf("  Byte %i = %i ('%c')\n", i, (int)buf[i], (buf[i] >= 32) ? buf[i] : ' ');
					}
				}
			}
			else
				printf("Unexpected answer (bad address or function)\n");
		}
		else
			printf("Unexpected answer\n");
	}
	else
		printf("Error receiving data from the sensor (%i)\n", errno);

	return true;
}

bool _LeddarVu::updateLidar(void)
{
	// Shows the last reading.
	SDetection tabDetections[LEDDAR_MAX_DETECTIONS];
	uint32_t uDetectionCount;
	uint32_t uTimestamp;

	if (ReadDetectionsReg(m_pMb, tabDetections, uDetectionCount, uTimestamp))
	{
		printf("Timestamp: %u\n", uTimestamp);
		printf("Got %u detections\n", uDetectionCount);

		for (uint32_t i = 0; i < uDetectionCount; ++i)
		{
			printf("#%-2i: Segment %-2i   Distance = %.3lfm    Amplitude = %.1lf\n", i + 1, (uint32_t)tabDetections[i].channel,
				   tabDetections[i].dDistance, tabDetections[i].dAmplitude);
		}
	}
	else
		printf("Error reading detections, errno=%i\n", errno);


	return true;
}

double _LeddarVu::minDist(void)
{
	return m_minDist;
}

double _LeddarVu::maxDist(void)
{
	return m_maxDist;
}

bool _LeddarVu::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	pWin->tabNext();
	msg = "";
	pWin->addMsg(&msg);

	pWin->tabPrev();
	IF_T(m_nDiv <= 0);

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	for (int i = 0; i < m_nDiv; i++)
	{
	}

	return true;
}

bool _LeddarVu::ReadDetectionsReg(modbus_t* mb, SDetection tabDetections[LEDDAR_MAX_DETECTIONS],
						      uint32_t& nbrDetections, uint32_t& uTimestamp)
{
	// This version of the ReadDetections function uses the standard Modbus registers.
	nbrDetections = 0;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Read 32+2 registers from address 14
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	const uint32_t NBR_REGISTERS = 32+2;
	uint16_t tabRegValues[NBR_REGISTERS];
	int numRead = modbus_read_input_registers(mb, 14, NBR_REGISTERS, tabRegValues);
	if (numRead == NBR_REGISTERS)
	{
		uTimestamp = tabRegValues[0] + (tabRegValues[1] << 16);

		for (uint32_t i = 0; i < 16; ++i)
		{
			tabDetections[i].channel = i;
			tabDetections[i].dDistance = (double)tabRegValues[2 + i] / 100.0;
			tabDetections[i].dAmplitude = (double)tabRegValues[18 + i] / 64.0;
			tabDetections[i].flags = 0;                // unknown with the registers method
		}

		nbrDetections = 16;                    // limit to the first detection (one per segment)

		//printf("Read %i registers:\n", numRead);
		//for (int i = 0; i < numRead; ++i)
		//{
		//	printf("    Register %-2i = %u\n", i, (unsigned int)tabRegValues[i]);
		//}
	}
	else
	{
		//printf("Error reading registers: numRead=%i, errno=%i\n", numRead, errno);
		return false;
	}

	return true;    // success
}


// ***********************************************************************************************
// IMPLEMENTATION
// ***********************************************************************************************
/*
bool _LeddarVu::ReadDetectionsFast(modbus_t* mb, SDetection tabDetections[LEDDAR_MAX_DETECTIONS],
						       uint32_t& nbrDetections, uint32_t& uTimestamp)
{
	// This version of the ReadDetections function uses the 0x41 function code.
	// It reduces the overhead and is the recommended way of reading measurements.

	// Read 5 holding registers from address 1
	uint8_t rawReq[] = { (uint8_t)m_slaveAddr, 0x41 };
	uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];

	if (modbus_send_raw_request(mb, rawReq, ARRAYSIZE(rawReq)) < 0)
	{
		printf("Error sending command 0x41 (errno=%i)\n", errno);
		return false;
	}

	// We had to add support for 0x41 in 'compute_data_length_after_meta()' in "modbus.c".
	int iResponseLength = modbus_receive_confirmation(mb, rsp);
	if (iResponseLength < 3)
	{
		printf("Error receiving response for command 0x41 (length=%i, errno=%i)\n", iResponseLength, errno);
		return false;
	}

	nbrDetections = rsp[2];

	if (iResponseLength >= 3 + (signed)nbrDetections*5)
	{
		for (uint32_t u = 0; (u < nbrDetections) && (u < LEDDAR_MAX_DETECTIONS); ++u)
		{
			uint8_t* pDetection = rsp + 3 + 5*u;
			tabDetections[u].dDistance = (pDetection[0] + (pDetection[1] << 8)) / 100.0;
			tabDetections[u].dAmplitude = (pDetection[2] + (pDetection[3] << 8)) / 64.0;
			tabDetections[u].channel = pDetection[4] >> 4;
			tabDetections[u].flags = pDetection[4] & 0x0F;
		}

		if (iResponseLength >= 3 + (signed)nbrDetections*5 + 6)
		{
			uint8_t* pTrailer = rsp + 3 + 5*nbrDetections;
			uTimestamp = pTrailer[0] + (pTrailer[1] << 8) + (pTrailer[2] << 16) + (pTrailer[3] << 24);
		}
	}
	else
	{
		printf("Not enough data received (length=%i)\n", iResponseLength);
		return false;
	}

	return true;        // success
}
*/

}
