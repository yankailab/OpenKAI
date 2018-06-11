#include "APcopter_thrust.h"

namespace kai
{

APcopter_thrust::APcopter_thrust()
{
	m_pAP = NULL;
	m_pSB = NULL;
	m_pCmd = NULL;
	m_pMavAP = NULL;
	m_pMavGCS = NULL;

	m_pTarget = -1;
	m_targetMin = 0;
	m_targetMax = DBL_MAX;
	m_dCollision = 2.0;
	m_pwmLow = 1000;
	m_pwmMid = 1500;
	m_pwmHigh = 2000;
	m_rcTimeOut = USEC_1SEC;

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

	KISSm(pK,rcTimeOut);

	KISSm(pK,pwmLow);
	KISSm(pK,pwmMid);
	KISSm(pK,pwmHigh);

	pK->v("targetX", &m_pTarget.x);
	pK->v("targetY", &m_pTarget.y);
	pK->v("targetZ", &m_pTarget.z);

	KISSm(pK,targetMin);
	KISSm(pK,targetMax);
	KISSm(pK,dCollision);

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
	pK->v("_WebSocket", &iName);
	m_pCmd = (_WebSocket*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pCmd, iName + ": not found");

	iName = "";
	pK->v("_Mavlink_GCS", &iName);
	m_pMavGCS = (_Mavlink*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pMavGCS, iName + ": not found");

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
	m_pMavAP = m_pAP->m_pMavlink;
	NULL_(m_pMavAP);
	NULL_(m_pMavGCS);
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
	if(pPos->y > 0 && m_pTarget.y > m_targetMin)
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
	if(pPos->x > 0 && m_pTarget.x > m_targetMin)
	{
		o = m_pRoll->update(pPos->x, m_pTarget.x);
		if(o > 0)
			pwmR += (uint16_t)abs(o);
		else
			pwmL += (uint16_t)abs(o);
	}

	//Alt = Z axis
	uint16_t pwmA = m_pwmMid;
	if(pPos->z > 0 && m_pTarget.z > m_targetMin)
	{
		pwmA += (uint16_t)m_pAlt->update(pPos->z, m_pTarget.z);
	}

	//Mavlink rc override to rc 1-4 is always alive, thrust only controls thrust fans
	mavlink_rc_channels_override_t* pRC = &m_pMavGCS->m_msg.rc_channels_override;
	m_rc.chan1_raw = pRC->chan1_raw;
	m_rc.chan2_raw = pRC->chan2_raw;
	m_rc.chan3_raw = pwmA;
	m_rc.chan4_raw = pRC->chan4_raw;
	m_rc.chan5_raw = pwmF;
	m_rc.chan6_raw = pwmR;
	m_rc.chan7_raw = pwmB;
	m_rc.chan8_raw = pwmL;

	if(abs(((int)pRC->chan3_raw) - ((int)m_pwmMid)) > 100)
	{
		m_rc.chan3_raw = pRC->chan3_raw;
	}

	if(!isActive() || m_pMavAP->m_msg.heartbeat.custom_mode != 2) //2:ALT_HOLD
	{
		m_pMavGCS->setCmdRoute(MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE, true);
		if(m_pMavAP->m_msg.heartbeat.custom_mode != 2)
		{
			LOG_F("OFF: Not ALT_HOLD: " + i2str((int)m_pMavAP->m_msg.heartbeat.custom_mode));
		}
		else
		{
			LOG_F("OFF");
		}
		return;
	}

	IF_l(tNow - m_pMavGCS->m_msg.time_stamps.rc_channels_override > m_rcTimeOut, "OFF: NO RC from GCS");

	m_pMavGCS->setCmdRoute(MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE, false);
	m_pMavAP->rcChannelsOverride(m_rc);

	LOG_I("ON: Mavlink RC Override ON");
}

void APcopter_thrust::cmd(void)
{
	NULL_(m_pCmd);

	char buf[N_IO_BUF];
	int nB = m_pCmd->read((uint8_t*) &buf, N_IO_BUF);
	IF_(nB <= 0);
	buf[nB]=0;
	string str = buf;
	string cmd;
	double v;

	std::string::size_type iDiv = str.find(' ');
	if(iDiv == std::string::npos)
	{
		cmd = str;
		v = -1.0;
	}
	else
	{
		cmd = str.substr(0, iDiv);
		IF_(++iDiv >= str.length());
		v = atof(str.substr(iDiv, str.length()-iDiv).c_str());
	}

	if(cmd=="on")
	{
		string strOn = "CC_ON";
		m_pAM->transit(&strOn);
		str = "State: " + strOn;
	}
	else if(cmd=="off")
	{
		string strOff = "CC_STANDBY";
		m_pAM->transit(&strOff);
		str = "State: " + strOff;
	}
	else if(cmd=="set")
	{
		m_pTarget = m_pSB->m_pos;
		if(m_pTarget.x < m_targetMin || m_pTarget.x > m_targetMax)m_pTarget.x = -1.0;
		if(m_pTarget.y < m_targetMin || m_pTarget.y > m_targetMax)m_pTarget.y = -1.0;
		if(m_pTarget.z < m_targetMin || m_pTarget.z > m_targetMax)m_pTarget.z = -1.0;

		str = "Set target: X=" + f2str(m_pTarget.x) +
				", Y=" + f2str(m_pTarget.y) +
				", Z=" + f2str(m_pTarget.z);
	}
	else if(cmd=="x")
	{
		if(v < m_targetMin || v > m_targetMax)v = -1.0;

		m_pTarget.x = v;
		str = "Set target: X=" + f2str(m_pTarget.x) +
				", Y=" + f2str(m_pTarget.y) +
				", Z=" + f2str(m_pTarget.z);
	}
	else if(cmd=="y")
	{
		if(v < m_targetMin || v > m_targetMax)v = -1.0;

		m_pTarget.y = v;
		str = "Set target: X=" + f2str(m_pTarget.x) +
				", Y=" + f2str(m_pTarget.y) +
				", Z=" + f2str(m_pTarget.z);
	}
	else if(cmd=="z")
	{
		if(v < m_targetMin || v > m_targetMax)v = -1.0;

		m_pTarget.z = v;
		str = "Set target: X=" + f2str(m_pTarget.x) +
				", Y=" + f2str(m_pTarget.y) +
				", Z=" + f2str(m_pTarget.z);
	}
	else
	{
		str = "Invalid Cmd: " + cmd;
	}

	m_pCmd->write((uint8_t*)str.c_str(), str.length(), WS_MODE_TXT);
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
