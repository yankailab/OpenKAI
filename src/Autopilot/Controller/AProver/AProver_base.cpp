#include "../../Controller/AProver/AProver_base.h"

namespace kai
{

AProver_base::AProver_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	m_thrust = 0;
	m_steer = 0;

	m_pidThrust.reset();
	m_pidSteer.reset();
	m_ctrlThrust.reset();
	m_ctrlSteer.reset();
}

AProver_base::~AProver_base()
{
}

bool AProver_base::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	Kiss* pCC;
	AProver_PID cPID;

	pCC = pK->o("steer");
	IF_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	m_pidSteer = cPID;

	pCC = pK->o("thrust");
	IF_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_INFO(pCC->v("I", &cPID.m_I));
	F_INFO(pCC->v("Imax", &cPID.m_Imax));
	F_INFO(pCC->v("D", &cPID.m_D));
	F_INFO(pCC->v("dT", &cPID.m_dT));
	m_pidThrust = cPID;

	//init controls
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	return true;
}

bool AProver_base::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";

	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInstByName(&iName));

	return true;
}


void AProver_base::sendHeartbeat(void)
{
	NULL_(m_pMavlink);

	//Sending Heartbeat at 1Hz
	uint64_t timeNow = get_time_usec();
	IF_(timeNow - m_lastHeartbeat < USEC_1SEC);

	m_pMavlink->sendHeartbeat();
	m_lastHeartbeat = timeNow;

	LOG(INFO)<<"APMrover HEARTBEAT:"<<++m_iHeartbeat;
}

void AProver_base::sendSteerThrust(void)
{
	NULL_(m_pMavlink);
	m_pMavlink->commandLongDoSetPositionYawThrust(m_steer, m_thrust);

}


}
