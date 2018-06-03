#include "APcopter_thrust.h"

namespace kai
{

APcopter_thrust::APcopter_thrust()
{
	m_pAP = NULL;
	m_pSB = NULL;
	m_pCmd = NULL;
	m_pTarget.x = -1;
	m_pTarget.y = -1;
	m_pTarget.z = -1;
	m_pwmLow = 1000;
	m_pwmMid = 1500;
	m_pwmHigh = 2000;

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pYaw = NULL;
	m_pAlt = NULL;

	m_rc.chan1_raw = UINT16_MAX;
	m_rc.chan2_raw = UINT16_MAX;
	m_rc.chan3_raw = UINT16_MAX;
	m_rc.chan4_raw = UINT16_MAX;
	m_rc.chan5_raw = m_pwmLow;
	m_rc.chan6_raw = m_pwmLow;
	m_rc.chan7_raw = m_pwmLow;
	m_rc.chan8_raw = m_pwmLow;

}

APcopter_thrust::~APcopter_thrust()
{
}

bool APcopter_thrust::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,pwmLow);
	KISSm(pK,pwmMid);
	KISSm(pK,pwmHigh);

	pK->v("targetX", &m_pTarget.x);
	pK->v("targetY", &m_pTarget.y);
	pK->v("targetZ", &m_pTarget.z);

	return true;
}

bool APcopter_thrust::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_SlamBase", &iName);
	m_pSB = (_SlamBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pSB, iName + ": not found");

	iName = "";
	pK->v("_IOBase", &iName);
	m_pCmd = (_IOBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pCmd, iName + ": not found");

	iName = "";
	pK->v("PIDroll", &iName);
	m_pRoll = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pRoll, iName + ": not found");

	iName = "";
	pK->v("PIDpitch", &iName);
	m_pPitch = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pPitch, iName + ": not found");

	iName = "";
	pK->v("PIDyaw", &iName);
	m_pYaw = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pYaw, iName + ": not found");

	iName = "";
	pK->v("PIDalt", &iName);
	m_pAlt = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pAlt, iName + ": not found");

	return true;
}

void APcopter_thrust::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	NULL_(m_pSB);
	NULL_(m_pRoll);
	NULL_(m_pPitch);
	NULL_(m_pYaw);
	NULL_(m_pAlt);

	//mode change
	if(m_pCmd)
	{

	}

	//update thrust output
	vDouble3* pV = &m_pSB->m_pos;
	double o;

	//Pitch = Y axis
	uint16_t pwmF = m_pwmLow;
	uint16_t pwmB = m_pwmLow;
	if(pV->y > 0 && m_pTarget.y > 0)
	{
		o = m_pPitch->update(pV->y, m_pTarget.y);
		if(o > 0)
			pwmF += (uint16_t)abs(o);
		else
			pwmB += (uint16_t)abs(o);
	}

	//Roll = X axis
	uint16_t pwmL = m_pwmLow;
	uint16_t pwmR = m_pwmLow;
	if(pV->x > 0 && m_pTarget.x > 0)
	{
		o = m_pPitch->update(pV->x, m_pTarget.x);
		if(o > 0)
			pwmR += (uint16_t)abs(o);
		else
			pwmL += (uint16_t)abs(o);
	}

	//Alt = Z axis
	uint16_t pwmA = UINT16_MAX;
	if(pV->z > 0 && m_pTarget.z > 0)
	{
		pwmA += (uint16_t)m_pAlt->update(pV->z, m_pTarget.z);
	}

	//Mavlink rc override to rc 1-4 is always alive, thrust only controls thrust fans
	m_rc.chan1_raw = UINT16_MAX;
	m_rc.chan2_raw = UINT16_MAX;
	m_rc.chan3_raw = pwmA;
	m_rc.chan4_raw = UINT16_MAX;
	m_rc.chan5_raw = pwmF;
	m_rc.chan6_raw = pwmR;
	m_rc.chan7_raw = pwmB;
	m_rc.chan8_raw = pwmL;
	m_pAP->m_pMavlink->rcChannelsOverride(m_rc);

}

bool APcopter_thrust::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool APcopter_thrust::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));

	NULL_F(m_pAP->m_pMavlink);
	NULL_F(m_pSB);
	vDouble3* pV = &m_pSB->m_pos;

	string msg;
	string pwmA;
	string pwmB;

	//Roll = X axis
	pwmA = i2str(m_rc.chan8_raw);
	pwmB = i2str(m_rc.chan6_raw);
	msg = "targetX=" + f2str(m_pTarget.x) + " | X=" + f2str(pV->x) + " | pwmL=" + pwmA  + " | pwmR=" + pwmB ;

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	//Pitch = Y axis
	pwmA = i2str(m_rc.chan5_raw);
	pwmB = i2str(m_rc.chan7_raw);
	msg = "targetY=" + f2str(m_pTarget.y) + " | Y=" + f2str(pV->y) + " | pwmF=" + pwmA  + " | pwmB=" + pwmB ;

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	//Alt = Z axis
	if(m_rc.chan3_raw == UINT16_MAX)
		pwmA = "-";
	else
		pwmA = i2str(m_rc.chan3_raw);
	msg = "targetZ=" + f2str(m_pTarget.z) + " | Z=" + f2str(pV->z) + " | pwm=" + pwmA;

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
