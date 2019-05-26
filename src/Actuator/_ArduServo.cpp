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
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

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
	m_recvMsg.init();
}

void _ArduServo::updatePWM(void)
{
	NULL_(m_pIO);
	IF_(!m_pIO->isOpen());

	m_nTargetPos = 0.6;//for test only
	IF_(m_nTargetPos < 0.0);

	int nChan = 2;
	uint16_t pChan[2];
	pChan[0] = m_nTargetPos * 1000 + 1000;
	pChan[1] = m_nTargetPos * 1000 + 1000;

	m_pBuf[0] = ARDU_CMD_BEGIN;
	m_pBuf[1] = ARDU_CMD_PWM;
	m_pBuf[2] = nChan * 2;

	for (int i = 0; i < nChan; i++)
	{
		m_pBuf[ARDU_CMD_N_HEADER + i * 2] = (uint8_t)(pChan[i] & 0xFF);
		m_pBuf[ARDU_CMD_N_HEADER + i * 2 + 1] = (uint8_t)((pChan[i] >> 8) & 0xFF);
	}

	m_pIO->write(m_pBuf, ARDU_CMD_N_HEADER + nChan * 2);
}

bool _ArduServo::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
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
	IF_F(!this->_ThreadBase::console(iY));
	IF_Fl(!m_pIO->isOpen(), "Not connected");
	string msg;

	C_MSG("nCMD = " + i2str(m_nCMDrecv));
	return true;
}

}
