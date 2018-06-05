#include "APcopter_thrust.h"

namespace kai
{

APcopter_thrust::APcopter_thrust()
{
	m_pAP = NULL;
	m_pSB = NULL;
	m_pCmd = NULL;
	m_pTarget = -1;
	m_pTargetMin = 0;
	m_pTargetMax = DBL_MAX;
	m_dCollision = 2.0;
	m_pwmLow = 1000;
	m_pwmMid = 1500;
	m_pwmHigh = 2000;

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pYaw = NULL;
	m_pAlt = NULL;

	m_rc.chan1_raw = m_pwmMid;
	m_rc.chan2_raw = m_pwmMid;
	m_rc.chan3_raw = m_pwmMid;
	m_rc.chan4_raw = m_pwmMid;
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

	pK->v("targetXmin", &m_pTargetMin.x);
	pK->v("targetYmin", &m_pTargetMin.y);
	pK->v("targetZmin", &m_pTargetMin.z);

	pK->v("targetXmax", &m_pTargetMax.x);
	pK->v("targetYmax", &m_pTargetMax.y);
	pK->v("targetZmax", &m_pTargetMax.z);

	pK->v("dCollisionX", &m_dCollision.x);
	pK->v("dCollisionY", &m_dCollision.y);
	pK->v("dCollisionZ", &m_dCollision.y);

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

	cmd();
	uint64_t tNow = getTimeUsec();
	vDouble3* pPos = &m_pSB->m_pos;
	double o;

	//TODO: collision avoid

	//Pitch = Y axis
	uint16_t pwmF = m_pwmLow;
	uint16_t pwmB = m_pwmLow;
	if(pPos->y > 0 && m_pTarget.y > 0)
	{
		o = m_pPitch->update(pPos->y, m_pTarget.y);
		if(o > 0)
			pwmF += (uint16_t)abs(o);
		else
			pwmB += (uint16_t)abs(o);
	}

	//Roll = X axis
	uint16_t pwmL = m_pwmLow;
	uint16_t pwmR = m_pwmLow;
	if(pPos->x > 0 && m_pTarget.x > 0)
	{
		o = m_pPitch->update(pPos->x, m_pTarget.x);
		if(o > 0)
			pwmR += (uint16_t)abs(o);
		else
			pwmL += (uint16_t)abs(o);
	}

	//Alt = Z axis
	uint16_t pwmA = m_pwmMid;
	if(pPos->z > 0 && m_pTarget.z > 0)
	{
		pwmA += (uint16_t)m_pAlt->update(pPos->z, m_pTarget.z);
	}

	//Mavlink rc override to rc 1-4 is always alive, thrust only controls thrust fans
	_Mavlink* pM = m_pAP->m_pMavlink;
	mavlink_rc_channels_override_t* pRC = &pM->m_msg.rc_channels_override;

	m_rc.chan1_raw = pRC->chan1_raw;
	m_rc.chan2_raw = pRC->chan2_raw;
	m_rc.chan3_raw = pwmA;
	m_rc.chan4_raw = pRC->chan3_raw;
	m_rc.chan5_raw = pwmF;
	m_rc.chan6_raw = pwmR;
	m_rc.chan7_raw = pwmB;
	m_rc.chan8_raw = pwmL;

	if(!isActive() || pM->m_msg.heartbeat.custom_mode != 2) //2:ALT_HOLD
	{
		pM->setCmdRoute(MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE, true);
		return;
	}

	pM->setCmdRoute(MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE, false);
	IF_(tNow - pM->m_msg.time_stamps.rc_channels_override > 30000);

	m_pAP->m_pMavlink->rcChannelsOverride(m_rc);
}

void APcopter_thrust::cmd(void)
{
	char buf[32];
	string str;

	NULL_(m_pCmd);
	IF_(m_pCmd->read((uint8_t*) &buf, 32) <= 0);
	str = buf;

	std::string::size_type iDiv = str.find(' ');
	if(iDiv == std::string::npos)
	{
		iDiv = str.length()+1;
	}

	string cmd = str.substr(0, iDiv-1);
	string v = str.substr(iDiv+1, str.length()-iDiv);

	if(cmd=="state")
	{
		m_pAM->transit(&v);
	}
	else if(cmd=="x")
	{
		m_pTarget.x = constrain(atof(v.c_str()), m_pTargetMin.x, m_pTargetMax.x);
	}
	else if(cmd=="y")
	{
		m_pTarget.y = constrain(atof(v.c_str()), m_pTargetMin.y, m_pTargetMax.y);
	}
	else if(cmd=="z")
	{
		m_pTarget.z = constrain(atof(v.c_str()), m_pTargetMin.z, m_pTargetMax.z);
	}
	else
	{
		str = "Invalid Cmd: " + cmd;
		m_pCmd->writeLine((uint8_t*)str.c_str(), str.length());
	}
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
	vDouble3* pPos = &m_pSB->m_pos;

	string msg;

	//Roll = X axis
	msg = "targetX=" + f2str(m_pTarget.x) +
		  " | X=" + f2str(pPos->x) +
		  " | pwmL=" + i2str((int)m_rc.chan8_raw) +
		  " | pwmR=" + i2str((int)m_rc.chan6_raw);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	//Pitch = Y axis
	msg = "targetY=" + f2str(m_pTarget.y) +
		  " | Y=" + f2str(pPos->y) +
		  " | pwmF=" + i2str((int)m_rc.chan5_raw)  +
		  " | pwmB=" + i2str((int)m_rc.chan7_raw);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	//Alt = Z axis
	msg = "targetZ=" + f2str(m_pTarget.z) +
		  " | Z=" + f2str(pPos->z) +
		  " | pwm=" + i2str((int)m_rc.chan3_raw);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
