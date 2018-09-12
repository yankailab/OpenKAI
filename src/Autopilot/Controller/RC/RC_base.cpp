#include "../../Controller/RC/RC_base.h"

namespace kai
{

RC_base::RC_base()
{
	m_pRC = NULL;
}

RC_base::~RC_base()
{
}

bool RC_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	RC_PID cPID;
	RC_CHANNEL RC;
	Kiss* pCC;

	pCC = pK->o("roll");
	IF_F(pCC->empty());

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

	m_pPWM[m_rcRoll.m_iCh] = m_rcRoll.m_pwmN;

	pCC = pK->o("pitch");
	IF_F(pCC->empty());

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

	m_pPWM[m_rcPitch.m_iCh] = m_rcPitch.m_pwmN;

	pCC = pK->o("alt");
	IF_F(pCC->empty());

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

	m_pPWM[m_rcAlt.m_iCh] = m_rcAlt.m_pwmN;

	pCC = pK->o("yaw");
	IF_F(pCC->empty());

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

	m_pPWM[m_rcYaw.m_iCh] = m_rcYaw.m_pwmN;

	//link
	string iName = "";
	F_INFO(pK->v("_RC", &iName));
	m_pRC = (_RC*) (pK->root()->getChildInstByName(iName));

	return true;
}

}
