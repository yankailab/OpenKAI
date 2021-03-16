#include "../ArduPilot/_AP_link.h"

namespace kai
{
_AP_link::_AP_link()
{
	m_iState = 0;
	m_tState = 0;
	m_vBB.init();
	m_tBB = 0;
	m_vTargetBB.init();
	m_tTargetBB = 0;
	m_alt = 0.0;
	m_tAlt = 0;
	m_hdg = 0.0;
	m_tHdg = 0;
}

_AP_link::~_AP_link()
{
}

bool _AP_link::init(void* pKiss)
{
	IF_F(!this->_ProtocolBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	return true;
}

bool _AP_link::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AP_link::check(void)
{
	return this->_ProtocolBase::check();
}

void _AP_link::update(void)
{
	while(m_pT->bRun())
	{
		if(!m_pIO)
		{
			m_pT->sleepT (USEC_1SEC);
			continue;
		}

		if(!m_pIO->isOpen())
		{
			m_pT->sleepT (USEC_1SEC);
			continue;
		}

		m_pT->autoFPSfrom();

		while(readCMD())
		{
			handleCMD();
			m_nCMDrecv++;
		}

		m_pT->autoFPSto();
	}
}

void _AP_link::handleCMD(void)
{
	int16_t x,y,z,w;
	switch (m_recvMsg.m_pB[1])
	{
	case APLINK_STATE:
		m_iState = m_recvMsg.m_pB[3];
		m_tState = m_pT->getTfrom();
		LOG_I("State="+i2str((int)m_iState));
		break;

	case APLINK_BB:
		x = unpack_int16(&m_recvMsg.m_pB[3], false);
		y = unpack_int16(&m_recvMsg.m_pB[5], false);
		z = unpack_int16(&m_recvMsg.m_pB[7], false);
		w = unpack_int16(&m_recvMsg.m_pB[9], false);
		m_vBB.x = ((float)x)*0.001;
		m_vBB.y = ((float)y)*0.001;
		m_vBB.z = ((float)z)*0.001;
		m_vBB.w = ((float)w)*0.001;
		m_tBB = m_pT->getTfrom();
		LOG_I("BB=("+f2str(m_vBB.x)+","+f2str(m_vBB.y)+","
					+f2str(m_vBB.z)+","+f2str(m_vBB.w)+")");
		break;

	case APLINK_TARGET:
		x = unpack_int16(&m_recvMsg.m_pB[3], false);
		y = unpack_int16(&m_recvMsg.m_pB[5], false);
		z = unpack_int16(&m_recvMsg.m_pB[7], false);
		w = unpack_int16(&m_recvMsg.m_pB[9], false);
		m_vTargetBB.x = ((float)x)*0.001;
		m_vTargetBB.y = ((float)y)*0.001;
		m_vTargetBB.z = ((float)z)*0.001;
		m_vTargetBB.w = ((float)w)*0.001;
		m_tTargetBB = m_pT->getTfrom();
		LOG_I("TargetBB=("+f2str(m_vTargetBB.x)+","+f2str(m_vTargetBB.y)+","
					+f2str(m_vTargetBB.z)+","+f2str(m_vTargetBB.w)+")");
		break;

	case APLINK_ALT:
		x = unpack_int16(&m_recvMsg.m_pB[3], false);
		m_alt = ((float)x)*0.001;
		m_tAlt = m_pT->getTfrom();
		LOG_I("Alt="+f2str(m_alt));
		break;

	case APLINK_HDG:
		x = unpack_int16(&m_recvMsg.m_pB[3], false);
		m_hdg = ((float)x)*0.001;
		m_tHdg = m_pT->getTfrom();
		LOG_I("Hdg="+f2str(m_hdg));
		break;

	default:
		break;
	}

	m_recvMsg.reset();
}

void _AP_link::state(uint8_t iState)
{
	IF_(check()<0);

	m_pBuf[0] = PB_BEGIN;
	m_pBuf[1] = APLINK_STATE;
	m_pBuf[2] = 1;
	m_pBuf[3] = iState;

	m_pIO->write(m_pBuf, PB_N_HDR + (int)m_pBuf[2]);
}

void _AP_link::setBB(vFloat4& vP)
{
	IF_(check()<0);

	m_pBuf[0] = PB_BEGIN;
	m_pBuf[1] = APLINK_BB;
	m_pBuf[2] = 8;

	pack_int16(&m_pBuf[3], (int16_t)(vP.x * 1000), false);
	pack_int16(&m_pBuf[5], (int16_t)(vP.y * 1000), false);
	pack_int16(&m_pBuf[7], (int16_t)(vP.z * 1000), false);
	pack_int16(&m_pBuf[9], (int16_t)(vP.w * 1000), false);

	m_pIO->write(m_pBuf, PB_N_HDR + (int)m_pBuf[2]);
}

void _AP_link::setTargetBB(vFloat4& vP)
{
	IF_(check()<0);

	m_pBuf[0] = PB_BEGIN;
	m_pBuf[1] = APLINK_TARGET;
	m_pBuf[2] = 8;

	pack_int16(&m_pBuf[3], (int16_t)(vP.x * 1000), false);
	pack_int16(&m_pBuf[5], (int16_t)(vP.y * 1000), false);
	pack_int16(&m_pBuf[7], (int16_t)(vP.z * 1000), false);
	pack_int16(&m_pBuf[9], (int16_t)(vP.w * 1000), false);

	m_pIO->write(m_pBuf, PB_N_HDR + (int)m_pBuf[2]);
}

void _AP_link::setAlt(float dA)
{
	IF_(check()<0);

	m_pBuf[0] = PB_BEGIN;
	m_pBuf[1] = APLINK_ALT;
	m_pBuf[2] = 2;
	pack_int16(&m_pBuf[3], (int16_t)(dA * 1000), false);

	m_pIO->write(m_pBuf, PB_N_HDR + (int)m_pBuf[2]);
}

void _AP_link::setHdg(float dH)
{
	IF_(check()<0);

	m_pBuf[0] = PB_BEGIN;
	m_pBuf[1] = APLINK_HDG;
	m_pBuf[2] = 2;
	pack_int16(&m_pBuf[3], (int16_t)(dH * 1000), false);

	m_pIO->write(m_pBuf, PB_N_HDR + (int)m_pBuf[2]);
}

void _AP_link::draw(void)
{
	this->_ProtocolBase::draw();
}

}
