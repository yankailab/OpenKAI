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

	ARDUSERVO_CHAN c;
	int i = 0;
	while (1)
	{
		Kiss* pC = pK->child(i++);
		if(pC->empty())break;

		c.init();
		pC->v("pwmL",&c.m_pwmL);
		pC->v("pwmH",&c.m_pwmH);
		pC->v("dir",&c.m_dir);
		m_vServo.push_back(c);
	}

	string n;
	n = "";
	F_ERROR_F(pK->v("_IOBase", &n));
	m_pIO = (_IOBase*) (pK->getInst(n));
	NULL_Fl(m_pIO, n + ": not found");

	return true;
}

bool _ArduServo::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _ArduServo::update(void)
{
	while(m_pT->bRun())
	{
		if(!m_pIO)
		{
			m_pT->sleepTime(USEC_1SEC);
			continue;
		}

		if(!m_pIO->isOpen())
		{
			m_pT->sleepTime(USEC_1SEC);
			continue;
		}

		m_pT->autoFPSfrom();

		updatePWM();

		while(readCMD())
		{
			handleCMD();
			m_nCMDrecv++;
		}

		m_pT->autoFPSto();
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
			else if (m_recvMsg.m_iByte == m_recvMsg.m_nPayload + OKLINK_N_HEADER)
			{
				return true;
			}
		}
		else if (inByte == OKLINK_BEGIN)
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

	ACTUATOR_AXIS* pA = &m_vAxis[0];
	IF_(pA->m_p.m_vTarget < 0.0);

	int nChan = m_vServo.size();
	uint16_t pChan[8];
	int i;
	for(i=0; i<nChan; i++)
	{
		ARDUSERVO_CHAN* pC = &m_vServo[i];
		uint16_t dPwm = pC->m_pwmH - pC->m_pwmL;

		pChan[i] = (pA->m_p.m_vTarget*pC->m_dir + 0.5*(1 - pC->m_dir)) * dPwm + pC->m_pwmL;
	}

	m_pBuf[0] = OKLINK_BEGIN;
	m_pBuf[1] = ARDU_CMD_PWM;
	m_pBuf[2] = nChan * 2;

	for (int i = 0; i < nChan; i++)
	{
		m_pBuf[OKLINK_N_HEADER + i * 2] = (uint8_t)(pChan[i] & 0xFF);
		m_pBuf[OKLINK_N_HEADER + i * 2 + 1] = (uint8_t)((pChan[i] >> 8) & 0xFF);
	}

	m_pIO->write(m_pBuf, OKLINK_N_HEADER + nChan * 2);

	pA->m_p.m_v = pA->m_p.m_vTarget;
}

void _ArduServo::draw(void)
{
	this->_ActuatorBase::draw();

	if (!m_pIO->isOpen())
	{
		addMsg("Not Connected",1);
		return;
	}

	addMsg("nCMD = " + i2str(m_nCMDrecv),1);
}

}
