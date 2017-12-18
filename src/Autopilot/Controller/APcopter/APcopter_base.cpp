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

	m_pidRoll.reset();
	m_pidPitch.reset();
	m_pidYaw.reset();
	m_pidThr.reset();

	m_ctrlRoll.reset();
	m_ctrlPitch.reset();
	m_ctrlYaw.reset();
	m_ctrlThr.reset();
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

	Kiss* pCC;
	APcopter_PID cPID;

	pCC = pK->o("roll");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	cPID.m_lim *= DEG_RAD;
	m_pidRoll = cPID;

	pCC = pK->o("pitch");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	cPID.m_lim *= DEG_RAD;
	m_pidPitch = cPID;

	pCC = pK->o("thr");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	m_pidThr = cPID;

	pCC = pK->o("yaw");
	cPID.reset();
	F_INFO(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	F_INFO(pCC->v("rate", &cPID.m_rate));
	F_INFO(pCC->v("lim", &cPID.m_lim));
	cPID.m_lim *= DEG_RAD;
	m_pidYaw = cPID;

	//init controls
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

	//update APM status from heartbeat msg
	m_flightMode = m_pMavlink->m_msg.heartbeat.custom_mode;

	uint64_t tNow = getTimeUsec();

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

}

bool APcopter_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = *this->getName()+": Flight Mode=" + i2str(m_flightMode);
	pWin->addMsg(&msg);

	return true;
}



}
