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
	m_dMin = 0.1;
	m_dMax = 100.0;
	m_angleV = 0.0;
	m_angleH = 0.0;

	m_portName = "";
	m_baud = 115200;

	m_slaveAddr = 1;
	m_bUse0x41 = false;

	m_bReady = 0;
	m_nSegment = N_SEGMENT;
	m_nDetection = 0;
	m_lightSrcPwr = 0;
	m_timeStamp = 0;
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

	F_INFO(pK->v("minDist", &m_dMin));
	F_INFO(pK->v("maxDist", &m_dMax));

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
		m_pMb = NULL;
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
	buf[1] = 0x11;	// Request MODBUS_SERVER_ID

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
					LOG_I("Talking to a M16");
				}
				else if ((buf[2 + 150] == 7) && (buf[2 + 151] == 0))
				{
					LOG_I("Talking to an evaluation kit");
				}
				else if ((buf[2 + 50] == 10) && (buf[2 + 51] == 0))
				{
					LOG_I(
							"Talking to a LeddarOne (please use the LeddarOne sample)");
				}
				else
				{
					string str = "";

					for (int i = 0; i < nReceived; ++i)
					{
						str += (buf[i] >= 32) ? buf[i] : ' ';
					}

					LOG_I(str);
				}
			}
			else
			{
				LOG_I("Unexpected answer (bad address or function)");
			}
		}
		else
		{
			LOG_I("Unexpected answer");
		}
	}
	else
	{
		LOG_I("Error receiving data from the sensor:" << errno);
	}

	return true;
}

bool _LeddarVu::updateLidar(void)
{
	NULL_F(m_pMb);

	const uint32_t N_REGISTERS = 15 + N_SEGMENT;
	uint16_t reg[N_REGISTERS];

	int nRead = modbus_read_input_registers(m_pMb, 1, N_REGISTERS, reg);
	IF_F(nRead != N_REGISTERS);

	m_bReady = reg[0];
	IF_F(m_bReady == 0);

	m_nSegment = reg[1];
	m_nDetection = reg[10];
	m_lightSrcPwr = reg[11];
	m_timeStamp = reg[13] + (reg[14] << 16);

	const static double BASE_D = 1.0/100.0;
	const static double BASE_A = 1.0/64.0;

	int i;
	for (i = 0; i < N_SEGMENT; i++)
	{
		m_pSegment[i].dDistance = (double) reg[15 + i] * BASE_D;
//		m_pSegment[i].dAmplitude = (double) reg[15 + N_SEGMENT + i] * BASE_A;
//		m_pSegment[i].flags = reg[15 + 2*N_SEGMENT + i];
	}

	IF_T(!m_bLog);

	string log = "nSeg:" + i2str(m_nSegment) + " nDet:" + i2str(m_nDetection);
	for (i = 0; i < N_SEGMENT; i++)
	{
		log += " | " + f2str(m_pSegment[i].dDistance);
	}
	log += " |";
	LOG_I(log);

	return true;
}

DISTANCE_SENSOR_TYPE _LeddarVu::type(void)
{
	return dsLeddarVu;
}

vDouble2 _LeddarVu::range(void)
{
	vDouble2 r;
	r.x = m_dMin;
	r.y = m_dMax;
	return r;
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
