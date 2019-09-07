#include "_RoverCMD.h"

namespace kai
{
_RoverCMD::_RoverCMD()
{
	m_mode = rover_idle;
	m_pwmModeIn = 0;
	m_pwmLin = 0;
	m_pwmRin = 0;
	m_pwmLmot = 0;
	m_pwmRmot = 0;
}

_RoverCMD::~_RoverCMD()
{
}

bool _RoverCMD::init(void* pKiss)
{
	IF_F(!this->_OKlink::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	return true;
}

bool _RoverCMD::start(void)
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

void _RoverCMD::update(void)
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

void _RoverCMD::handleCMD(void)
{
	switch (m_recvMsg.m_pBuf[1])
	{
	case OKLINK_STATUS:
		m_mode = (uint8_t)m_recvMsg.m_pBuf[3];
		m_pwmModeIn = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[4], false);
		m_pwmLin = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[6], false);
		m_pwmRin = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[8], false);
		m_pwmLmot = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[10], false);
		m_pwmRmot = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[12], false);

		LOG_I("Mode: " + c_roverModeName[m_mode] +
				", pwmModein=" + i2str(m_pwmModeIn) +
				", pwmLin=" + i2str(m_pwmLin) +
				", pwmRin=" + i2str(m_pwmRin) +
				", pwmLmot=" + i2str(m_pwmLmot) +
				", pwmRmot=" + i2str(m_pwmRmot)
				);
		break;
	default:
		break;
	}

	m_recvMsg.reset();
}

bool _RoverCMD::draw(void)
{
	IF_F(!this->_OKlink::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;

	pWin->tabNext();
	msg = "Mode: " + c_roverModeName[m_mode] +
			", pwmModein=" + i2str(m_pwmModeIn) +
			", pwmLin=" + i2str(m_pwmLin) +
			", pwmRin=" + i2str(m_pwmRin) +
			", pwmLmot=" + i2str(m_pwmLmot) +
			", pwmRmot=" + i2str(m_pwmRmot);
	pWin->addMsg(msg);
	pWin->tabPrev();

	return true;
}

bool _RoverCMD::console(int& iY)
{
	IF_F(!this->_OKlink::console(iY));
	string msg;

	C_MSG("Mode: " + c_roverModeName[m_mode]);
	C_MSG("pwmModein=" + i2str(m_pwmModeIn) +
			", pwmLin=" + i2str(m_pwmLin) +
			", pwmRin=" + i2str(m_pwmRin) +
			", pwmLmot=" + i2str(m_pwmLmot) +
			", pwmRmot=" + i2str(m_pwmRmot)
			);

	return true;
}

}
