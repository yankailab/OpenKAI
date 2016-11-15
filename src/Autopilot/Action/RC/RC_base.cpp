#include "RC_base.h"

namespace kai
{

RC_base::RC_base()
{
}

RC_base::~RC_base()
{
}

bool RC_base::init(Config* pConfig)
{
	CHECK_F(this->ActionBase::init(pConfig)==false);
	pConfig->m_pInst = this;


	int i;
	RC_PID cPID;
	RC_CHANNEL RC;
	Config* pCC;

	pCC = pConfig->o("roll");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidRoll = cPID;

	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));
	m_rcRoll = RC;

	pCC = pConfig->o("pitch");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidPitch = cPID;

	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));
	m_rcPitch = RC;

	pCC = pConfig->o("alt");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidAlt = cPID;

	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));
	m_rcAlt = RC;

	pCC = pConfig->o("yaw");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	m_pidYaw = cPID;

	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));
	m_rcYaw = RC;

	return true;
}

}
