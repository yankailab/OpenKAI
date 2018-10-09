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

	m_freqExtra1 = 1;
	m_freqPos = 1;
	m_freqSendHeartbeat = 1;
	m_freqRC = 0;
	m_freqExtStat = 1;
}

APcopter_base::~APcopter_base()
{
}

bool APcopter_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,freqExtra1);
	KISSm(pK,freqPos);
	KISSm(pK,freqSendHeartbeat);
	KISSm(pK,freqRC);
	KISSm(pK,freqExtStat);

	if(m_freqSendHeartbeat > 0)
		m_freqSendHeartbeat = USEC_1SEC / m_freqSendHeartbeat;
	else
		m_freqSendHeartbeat = 0;

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

	//Send Heartbeat
	if(m_freqSendHeartbeat > 0 && m_tStamp - m_lastHeartbeat >= m_freqSendHeartbeat)
	{
		m_pMavlink->sendHeartbeat();
		m_lastHeartbeat = m_tStamp;
	}

	//request updates from Mavlink
	if(m_freqExtra1 > 0 && m_tStamp - m_pMavlink->m_msg.time_stamps.attitude > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_freqExtra1);

	if(m_freqPos > 0 && m_tStamp - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_freqPos);

	if(m_freqRC > 0 && m_tStamp - m_pMavlink->m_msg.time_stamps.rc_channels_raw > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_RC_CHANNELS, m_freqRC);

	if(m_freqExtStat > 0 && m_tStamp - m_pMavlink->m_msg.time_stamps.mission_current > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTENDED_STATUS, m_freqExtStat);
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

string APcopter_base::apModeName(void)
{
	if(m_apMode >= AP_N_CUSTOM_MODE)return "Unknown";

	return custom_mode_name[m_apMode];
}

bool APcopter_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg;

	pWin->tabNext();

	msg = "apMode = " + i2str(m_apMode) + ": " + apModeName();
	pWin->addMsg(&msg);

	msg = "y=" + f2str((double)m_pMavlink->m_msg.attitude.yaw) +
			" p=" + f2str((double)m_pMavlink->m_msg.attitude.pitch) +
			" r=" + f2str((double)m_pMavlink->m_msg.attitude.roll);
	pWin->addMsg(&msg);

	msg = "hdg=" + f2str(((double)m_pMavlink->m_msg.global_position_int.hdg)*0.01);
	pWin->addMsg(&msg);

	msg = "height=" + f2str(((double)m_pMavlink->m_msg.global_position_int.alt)*0.001);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_base::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;

	msg = "apMode = " + i2str(m_apMode) + ": " + apModeName();
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "y=" + f2str((double)m_pMavlink->m_msg.attitude.yaw) +
			" p=" + f2str((double)m_pMavlink->m_msg.attitude.pitch) +
			" r=" + f2str((double)m_pMavlink->m_msg.attitude.roll);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "hdg=" + f2str(((double)m_pMavlink->m_msg.global_position_int.hdg)*0.01);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "height=" + f2str(((double)m_pMavlink->m_msg.global_position_int.alt)*0.001);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
