#include "_APcopter_link.h"

namespace kai
{
_APcopter_link::_APcopter_link()
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

int _APcopter_link::check(void)
{
	return this->_ProtocolBase::check();
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
	int16_t x,y,z,w;
	switch (m_recvMsg.m_pBuf[1])
	{
	case APLINK_STATE:
		m_iState = m_recvMsg.m_pBuf[3];
		m_tState = m_tStamp;
		LOG_I("State="+i2str((int)m_iState));
		break;

	case APLINK_BB:
		x = unpack_int16(&m_recvMsg.m_pBuf[3], false);
		y = unpack_int16(&m_recvMsg.m_pBuf[5], false);
		z = unpack_int16(&m_recvMsg.m_pBuf[7], false);
		w = unpack_int16(&m_recvMsg.m_pBuf[9], false);
		m_vBB.x = ((float)x)*0.001;
		m_vBB.y = ((float)y)*0.001;
		m_vBB.z = ((float)z)*0.001;
		m_vBB.w = ((float)w)*0.001;
		m_tBB = m_tStamp;
		LOG_I("BB=("+f2str(m_vBB.x)+","+f2str(m_vBB.y)+","
					+f2str(m_vBB.z)+","+f2str(m_vBB.w)+")");
		break;

	case APLINK_TARGET:
		x = unpack_int16(&m_recvMsg.m_pBuf[3], false);
		y = unpack_int16(&m_recvMsg.m_pBuf[5], false);
		z = unpack_int16(&m_recvMsg.m_pBuf[7], false);
		w = unpack_int16(&m_recvMsg.m_pBuf[9], false);
		m_vTargetBB.x = ((float)x)*0.001;
		m_vTargetBB.y = ((float)y)*0.001;
		m_vTargetBB.z = ((float)z)*0.001;
		m_vTargetBB.w = ((float)w)*0.001;
		m_tTargetBB = m_tStamp;
		LOG_I("TargetBB=("+f2str(m_vTargetBB.x)+","+f2str(m_vTargetBB.y)+","
					+f2str(m_vTargetBB.z)+","+f2str(m_vTargetBB.w)+")");
		break;

	case APLINK_ALT:
		x = unpack_int16(&m_recvMsg.m_pBuf[3], false);
		m_alt = ((float)x)*0.001;
		m_tAlt = m_tStamp;
		LOG_I("Alt="+f2str(m_alt));
		break;

	case APLINK_HDG:
		x = unpack_int16(&m_recvMsg.m_pBuf[3], false);
		m_hdg = ((float)x)*0.001;
		m_tHdg = m_tStamp;
		LOG_I("Hdg="+f2str(m_hdg));
		break;

	default:
		break;
	}

	m_recvMsg.reset();
}

void _APcopter_link::state(uint8_t iState)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = APLINK_STATE;
	m_pBuf[2] = 1;
	m_pBuf[3] = iState;

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + (int)m_pBuf[2]);
}

void _APcopter_link::setBB(vFloat4& vP)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = APLINK_BB;
	m_pBuf[2] = 8;

	pack_int16(&m_pBuf[3], (int16_t)(vP.x * 1000), false);
	pack_int16(&m_pBuf[5], (int16_t)(vP.y * 1000), false);
	pack_int16(&m_pBuf[7], (int16_t)(vP.z * 1000), false);
	pack_int16(&m_pBuf[9], (int16_t)(vP.w * 1000), false);

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + (int)m_pBuf[2]);
}

void _APcopter_link::setTargetBB(vFloat4& vP)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = APLINK_TARGET;
	m_pBuf[2] = 8;

	pack_int16(&m_pBuf[3], (int16_t)(vP.x * 1000), false);
	pack_int16(&m_pBuf[5], (int16_t)(vP.y * 1000), false);
	pack_int16(&m_pBuf[7], (int16_t)(vP.z * 1000), false);
	pack_int16(&m_pBuf[9], (int16_t)(vP.w * 1000), false);

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + (int)m_pBuf[2]);
}

void _APcopter_link::setAlt(float dA)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = APLINK_ALT;
	m_pBuf[2] = 2;
	pack_int16(&m_pBuf[3], (int16_t)(dA * 1000), false);

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + (int)m_pBuf[2]);
}

void _APcopter_link::setHdg(float dH)
{
	IF_(check()<0);

	m_pBuf[0] = PROTOCOL_BEGIN;
	m_pBuf[1] = APLINK_HDG;
	m_pBuf[2] = 2;
	pack_int16(&m_pBuf[3], (int16_t)(dH * 1000), false);

	m_pIO->write(m_pBuf, PROTOCOL_N_HEADER + (int)m_pBuf[2]);
}

void _APcopter_link::draw(void)
{
	this->_ProtocolBase::draw();
}

}
