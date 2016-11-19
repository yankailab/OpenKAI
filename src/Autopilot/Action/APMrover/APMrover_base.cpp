#include "APMrover_base.h"

namespace kai
{

APMrover_base::APMrover_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	m_pidSpeed.reset();
	m_pidSteer.reset();
	m_ctrlSpeed.reset();
	m_ctrlSteer.reset();
}

APMrover_base::~APMrover_base()
{
}

bool APMrover_base::init(Config* pConfig)
{
	CHECK_F(this->ActionBase::init(pConfig)==false);
	pConfig->m_pInst = this;

	int i;
	Config* pCC;
	APMrover_PID cPID;

	pCC = pConfig->o("steer");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidSteer = cPID;

	pCC = pConfig->o("speed");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidSpeed = cPID;

	//init controls
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	return true;
}

bool APMrover_base::link(void)
{
	NULL_F(m_pConfig);

	string iName = "";

	F_INFO(m_pConfig->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (m_pConfig->root()->getChildInstByName(&iName));

	return true;
}


void APMrover_base::sendHeartbeat(void)
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
