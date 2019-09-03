#include "_OKlinkAPcopter.h"

namespace kai
{
_OKlinkAPcopter::_OKlinkAPcopter()
{
	m_vPos.init();
	m_tPos = 0;
}

_OKlinkAPcopter::~_OKlinkAPcopter()
{
}

bool _OKlinkAPcopter::init(void* pKiss)
{
	IF_F(!this->_OKlink::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	return true;
}

bool _OKlinkAPcopter::start(void)
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

void _OKlinkAPcopter::update(void)
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

void _OKlinkAPcopter::handleCMD(void)
{
	uint16_t x,y;
	switch (m_recvMsg.m_pBuf[1])
	{
	case OKLINK_POS:
		x = *((uint16_t*)&m_recvMsg.m_pBuf[3]);
		y = *((uint16_t*)&m_recvMsg.m_pBuf[5]);
		m_vPos.x = ((float)x)*0.001;
		m_vPos.y = ((float)y)*0.001;
		m_tPos = m_tStamp;
		LOG_I("Pos=("+f2str(m_vPos.x)+","+f2str(m_vPos.y)+")");
		break;
	default:
		break;
	}

	m_recvMsg.init();
}

void _OKlinkAPcopter::setPos(vFloat2 vP)
{
	NULL_(m_pIO);
	IF_(!m_pIO->isOpen());

	m_pBuf[0] = OKLINK_BEGIN;
	m_pBuf[1] = OKLINK_POS;
	m_pBuf[2] = 4;

	uint16_t x = (uint16_t)(vP.x * 1000);
	uint16_t y = (uint16_t)(vP.y * 1000);

	m_pBuf[3] = (uint8_t)(x & 0xFF);
	m_pBuf[4] = (uint8_t)((x >> 8) & 0xFF);
	m_pBuf[5] = (uint8_t)(y & 0xFF);
	m_pBuf[6] = (uint8_t)((y >> 8) & 0xFF);

	m_pIO->write(m_pBuf, OKLINK_N_HEADER + 4);
}

bool _OKlinkAPcopter::draw(void)
{
	IF_F(!this->_OKlink::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;
	pWin->tabNext();
	pWin->tabPrev();

	return true;
}

bool _OKlinkAPcopter::console(int& iY)
{
	IF_F(!this->_OKlink::console(iY));
	string msg;

	return true;
}

}
