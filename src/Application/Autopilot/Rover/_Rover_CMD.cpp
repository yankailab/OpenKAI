#include "_Rover_CMD.h"

namespace kai
{
_Rover_CMD::_Rover_CMD()
{
	m_mode = rover_mode_unknown;
	m_action = rover_action_unknown;
	m_nSpeed = -1.0;
	m_dSpeed = -1.0;
}

_Rover_CMD::~_Rover_CMD()
{
}

bool _Rover_CMD::init(void* pKiss)
{
	IF_F(!this->_ProtocolBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	return true;
}

bool _Rover_CMD::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Rover_CMD::update(void)
{
	while (m_bThreadON)
	{
		if(!m_pIO)
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		if(!m_pIO->isOpen())
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		this->autoFPSfrom();

		while(readCMD())
		{
			handleCMD();
			m_nCMDrecv++;
		}

		this->autoFPSto();
	}
}

void _Rover_CMD::handleCMD(void)
{
	switch (m_recvMsg.m_pBuf[1])
	{
	case ROVERCMD_STATE:
		m_mode = (ROVER_MODE)m_recvMsg.m_pBuf[3];
		m_action = (ROVER_ACTION)m_recvMsg.m_pBuf[4];
		m_nSpeed = FLT_SCALE_INV * (float)((int16_t)unpack_int16(&m_recvMsg.m_pBuf[5], false));
		m_dSpeed = FLT_SCALE_INV * (float)((int16_t)unpack_int16(&m_recvMsg.m_pBuf[7], false));
		break;
	default:
		break;
	}

	m_recvMsg.reset();
}

void _Rover_CMD::sendState(int iState)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = ROVERCMD_STATE;
	m_pBuf[2] = 4;
	pack_int32(&m_pBuf[3], iState, false);

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + 4);
}

void _Rover_CMD::setSpeed(float nSpeed, float dSpeed)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = ROVERCMD_SPEED;
	m_pBuf[2] = 4;

	int16_t v = (int16_t)(nSpeed * FLT_SCALE);
	m_pBuf[3] = (uint8_t)(v & 0xFF);
	m_pBuf[4] = (uint8_t)((v >> 8) & 0xFF);
	v = (int16_t)(dSpeed * FLT_SCALE);
	m_pBuf[5] = (uint8_t)(v & 0xFF);
	m_pBuf[6] = (uint8_t)((v >> 8) & 0xFF);

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + m_pBuf[2]);
}

void _Rover_CMD::pinOut(uint8_t iPin, uint8_t state)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = ROVERCMD_PINOUT;
	m_pBuf[2] = 2;
	m_pBuf[3] = iPin;
	m_pBuf[4] = state;

	m_pIO->write(m_pBuf, 5);
}

void _Rover_CMD::draw(void)
{
	this->_ProtocolBase::draw();

	string msg;
	msg = "mode=" + c_roverModeName[m_mode]
			+ ", action=" + c_roverActionName[m_action]
		    + ", nSpeed=" + f2str(m_nSpeed)
			+ ", dSpeed="+f2str(m_dSpeed);
	addMsg(msg,1);
}

}
