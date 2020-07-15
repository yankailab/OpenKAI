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
	m_port = "";
	m_baud = 115200;
	m_slaveAddr = 1;
	m_bUse0x41 = false;
	m_showOriginOffsetX = 0.5;
	m_showOriginOffsetY = 0.5;

	m_nDetection = 0;
	m_lightSrcPwr = 0;

	m_nAccumulationsExpo = 5;
	m_nOversamplingsExpo = 1;
	m_lightSrcPwr = 100;
	m_nPoint = 18;
	m_bAutoLightSrcPwr = false;
	m_bDemergeObj = true;
	m_bStaticNoiseRemoval = true;
	m_bPrecision = true;
	m_bSaturationCompensation = true;
	m_bOvershootManagement = true;
	m_oprMode = 1;

	m_nDiv = 8;
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
	IF_F(!this->_DistSensorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("port",&m_port);
	pK->v("baud",&m_baud);
	pK->v("slaveAddr",&m_slaveAddr);
	pK->v("bUse0x41",&m_bUse0x41);
	pK->v("showOriginOffsetX",&m_showOriginOffsetX);
	pK->v("showOriginOffsetY",&m_showOriginOffsetY);

	pK->v("nAccumulationsExpo",&m_nAccumulationsExpo);
	pK->v("nOversamplingsExpo",&m_nOversamplingsExpo);
	pK->v("nPoint",&m_nPoint);
	pK->v("lightSrcPwr",&m_lightSrcPwr);
	pK->v("bAutoLightSrcPwr",&m_bAutoLightSrcPwr);
	pK->v("bDemergeObj",&m_bDemergeObj);
	pK->v("bStaticNoiseRemoval",&m_bStaticNoiseRemoval);
	pK->v("bPrecicion",&m_bPrecision);
	pK->v("bSaturationCompensation",&m_bSaturationCompensation);
	pK->v("bOvershootManagement",&m_bOvershootManagement);
	pK->v("oprMode",&m_oprMode);

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

		if (m_bUse0x41)
		{
			updateLidarFast();
		}
		else
		{
			updateLidar();
		}

		this->autoFPSto();
	}
}

bool _LeddarVu::open(void)
{
	m_pMb = modbus_new_rtu(m_port.c_str(), m_baud, 'N', 8, 1);
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
	IF_Fl(nReceived < 0, "Error receiving data from the sensor: " + i2str(errno));
	LOG_I("Received " + i2str(nReceived) + " bytes from the sensor");

	IF_Fl(nReceived < 155, "Unexpected answer");
	IF_Fl(!((buf[0] == 1) && (buf[1] == 0x11)), "Bad address or function");

	if ((buf[2 + 150] == 9) && (buf[2 + 151] == 0))
	{
		LOG_I("LeddarTech M16");
	}
	else if ((buf[2 + 150] == 7) && (buf[2 + 151] == 0))
	{
		LOG_I("LeddarTech evaluation kit");
	}
	else if ((buf[2 + 50] == 10) && (buf[2 + 51] == 0))
	{
		LOG_I("LeddarOne (please use the LeddarOne sample)");
	}
	else
	{
		string str = "";
		for (int i = 0; i < nReceived; i++)
			str += (buf[i] >= 32) ? buf[i] : ' ';

		string::size_type iSensor = str.find("Sensor");
		if(iSensor != string::npos)
		{
			str = str.substr(iSensor,20);
		}

		LOG_I(str);
	}

	modbus_write_register(m_pMb, 0, m_nAccumulationsExpo);
	modbus_write_register(m_pMb, 1, m_nOversamplingsExpo);
	modbus_write_register(m_pMb, 2, m_nPoint);
	modbus_write_register(m_pMb, 5, m_lightSrcPwr);
	modbus_write_register(m_pMb, 10, m_oprMode);

	uint16_t acOpt = 0;
	if(m_bAutoLightSrcPwr)acOpt |= 1;
	if(m_bDemergeObj)acOpt |= 1 << 1;
	if(m_bStaticNoiseRemoval)acOpt |= 1 << 2;
	if(m_bPrecision)acOpt |= 1 << 3;
	if(m_bSaturationCompensation)acOpt |= 1 << 4;
	if(m_bOvershootManagement)acOpt |= 1 << 5;
	modbus_write_register(m_pMb, 6, acOpt);

	m_bReady = 1;

	return true;
}

bool _LeddarVu::updateLidar(void)
{
	NULL_F(m_pMb);

	const uint32_t N_REGISTERS = 15 + m_nDiv*2;
	uint16_t reg[N_REGISTERS];

	int nRead = modbus_read_input_registers(m_pMb, 1, N_REGISTERS, reg);
	IF_F(nRead != N_REGISTERS);
	IF_F(reg[0] == 0);

	m_nDiv = reg[1];
	m_nDetection = reg[10];
	m_lightSrcPwr = reg[11];
	m_tStamp = reg[13] + (reg[14] << 16);

	const static float BASE_D = 1.0 / 100.0;
	const static float BASE_A = 1.0 / 64.0;

	int i;
	for (i = 0; i < m_nDiv; i++)
	{
		this->input(i*m_dDeg,
					(float) reg[15 + i] * BASE_D,
					(float) reg[15 + m_nDiv + i] * BASE_A);
//		m_pSegment[i].dDistance = (float) reg[15 + i] * BASE_D;
//		m_pSegment[i].dAmplitude = (float) reg[15 + N_SEGMENT + i] * BASE_A;
//		m_pSegment[i].flags = reg[15 + 2 * N_SEGMENT + i];
	}

	return true;
}

