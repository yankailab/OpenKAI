#include "_OKlink.h"

namespace kai
{
_OKlink::_OKlink()
{
	m_pIO = NULL;
	m_pBuf = NULL;
	m_nBuf = 256;
	m_nCMDrecv = 0;

	m_pfCallback = NULL;
	m_pfInst = NULL;
}

_OKlink::~_OKlink()
{
}

bool _OKlink::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("nBuf", &m_nBuf);
	m_pBuf = new uint8_t[m_nBuf];
	m_recvMsg.init(m_nBuf);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pIO, iName + ": not found");

	return true;
}

bool _OKlink::start(void)
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

int _OKlink::check(void)
{
	NULL__(m_pIO,-1);
	IF__(!m_pIO->isOpen(),-1);
	NULL_F(m_pBuf);

	return 0;
}

void _OKlink::update(void)
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

bool _OKlink::readCMD(void)
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
				m_recvMsg.m_nPayload = m_recvMsg.m_pBuf[2];
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

void _OKlink::handleCMD(void)
{
	if(m_pfCallback)
	{
		m_pfCallback(m_recvMsg.m_pBuf, m_pfInst);
	}

	m_recvMsg.reset();
}

void _OKlink::setCallback(CallbackCMD cb, void* pInst)
{
	NULL_(cb);
	NULL_(pInst);

	m_pfCallback = cb;
	m_pfInst = pInst;
}

void _OKlink::sendState(int iState)
{
	IF_(check()<0);

	m_pBuf[0] = OKLINK_BEGIN;
	m_pBuf[1] = OKLINK_STATE;
	m_pBuf[2] = 4;
	pack_int32(&m_pBuf[3], iState, false);

	m_pIO->write(m_pBuf, OKLINK_N_HEADER + 4);
}

void _OKlink::setPWM(int nChan, uint16_t* pChan)
{
	IF_(check()<0);
	IF_(nChan <= 0);

	m_pBuf[0] = OKLINK_BEGIN;
	m_pBuf[1] = OKLINK_PWM;
	m_pBuf[2] = nChan * 2;

	for (int i = 0; i < nChan; i++)
	{
		m_pBuf[OKLINK_N_HEADER + i * 2] = (uint8_t)(pChan[i] & 0xFF);
		m_pBuf[OKLINK_N_HEADER + i * 2 + 1] = (uint8_t)((pChan[i] >> 8) & 0xFF);
	}

	m_pIO->write(m_pBuf, OKLINK_N_HEADER + nChan * 2);
}

void _OKlink::pinOut(uint8_t iPin, uint8_t state)
{
	IF_(check()<0);

	m_pBuf[0] = OKLINK_BEGIN;
	m_pBuf[1] = OKLINK_PIN_OUTPUT;
	m_pBuf[2] = 2;
	m_pBuf[3] = iPin;
	m_pBuf[4] = state;

	m_pIO->write(m_pBuf, 5);
}

void _OKlink::sendBB(uint32_t id, uint16_t iClass, vFloat4& bb)
{
	IF_(check()<0);

	m_pBuf[0] = OKLINK_BEGIN;
	m_pBuf[1] = OKLINK_BB;
	m_pBuf[2] = 14;
	pack_uint32(&m_pBuf[3], id, false);
	pack_int16(&m_pBuf[7], iClass, false);
	pack_uint16(&m_pBuf[9], (uint16_t)(bb.x*1000.0), false);
	pack_uint16(&m_pBuf[11], (uint16_t)(bb.y*1000.0), false);
	pack_uint16(&m_pBuf[13], (uint16_t)(bb.z*1000.0), false);
	pack_uint16(&m_pBuf[15], (uint16_t)(bb.w*1000.0), false);

	m_pIO->write(m_pBuf, OKLINK_N_HEADER + 14);
}

bool _OKlink::draw(void)
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

bool _OKlink::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));
	IF_Fl(!m_pIO->isOpen(), "Not connected");
	string msg;

	C_MSG("nCMD = " + i2str(m_nCMDrecv));
	return true;
}

}
