#include "_APcopter_link.h"

namespace kai
{
_APcopter_link::_APcopter_link()
{
	m_vPos.init();
	m_tPos = 0;
}

_APcopter_link::~_APcopter_link()
{
}

bool _APcopter_link::init(void* pKiss)
{
	IF_F(!this->_ProtocolBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	return true;
}

bool _APcopter_link::start(void)
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

void _APcopter_link::update(void)
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

void _APcopter_link::handleCMD(void)
{
	int16_t x,y,z;
	switch (m_recvMsg.m_pBuf[1])
	{
	case APLINK_POS:
		x = unpack_int16(&m_recvMsg.m_pBuf[3], false);
		y = unpack_int16(&m_recvMsg.m_pBuf[5], false);
		z = unpack_int16(&m_recvMsg.m_pBuf[7], false);
		m_vPos.x = ((float)x)*0.001;
		m_vPos.y = ((float)y)*0.001;
		m_vPos.z = ((float)z)*0.001;
		m_tPos = m_tStamp;
		LOG_I("Pos=("+f2str(m_vPos.x)+","+f2str(m_vPos.y)+","+f2str(m_vPos.z)+")");
		break;
	default:
		break;
	}

	m_recvMsg.reset();
}

void _APcopter_link::setPos(vFloat3 vP)
{
	NULL_(m_pIO);
	IF_(!m_pIO->isOpen());

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = APLINK_POS;
	m_pBuf[2] = 6;

	pack_uint16(&m_pBuf[3], (int16_t)(vP.x * 1000), false);
	pack_uint16(&m_pBuf[5], (int16_t)(vP.y * 1000), false);
	pack_int16(&m_pBuf[7], (int16_t)(vP.z * 1000), false);

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + 6);
}

bool _APcopter_link::draw(void)
{
	IF_F(!this->_ProtocolBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;
	pWin->tabNext();
	pWin->tabPrev();

	return true;
}

bool _APcopter_link::console(int& iY)
{
	IF_F(!this->_ProtocolBase::console(iY));
	string msg;

	return true;
}

}
