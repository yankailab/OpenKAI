#include "../../Controller/APcopter/APcopter_base.h"

namespace kai
{

APcopter_base::APcopter_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;
	m_flightMode = 0;

	m_freqAtti = 0;
	m_freqGlobalPos = 0;
	m_freqHeartbeat = 0;

}

APcopter_base::~APcopter_base()
{
}

bool APcopter_base::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,freqAtti);
	KISSm(pK,freqGlobalPos);
	KISSm(pK,freqHeartbeat);

	if(m_freqHeartbeat > 0)
		m_freqHeartbeat = USEC_1SEC / m_freqHeartbeat;
	else
		m_freqHeartbeat = 0;

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	return true;
}

bool APcopter_base::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void APcopter_base::update(void)
{
	this->ActionBase::update();
	NULL_(m_pMavlink);

	uint64_t tNow = getTimeUsec();

	//update APM status from heartbeat msg
	m_flightMode = m_pMavlink->m_msg.heartbeat.custom_mode;

	//Sending Heartbeat
	if(m_freqHeartbeat > 0)
	{
		if (tNow - m_lastHeartbeat >= m_freqHeartbeat)
		{
			m_pMavlink->sendHeartbeat();
			m_lastHeartbeat = tNow;
		}
	}

	//request updates from Mavlink
	if(m_freqAtti > 0)
	{
		if(tNow - m_pMavlink->m_msg.time_stamps.attitude > USEC_1SEC)
			m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_freqAtti);
	}

	if(m_freqGlobalPos > 0)
	{
		if(tNow - m_pMavlink->m_msg.time_stamps.global_position_int > USEC_1SEC)
			m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_freqGlobalPos);
	}

	//test
	static uint16_t rcPWM = 1100;
	rcPWM += 10;
	if(rcPWM >= 1900)rcPWM = 1100;

	uint16_t rcN = 1500;

	__mavlink_rc_channels_override_t rc;
	rc.chan1_raw = rcN;
	rc.chan2_raw = rcN;
	rc.chan3_raw = (uint16_t)rcPWM;
	rc.chan4_raw = rcN;
	rc.chan5_raw = rcN;
	rc.chan6_raw = (uint16_t)rcPWM;
	rc.chan7_raw = (uint16_t)rcPWM;
	rc.chan8_raw = rcN;
	m_pMavlink->rcChannelsOverride(rc);
	LOG_I("RC PWM: "+i2str(rcPWM));

	m_pMavlink->clComponentArmDisarm(1);

//	m_pMavlink->clDoSetServo(6,rcPWM);
}

bool APcopter_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = *this->getName()+": Flight Mode=" + i2str(m_flightMode);
	pWin->addMsg(&msg);

	return true;
}

}