bool _LeddarVu::updateLidarFast(void)
{
	NULL_F(m_pMb);

	// This version of the ReadDetections function uses the 0x41 function code.
	// It reduces the overhead and is the recommended way of reading measurements.

	// Read 5 holding registers from address 1
	uint8_t rawReq[] =
	{ (uint8_t) m_slaveAddr, 0x41 };
	uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];

//	if (modbus_send_raw_request(m_pMb, rawReq, ARRAYSIZE(rawReq)) < 0)
	if (modbus_send_raw_request(m_pMb, rawReq, 2) < 0)
	{
		LOG_E("Error sending command 0x41, errno = " + i2str(errno));
		return false;
	}

	// We had to add support for 0x41 in 'compute_data_length_after_meta()' in "modbus.c".
	int iResponseLength = modbus_receive_confirmation(m_pMb, rsp);
	if (iResponseLength < 3)
	{
		LOG_E("Error receiving response for command 0x41, length = " + i2str(iResponseLength) + "; errno = " + i2str(errno));
		return false;
	}

	const static float BASE_D = 1.0 / 100.0;
	const static float BASE_A = 1.0 / 64.0;
	m_nDetection = rsp[2];

	if (iResponseLength >= 3 + (signed) m_nDetection * 5)
	{
		for (uint32_t u = 0; (u < m_nDetection) && (u < LEDDAR_MAX_DETECTIONS); u++)
		{
			uint8_t* pDetection = rsp + 3 + 5 * u;
//			m_pSegment[u].dDistance = (pDetection[0] + (pDetection[1] << 8)) * BASE_D;
//			m_pSegment[u].dAmplitude = (pDetection[2] + (pDetection[3] << 8)) * BASE_A;

//			tabDetections[u].channel = pDetection[4] >> 4;
//			tabDetections[u].flags = pDetection[4] & 0x0F;
		}

		if (iResponseLength >= 3 + (signed) m_nDetection * 5 + 6)
		{
			uint8_t* pTrailer = rsp + 3 + 5 * m_nDetection;
//			uTimestamp = pTrailer[0] + (pTrailer[1] << 8) + (pTrailer[2] << 16) + (pTrailer[3] << 24);
		}
	}
	else
	{
		printf("Not enough data received (length=%i)\n", iResponseLength);
		return false;
	}

	return true;
}

DIST_SENSOR_TYPE _LeddarVu::type(void)
{
	return ds_LeddarVu;
}

void _LeddarVu::draw(void)
{
	this->_ThreadBase::draw();

	string msg;
	msg += "nDiv=" + i2str(m_nDiv);
	msg += ", nDet=" + i2str(m_nDetection);
	msg += ", lightSrcPwr=" + i2str(m_lightSrcPwr);
	msg += ", tStamp=" + i2str(m_tStamp);
	addMsg(msg);

	int i;
	msg = "";
	for (i=0; i<m_nDiv; i++)
	{
		DIST_SENSOR_DIV* pD = &m_pDiv[i];
		msg += " | " + f2str(pD->d()) + " (" + f2str(pD->a()) + ")";
	}
	msg += " |";
	addMsg(msg);

#ifdef USE_OPENCV
	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	Point pCenter(pMat->cols * m_showOriginOffsetX, pMat->rows * m_showOriginOffsetY);
	Scalar col = Scalar(0, 255, 0);
	Scalar colD = Scalar(0, 0, 255);
	float rMax = m_rMax * m_showScale;

	for(i=0; i<m_nDiv; i++)
	{
		float radFrom = (i*m_dDeg + m_showDegOffset) * DEG_2_RAD;
		float radTo = ((i+1)*m_dDeg + m_showDegOffset) * DEG_2_RAD;
		float d = m_pDiv[i].d() * m_showScale;

		vDouble2 pFrom,pTo;
		pFrom.x = sin(radFrom);
		pFrom.y = -cos(radFrom);
		pTo.x = sin(radTo);
		pTo.y = -cos(radTo);

		line(*pMat, pCenter + Point(pFrom.x*d,pFrom.y*d), pCenter + Point(pTo.x*d,pTo.y*d), colD, 2);
		line(*pMat, pCenter + Point(pFrom.x*rMax,pFrom.y*rMax), pCenter, col, 1);
		line(*pMat, pCenter, pCenter + Point(pTo.x*rMax,pTo.y*rMax), col, 1);
	}
#endif

}

}
