#include "APMcopter_base.h"

namespace kai
{

APMcopter_base::APMcopter_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

}

APMcopter_base::~APMcopter_base()
{
}

bool APMcopter_base::init(Kiss* pKiss)
{
	CHECK_F(this->ActionBase::init(pKiss)==false);
	pKiss->m_pInst = this;

	int i;
	Kiss* pCC;
	APMcopter_PID cPID;

	pCC = pKiss->o("roll");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidRoll = cPID;

	pCC = pKiss->o("pitch");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidPitch = cPID;

	pCC = pKiss->o("alt");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidAlt = cPID;

	pCC = pKiss->o("yaw");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidYaw = cPID;

	//init controls
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	return true;
}

bool APMcopter_base::link(void)
{
	NULL_F(m_pKiss);

	string iName = "";

	F_INFO(m_pKiss->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (m_pKiss->root()->getChildInstByName(&iName));

	return true;
}


void APMcopter_base::sendHeartbeat(void)
{
	NULL_(m_pMavlink);

	//Sending Heartbeat at 1Hz
	uint64_t timeNow = get_time_usec();
	if (timeNow - m_lastHeartbeat >= USEC_1SEC)
	{
		m_pMavlink->sendHeartbeat();
		m_lastHeartbeat = timeNow;

#ifdef MAVLINK_DEBUG
		printf("   SENT HEARTBEAT:%d\n", (++m_iHeartbeat));
#endif
	}
}


}
