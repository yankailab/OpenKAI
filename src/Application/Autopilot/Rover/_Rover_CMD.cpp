#include "_Rover_CMD.h"

namespace kai
{
_Rover_CMD::_Rover_CMD()
{
	m_mode = rover_unknown;
	m_nPwmIn = 8;
	m_pPwmIn = NULL;
}

_Rover_CMD::~_Rover_CMD()
{
	DEL(m_pPwmIn);
}

bool _Rover_CMD::init(void* pKiss)
{
	IF_F(!this->_ProtocolBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("nPwmIn", &m_nPwmIn);
	m_pPwmIn = new uint16_t[m_nPwmIn];
	for(int i=0; i<m_nPwmIn; i++)
		m_pPwmIn[i]=0;

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
		for(int i=0; i<m_nPwmIn; i++)
		{
			if(i>=m_nPwmIn)break;
			m_pPwmIn[i] = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[4+i*2], false);
		}
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

void _Rover_CMD::setPWM(uint16_t* pPWM, int nCh)
{
	IF_(check()<0);
	NULL_(pPWM);
	IF_(nCh <= 0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = ROVERCMD_PWM;
	m_pBuf[2] = nCh * 2;

	for (int i = 0; i < nCh; i++)
	{
		m_pBuf[PROTOCOL_N_HEADER + i * 2] = (uint8_t)(pPWM[i] & 0xFF);
		m_pBuf[PROTOCOL_N_HEADER + i * 2 + 1] = (uint8_t)((pPWM[i] >> 8) & 0xFF);
	}

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
	msg = "Mode: " + c_roverModeName[m_mode];
	addMsg(msg,1);

	msg = "PwmIn: ";
	for(int i=0; i<m_nPwmIn; i++)
		msg += ", ch" + i2str(i) + "=" + i2str(m_pPwmIn[i]);
	addMsg(msg,1);

}

}
