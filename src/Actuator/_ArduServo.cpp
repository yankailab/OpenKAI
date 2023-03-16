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

	bool _ArduServo::init(void *pKiss)
	{
		IF_F(!this->_ActuatorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		F_ERROR_F(pK->v("_IOBase", &n));
		m_pIO = (_IOBase *)(pK->getInst(n));
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
		while (m_pT->bRun())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			if (!m_pIO->isOpen())
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			m_pT->autoFPSfrom();


			this->setPtarget(0, 0.5, true);
			this->setPtarget(1, 0.5, true);
			

			updatePWM();

			while (readCMD())
			{
				handleCMD();
				m_nCMDrecv++;
			}

			m_pT->autoFPSto();
		}
	}

	void _ArduServo::updatePWM(void)
	{
		NULL_(m_pIO);
		IF_(!m_pIO->isOpen());

		int i;
		uint16_t pChan[16];
		int nC = m_vAxis.size();
		for (i = 0; i < nC; i++)
		{
			ACTUATOR_AXIS *pA = &m_vAxis[i];
			pChan[i] = 1500;
			if (pA->m_p.bInRange(pA->m_p.m_vTarget))
				pChan[i] = pA->m_p.m_vTarget;

			pA->m_p.m_v = pA->m_p.m_vTarget;
		}

		m_pB[0] = ARDUSV_BEGIN;
		m_pB[1] = ARDU_CMD_PWM;
		m_pB[2] = nC * 2;
		int j = 3;
		for (int i = 0; i < nC; i++)
		{
			uint16_t v = pChan[i];
			m_pB[j++] = ((uint8_t)(v & 0xFF));
			m_pB[j++] = ((uint8_t)((v >> 8) & 0xFF));
		}

		m_pIO->write(m_pB, ARDUSV_N_HEADER + nC * 2);
	}

	bool _ArduServo::readCMD(void)
	{
		uint8_t inByte;
		int byteRead;

		while ((byteRead = m_pIO->read(&inByte, 1)) > 0)
		{
			if (m_recvMsg.m_cmd != 0)
			{
				m_recvMsg.m_pB[m_recvMsg.m_iByte] = inByte;
				m_recvMsg.m_iByte++;

				if (m_recvMsg.m_iByte == 3)
				{
					m_recvMsg.m_nPayload = inByte;
				}
				else if (m_recvMsg.m_iByte == m_recvMsg.m_nPayload + ARDUSV_N_HEADER)
				{
					return true;
				}
			}
			else if (inByte == ARDUSV_BEGIN)
			{
				m_recvMsg.m_cmd = inByte;
				m_recvMsg.m_pB[0] = inByte;
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

		switch (m_recvMsg.m_pB[1])
		{
		case ARDU_CMD_STATUS:
			pwm1 = (uint16_t)unpack_int16(&m_recvMsg.m_pB[3], false);
			pwm2 = (uint16_t)unpack_int16(&m_recvMsg.m_pB[5], false);

			LOG_I("pwm1=" + i2str(pwm1) + ", pwm2=" + i2str(pwm2));
			break;
		default:
			break;
		}

		m_recvMsg.init();
	}

	void _ArduServo::console(void *pConsole)
	{
#ifdef WITH_UI
		NULL_(pConsole);
		this->_ActuatorBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_pIO->isOpen())
		{
			pC->addMsg("Not Connected", 1);
			return;
		}

		pC->addMsg("nCMD = " + i2str(m_nCMDrecv), 1);
#endif
	}

}
