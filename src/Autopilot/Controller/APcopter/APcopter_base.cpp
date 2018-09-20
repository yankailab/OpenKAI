#include "../../Controller/APcopter/APcopter_base.h"

namespace kai
{

APcopter_base::APcopter_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;
	m_apMode = 0;
	m_lastApMode = 0xffffffff;
	m_bApModeChanged = false;

	m_freqAtti = 0;
	m_freqGlobalPos = 0;
	m_freqHeartbeat = 0;
	m_freqRC = 0;
}

APcopter_base::~APcopter_base()
{
}

bool APcopter_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,freqAtti);
	KISSm(pK,freqGlobalPos);
	KISSm(pK,freqHeartbeat);
	KISSm(pK,freqRC);

	if(m_freqHeartbeat > 0)
		m_freqHeartbeat = USEC_1SEC / m_freqHeartbeat;
	else
		m_freqHeartbeat = 0;

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_F(m_pMavlink);

	return true;
}

int APcopter_base::check(void)
{
	NULL__(m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_base::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	//update Ardupilot mode
	m_apMode = m_pMavlink->m_msg.heartbeat.custom_mode;
	if(m_apMode != m_lastApMode)
	{
		m_bApModeChanged = true;
		m_lastApMode = m_apMode;
	}
	else
	{
		m_bApModeChanged = false;
	}

	//Sending Heartbeat
	if(m_freqHeartbeat > 0)
	{
		if (m_tStamp - m_lastHeartbeat >= m_freqHeartbeat)
		{
			m_pMavlink->sendHeartbeat();
			m_lastHeartbeat = m_tStamp;
		}
	}

	//request updates from Mavlink
	if(m_freqAtti > 0)
	{
		if(m_tStamp - m_pMavlink->m_msg.time_stamps.attitude > USEC_1SEC)
			m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_freqAtti);
	}

	if(m_freqGlobalPos > 0)
	{
		if(m_tStamp - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
			m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_freqGlobalPos);
	}

	if(m_freqRC > 0)
	{
		if(m_tStamp - m_pMavlink->m_msg.time_stamps.rc_channels_raw > USEC_1SEC)
			m_pMavlink->requestDataStream(MAV_DATA_STREAM_RC_CHANNELS, m_freqRC);
	}

}

void APcopter_base::setApMode(uint32_t iMode)
{
	IF_(check()<0);

	mavlink_set_mode_t D;
	D.custom_mode = iMode;
	m_pMavlink->setMode(D);
}

uint32_t APcopter_base::getApMode(void)
{
	return m_apMode;
}

void APcopter_base::setApArm(bool bArm)
{
	IF_(check()<0);

	m_pMavlink->clComponentArmDisarm(bArm);
}

bool APcopter_base::getApArm(void)
{
	return false; //TODO
}

bool APcopter_base::bApModeChanged(void)
{
	return m_bApModeChanged;
}

uint32_t APcopter_base::apMode(void)
{
	return m_apMode;
}

bool APcopter_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	pWin->tabNext();

	string msg = "apMode = " + i2str(m_apMode);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_base::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;

	msg = "apMode = " + i2str(m_apMode);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
