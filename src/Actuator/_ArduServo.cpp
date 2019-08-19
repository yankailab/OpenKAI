#include "_ArduServo.h"

namespace kai
{
_ArduServo::_ArduServo()
{
	m_pIO = NULL;
	m_recvMsg.init();
	m_nCMDrecv = 0;
}

_ArduServo::~_ArduServo()
{
}

bool _ArduServo::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	Kiss** pItr = pK->getChildItr();
	ARDUSERVO_CHAN c;
	unsigned int i = 0;
	while (pItr[i])
	{
		Kiss* pC = pItr[i++];
		c.init();
		pC->v("pwmL",&c.m_pwmL);
		pC->v("pwmH",&c.m_pwmH);
		pC->v("dir",&c.m_dir);
		m_vServo.push_back(c);
	}

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pIO, iName + ": not found");

	return true;
}

bool _ArduServo::start(void)
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

void _ArduServo::update(void)
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

		updatePWM();

		while(readCMD())
		{
			handleCMD();
			m_nCMDrecv++;
		}

		this->autoFPSto();
	}
}

bool _ArduServo::readCMD(void)
{
	uint8_t	inByte;
	int		byteRead;

	while ((byteRead = m_pIO->read(&inByte,1)) > 0)
	{
		if (m_recvMsg.m_cmd != 0)
		{
			m_recvMsg.m_pBuf[m_recvMsg.m_iByte] = inByte;
			m_recvMsg.m_iByte++;

			if (m_recvMsg.m_iByte == 3)
			{
				m_recvMsg.m_nPayload = inByte;
			}
			else if (m_recvMsg.m_iByte == m_recvMsg.m_nPayload + ARDU_CMD_N_HEADER)
			{
				return true;
			}
		}
		else if (inByte == ARDU_CMD_BEGIN)
		{
			m_recvMsg.m_cmd = inByte;
			m_recvMsg.m_pBuf[0] = inByte;
			m_recvMsg.m_iByte = 1;
			m_recvMsg.m_nPayload = 0;
		}
	}

	return false;
}

void _ArduServo::handleCMD(void)
{
	uint16_t pwm1;
	uint16_t pwm2;

	switch (m_recvMsg.m_pBuf[1])
	{
	case ARDU_CMD_STATUS:
		pwm1 = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[3], false);
		pwm2 = (uint16_t)unpack_int16(&m_recvMsg.m_pBuf[5], false);

		LOG_I("pwm1=" + i2str(pwm1) + ", pwm2=" + i2str(pwm2));
		break;
	default:
		break;
	}

	m_recvMsg.init();
}

void _ArduServo::updatePWM(void)
{
	NULL_(m_pIO);
	IF_(!m_pIO->isOpen());
	IF_(m_vNormTargetPos.x < 0.0);

	int nChan = m_vServo.size();
	uint16_t pChan[8];
	int i;
	for(i=0; i<nChan; i++)
	{
		ARDUSERVO_CHAN* pC = &m_vServo[i];
		uint16_t dPwm = pC->m_pwmH - pC->m_pwmL;

		pChan[i] = (m_vNormTargetPos.x*pC->m_dir + 0.5*(1 - pC->m_dir)) * dPwm + pC->m_pwmL;
	}

	m_pBuf[0] = ARDU_CMD_BEGIN;
	m_pBuf[1] = ARDU_CMD_PWM;
	m_pBuf[2] = nChan * 2;

	for (int i = 0; i < nChan; i++)
	{
		m_pBuf[ARDU_CMD_N_HEADER + i * 2] = (uint8_t)(pChan[i] & 0xFF);
		m_pBuf[ARDU_CMD_N_HEADER + i * 2 + 1] = (uint8_t)((pChan[i] >> 8) & 0xFF);
	}

	m_pIO->write(m_pBuf, ARDU_CMD_N_HEADER + nChan * 2);

	m_vNormPos.x = m_vNormTargetPos.x;
}

bool _ArduServo::draw(void)
{
	IF_F(!this->_ActuatorBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;
	if (!m_pIO->isOpen())
	{
		pWin->tabNext();
		msg = "Not Connected";
		pWin->addMsg(msg);
		pWin->tabPrev();
		return false;
	}

	pWin->tabNext();
	msg = "nCMD = " + i2str(m_nCMDrecv);
	pWin->addMsg(msg);
	pWin->tabPrev();

	return true;
}

bool _ArduServo::console(int& iY)
{
	IF_F(!this->_ActuatorBase::console(iY));
	IF_Fl(!m_pIO->isOpen(), "Not connected");
	string msg;

	C_MSG("nCMD = " + i2str(m_nCMDrecv));
	return true;
}

}
