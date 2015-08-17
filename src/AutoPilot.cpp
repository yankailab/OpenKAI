#include "AutoPilot.h"


AutoPilot::AutoPilot()
{
}


AutoPilot::~AutoPilot()
{
}

bool AutoPilot::init()
{
/*
	m_roll = { 0, 0, 0, 0, 0, 0, ROLL_P, ROLL_I, ROLL_IMAX, ROLL_D, PWM_CENTER };
	m_pitch = { 0, 0, 0, 0, 0, 0, PITCH_P, PITCH_I, PITCH_IMAX, PITCH_D, PWM_CENTER };
	m_alt = { 0, 0, 0, 0, 0, 0, ALT_P, ALT_I, ALT_IMAX, ALT_D, PWM_CENTER };

	m_rollFar = { ROLL_P, ROLL_I, ROLL_D, Z_FAR_LIM};
	m_rollNear = { ROLL_P, ROLL_I, ROLL_D, Z_NEAR_LIM };
	m_altFar = { ALT_P, ALT_I, ALT_D, Z_FAR_LIM };
	m_altNear = { ALT_P, ALT_I, ALT_D, Z_NEAR_LIM };
	m_pitchFar = { PITCH_P, PITCH_I, PITCH_D, Z_FAR_LIM };
	m_pitchNear = { PITCH_P, PITCH_I, PITCH_D, Z_NEAR_LIM };
	m_yawFar = { YAW_P, YAW_I, YAW_D, Z_FAR_LIM };
	m_yawNear = { YAW_P, YAW_I, YAW_D, Z_NEAR_LIM };
*/
	m_pCV = NULL;
	m_pMavlink = NULL;

	for (int i = 0; i < NUM_RC_CHANNEL; i++)
	{
		m_RC[i] = PWM_LOW;
	}

	m_hostSystem.m_mode = OPE_BOOT;
	m_remoteSystem.m_mode = OPE_BOOT;

	m_dT = 0.0;
	m_bAutoCalcDelay = true;

	m_iFlowFrame = 0;
	m_resetFlowFrame = NUM_FLOW_FRAME_RESET;
	m_resetFactor = FLOW_RESET_FACTOR;
	m_flowTotal.m_x = 0;
	m_flowTotal.m_y = 0;
	m_flowTotal.m_z = 0;
	m_flowTotal.m_w = 0;
//	m_flowTotal = fVector4{ 0, 0, 0, 0 };

	return true;
}

void AutoPilot::setCameraVision(CameraVision* pCV)
{
	if (pCV == NULL)return;

	m_pCV = pCV;
}

void AutoPilot::setMavlink(MavlinkInterface* pMavlink)
{
	m_pMavlink = pMavlink;
}

void AutoPilot::setRC(int channelID, int pwmCenter, int pwmFrom, int pwmTo)
{
/*	m_RC[channelID].m_pwmCenter = pwmCenter;
	m_RC[channelID].m_pwmFrom = pwmFrom;
	m_RC[channelID].m_pwmTo = pwmTo;
	m_RC[channelID].m_current = pwmCenter;
	*/
}

void AutoPilot::setTargetPosCV(fVector3 pos)
{
	m_roll.m_targetPos = pos.m_x;
	m_pitch.m_targetPos = pos.m_z;
	m_alt.m_targetPos = pos.m_y;

}

fVector3 AutoPilot::getTargetPosCV(void)
{
	fVector3 v;
	v.m_x = m_roll.m_targetPos;
	v.m_y = m_alt.m_targetPos;
	v.m_z = m_pitch.m_targetPos;
//	return fVector3{ m_roll.m_targetPos, m_alt.m_targetPos, m_pitch.m_targetPos };

	return v;
}

void AutoPilot::markerLock(void)
{
	double zRatio;
	double cvSize;

	m_lockLevel = m_pCV->getObjLockLevel();

	if (m_lockLevel >= LOCK_LEVEL_POS)
	{
		//Update current position with trajectory estimation
		m_pCV->getObjPosition(&m_cvPos);

		m_roll.m_pos = m_cvPos.m_x + (m_cvPos.m_x - m_cvPosOld.m_x)*m_dT;
		m_alt.m_pos = m_cvPos.m_y + (m_cvPos.m_y - m_cvPosOld.m_y)*m_dT;
		m_cvPosOld.m_x = m_cvPos.m_x;
		m_cvPosOld.m_y = m_cvPos.m_y;

		//If size is detected, use it
		if (m_lockLevel >= LOCK_LEVEL_SIZE)
		{
			m_pitch.m_pos = m_cvPos.m_z + (m_cvPos.m_z - m_cvPosOld.m_z)*m_dT;
			m_cvPosOld.m_z = m_cvPos.m_z;
			cvSize = confineVal(m_pitch.m_pos, m_pitchNear.m_Z, m_pitchFar.m_Z);

			zRatio = m_pitchNear.m_Z - m_pitchFar.m_Z;
			if (zRatio!=0.0)
			{
				zRatio = abs((m_pitchNear.m_Z - cvSize) / zRatio);
			}
			else
			{
				zRatio = 0.0;
			}
		}
		else
		{
			zRatio = 0.5;
		}

		//Update PID based on Z position

		//Roll = X axis
		m_roll.m_P = m_rollNear.m_P + (m_rollFar.m_P - m_rollNear.m_P)*zRatio;
		m_roll.m_I = m_rollNear.m_I + (m_rollFar.m_I - m_rollNear.m_I)*zRatio;
		m_roll.m_D = m_rollNear.m_D + (m_rollFar.m_D - m_rollNear.m_D)*zRatio;
		
		m_roll.m_cvErrOld = m_roll.m_cvErr;
		m_roll.m_cvErr = m_roll.m_targetPos - m_roll.m_pos;
		m_roll.m_cvErrInteg = confineVal(m_roll.m_cvErrInteg + m_roll.m_cvErr, ERROR_RANGE,-ERROR_RANGE);
		m_roll.m_pwm = PWM_CENTER +
			m_roll.m_P * m_roll.m_cvErr
			+ m_roll.m_D * (m_roll.m_cvErr - m_roll.m_cvErrOld)
			+ confineVal(m_roll.m_I * m_roll.m_cvErrInteg, m_roll.m_Imax, -m_roll.m_Imax);

		m_RC[RC_ROLL] = constrain(m_roll.m_pwm, PWM_LOW, PWM_HIGH);

		//Alt = Y axis
		m_alt.m_P = m_altNear.m_P + (m_altFar.m_P - m_altNear.m_P)*zRatio;
		m_alt.m_I = m_altNear.m_I + (m_altFar.m_I - m_altNear.m_I)*zRatio;
		m_alt.m_D = m_altNear.m_D + (m_altFar.m_D - m_altNear.m_D)*zRatio;

		m_alt.m_cvErrOld = m_alt.m_cvErr;
		m_alt.m_cvErr = m_alt.m_targetPos - m_alt.m_pos;
		m_alt.m_cvErrInteg = confineVal(m_alt.m_cvErrInteg + m_alt.m_cvErr, ERROR_RANGE, -ERROR_RANGE);
		m_alt.m_pwm = PWM_CENTER +
			m_alt.m_P * m_alt.m_cvErr
			+ m_alt.m_D * (m_alt.m_cvErr - m_alt.m_cvErrOld)
			+ confineVal(m_alt.m_I * m_alt.m_cvErrInteg, m_alt.m_Imax, -m_alt.m_Imax);

		m_RC[RC_THROTTLE] = constrain(m_alt.m_pwm, PWM_LOW, PWM_HIGH);

		//Pitch = Z axis (Depth)
		if (m_lockLevel >= LOCK_LEVEL_SIZE)
		{
			m_pitch.m_P = m_pitchNear.m_P + (m_pitchFar.m_P - m_pitchNear.m_P)*zRatio;
			m_pitch.m_I = m_pitchNear.m_I + (m_pitchFar.m_I - m_pitchNear.m_I)*zRatio;
			m_pitch.m_D = m_pitchNear.m_D + (m_pitchFar.m_D - m_pitchNear.m_D)*zRatio;

			m_pitch.m_cvErrOld = m_pitch.m_cvErr;
			m_pitch.m_cvErr = m_pitch.m_targetPos - m_pitch.m_pos;
			m_pitch.m_cvErrInteg = confineVal(m_pitch.m_cvErrInteg + m_pitch.m_cvErr, ERROR_RANGE, -ERROR_RANGE);
			m_pitch.m_pwm = PWM_CENTER +
				m_pitch.m_P * m_pitch.m_cvErr
				+ m_pitch.m_D * (m_pitch.m_cvErr - m_pitch.m_cvErrOld)
				+ confineVal(m_pitch.m_I * m_pitch.m_cvErrInteg, m_pitch.m_Imax, -m_pitch.m_Imax);

			m_RC[RC_PITCH] = constrain(m_pitch.m_pwm, PWM_LOW, PWM_HIGH);
		}
		else
		{
			m_pitch.m_cvErrOld = 0;
			m_pitch.m_cvErr = 0;
			m_pitch.m_cvErrInteg = 0;
			m_RC[RC_PITCH] = PWM_CENTER;
		}

		//Yaw axis
		if (m_lockLevel >= LOCK_LEVEL_ATT)
		{
			m_yaw.m_P = m_yawNear.m_P + (m_yawFar.m_P - m_yawNear.m_P)*zRatio;
			m_yaw.m_I = m_yawNear.m_I + (m_yawFar.m_I - m_yawNear.m_I)*zRatio;
			m_yaw.m_D = m_yawNear.m_D + (m_yawFar.m_D - m_yawNear.m_D)*zRatio;

			m_pCV->getObjAttitude(&m_cvAtt);
			m_yaw.m_pos = m_cvAtt.m_z;
			m_yaw.m_targetPos = 1.0;

			m_yaw.m_cvErrOld = m_yaw.m_cvErr;
			m_yaw.m_cvErr = m_yaw.m_targetPos - m_yaw.m_pos;
			m_yaw.m_cvErrInteg = confineVal(m_yaw.m_cvErrInteg + m_yaw.m_cvErr, ERROR_RANGE, -ERROR_RANGE);
			m_yaw.m_pwm = PWM_CENTER +
				m_yaw.m_P * m_yaw.m_cvErr
				+ m_yaw.m_D * (m_yaw.m_cvErr - m_yaw.m_cvErrOld)
				+ m_yaw.m_I * m_yaw.m_cvErrInteg;

			m_RC[RC_YAW] = constrain(m_yaw.m_pwm, PWM_LOW, PWM_HIGH);
		}
		else
		{
			m_yaw.m_cvErrOld = 0;
			m_yaw.m_cvErr = 0;
			m_yaw.m_cvErrInteg = 0;
			m_RC[RC_YAW] = PWM_CENTER;
		}


	}
	else 
	{
		//Mark not detected
		m_roll.m_cvErrOld = 0;
		m_roll.m_cvErr = 0;
		m_roll.m_cvErrInteg = 0;

		m_alt.m_cvErrOld = 0;
		m_alt.m_cvErr = 0;
		m_alt.m_cvErrInteg = 0;

		m_pitch.m_cvErrOld = 0;
		m_pitch.m_cvErr = 0;
		m_pitch.m_cvErrInteg = 0;

		m_yaw.m_cvErrOld = 0;
		m_yaw.m_cvErr = 0;
		m_yaw.m_cvErrInteg = 0;

		//RC
		m_RC[RC_ROLL] = PWM_CENTER;
		m_RC[RC_THROTTLE] = PWM_CENTER;
		m_RC[RC_PITCH] = PWM_CENTER;
		m_RC[RC_YAW] = PWM_CENTER;
	}

	//Mavlink
	m_pMavlink->rc_overide(NUM_RC_CHANNEL, m_RC);

	return;

	//Decode mavlink message from device
	if (m_pMavlink->readMessages())
	{
		m_pRecvMsg = &m_pMavlink->m_recvMsg;
		remoteMavlinkMsg(m_pRecvMsg);

		//reset the flag to accept new messages
		m_pRecvMsg->m_cmd = 0;
	}

	if (m_remoteSystem.m_mode != m_hostSystem.m_mode)
	{
		if (m_hostSystem.m_mode == OPE_BOOT)
		{
			m_hostSystem.m_mode = m_remoteSystem.m_mode;
		}
		else
		{
			m_pMavlink->controlMode(m_hostSystem.m_mode);
		}
	}

}

void AutoPilot::flowLock(void)
{
	fVector4 vFlow;

	//To avoid accumulated error
	if (++m_iFlowFrame >= m_resetFlowFrame)
	{
		m_flowTotal.m_x *= m_resetFactor;
		m_flowTotal.m_y *= m_resetFactor;
		m_flowTotal.m_z *= m_resetFactor;
		m_flowTotal.m_w *= m_resetFactor;
		m_iFlowFrame = 0;
	}

	m_pCV->getOpticalFlowVelocity(&vFlow);
	m_flowTotal.m_x += vFlow.m_x * (1.0 + m_dT);
	m_flowTotal.m_y += vFlow.m_y * (1.0 + m_dT);
	m_flowTotal.m_z += vFlow.m_z * (1.0 + m_dT);
	m_flowTotal.m_w += vFlow.m_w * (1.0 + m_dT);

	//Roll = X axis
	m_roll.m_P = m_rollNear.m_P;
	m_roll.m_I = m_rollNear.m_I;
	m_roll.m_D = m_rollNear.m_D;

	m_roll.m_cvErrOld = m_roll.m_cvErr;
	m_roll.m_cvErr = m_flowTotal.m_x;
	m_roll.m_cvErrInteg += m_roll.m_cvErr;
	m_roll.m_pwm = PWM_CENTER +
		m_roll.m_P * m_roll.m_cvErr
		+ m_roll.m_D * (m_roll.m_cvErr - m_roll.m_cvErrOld)
		+ confineVal(m_roll.m_I * m_roll.m_cvErrInteg, m_roll.m_Imax, -m_roll.m_Imax);

	m_RC[RC_ROLL] = constrain(m_roll.m_pwm, PWM_LOW, PWM_HIGH);

	//Pitch = Y axis
	m_pitch.m_P = m_pitchNear.m_P;
	m_pitch.m_I = m_pitchNear.m_I;
	m_pitch.m_D = m_pitchNear.m_D;

	m_pitch.m_cvErrOld = m_pitch.m_cvErr;
	m_pitch.m_cvErr = m_flowTotal.m_y;
	m_pitch.m_cvErrInteg += m_pitch.m_cvErr;
	m_pitch.m_pwm = PWM_CENTER +
		m_pitch.m_P * m_pitch.m_cvErr
		+ m_pitch.m_D * (m_pitch.m_cvErr - m_pitch.m_cvErrOld)
		+ confineVal(m_pitch.m_I * m_pitch.m_cvErrInteg, m_pitch.m_Imax, -m_pitch.m_Imax);

	m_RC[RC_PITCH] = constrain(m_pitch.m_pwm, PWM_LOW, PWM_HIGH);

/*
	//Alt = Z axis
	m_alt.m_P = m_altNear.m_P;
	m_alt.m_I = m_altNear.m_I;
	m_alt.m_D = m_altNear.m_D;

	m_alt.m_cvErrOld = m_alt.m_cvErr;
	m_alt.m_cvErr = m_flowTotal.m_z;
	m_alt.m_cvErrInteg += m_alt.m_cvErr;
	m_alt.m_pwm = PWM_CENTER +
		m_alt.m_P * m_alt.m_cvErr
		+ m_alt.m_D * (m_alt.m_cvErr - m_alt.m_cvErrOld)
		+ confineVal(m_alt.m_I * m_alt.m_cvErrInteg, m_alt.m_Imax, -m_alt.m_Imax);

	m_RC[RC_THROTTLE] = constrain(m_alt.m_pwm, PWM_LOW, PWM_HIGH);

	//Yaw axis
	m_yaw.m_P = m_yawNear.m_P;
	m_yaw.m_I = m_yawNear.m_I;
	m_yaw.m_D = m_yawNear.m_D;

	m_pCV->getObjAttitude(&m_cvAtt);
	m_yaw.m_pos = m_cvAtt.m_z;
	m_yaw.m_targetPos = 1.0;

	m_yaw.m_cvErrOld = m_yaw.m_cvErr;
	m_yaw.m_cvErr = m_flowTotal.m_w;
	m_yaw.m_cvErrInteg += m_yaw.m_cvErr;
	m_yaw.m_pwm = PWM_CENTER +
		m_yaw.m_P * m_yaw.m_cvErr
		+ m_yaw.m_D * (m_yaw.m_cvErr - m_yaw.m_cvErrOld)
		+ m_yaw.m_I * m_yaw.m_cvErrInteg;

	m_RC[RC_YAW] = constrain(m_yaw.m_pwm, PWM_LOW, PWM_HIGH);
*/

	m_RC[RC_THROTTLE] = PWM_CENTER;
	m_RC[RC_YAW] = PWM_CENTER;

	//Mavlink
	m_pMavlink->rc_overide(NUM_RC_CHANNEL, m_RC);

}


void AutoPilot::remoteMavlinkMsg(MESSAGE* pMsg)
{
	int i;
	unsigned char numChannel;
	unsigned int val;

	switch (pMsg->m_pBuf[2]) //Command
	{
/*	case CMD_RC_UPDATE:
		if (*m_pOprMode != OPE_RC_BRIDGE)break;

		numChannel = m_hostCMD.m_pBuf[3];
		if (m_numChannel > RC_CHANNEL_NUM)
		{
			numChannel = RC_CHANNEL_NUM;
		}

		for (i = 0; i<numChannel; i++)
		{
			val = (int)makeWord(m_hostCMD.m_pBuf[4 + i * 2 + 1], m_hostCMD.m_pBuf[4 + i * 2]);
			m_channelValues[i] = val;
		}

		break;
*/
	case CMD_OPERATE_MODE:
		val = pMsg->m_pBuf[3];
		m_remoteSystem.m_mode = val;
		break;

	default:

		break;
	}
}

int* AutoPilot::getPWMOutput(void)
{
	return m_RC;
}

PID_SETTING AutoPilot::getRollFarPID(void)
{
	return m_rollFar;
}

PID_SETTING AutoPilot::getRollNearPID(void)
{
	return m_rollNear;
}

PID_SETTING AutoPilot::getAltFarPID(void)
{
	return m_altFar;
}

PID_SETTING AutoPilot::getAltNearPID(void)
{
	return m_altNear;
}

PID_SETTING AutoPilot::getPitchFarPID(void)
{
	return m_pitchFar;
}

PID_SETTING AutoPilot::getPitchNearPID(void)
{
	return m_pitchNear;
}

PID_SETTING AutoPilot::getYawFarPID(void)
{
	return m_yawFar;
}

PID_SETTING AutoPilot::getYawNearPID(void)
{
	return m_yawNear;
}


fVector3 AutoPilot::getRollPID(void)
{
	fVector3 v;
	v.m_x = m_roll.m_P;
	v.m_y = m_roll.m_I;
	v.m_z = m_roll.m_D;

	return v;
//	return fVector3{ m_roll.m_P, m_roll.m_I, m_roll.m_D};
}

fVector3 AutoPilot::getAltPID(void)
{
	fVector3 v;
	v.m_x = m_alt.m_P;
	v.m_y = m_alt.m_I;
	v.m_z = m_alt.m_D;

	return v;
//	return fVector3{ m_alt.m_P, m_alt.m_I, m_alt.m_D };
}

fVector3 AutoPilot::getPitchPID(void)
{
	fVector3 v;
	v.m_x = m_pitch.m_P;
	v.m_y = m_pitch.m_I;
	v.m_z = m_pitch.m_D;

	return v;
//	return fVector3{ m_pitch.m_P, m_pitch.m_I, m_pitch.m_D };
}

fVector3 AutoPilot::getYawPID(void)
{
	fVector3 v;
	v.m_x = m_yaw.m_P;
	v.m_y = m_yaw.m_I;
	v.m_z = m_yaw.m_D;

	return v;
//	return fVector3{ m_yaw.m_P, m_yaw.m_I, m_yaw.m_D };
}

void AutoPilot::setRollFarPID(PID_SETTING pid)
{
	m_rollFar = pid;
}

void AutoPilot::setRollNearPID(PID_SETTING pid)
{
	m_rollNear = pid;
}

void AutoPilot::setAltFarPID(PID_SETTING pid)
{
	m_altFar = pid;
}

void AutoPilot::setAltNearPID(PID_SETTING pid)
{
	m_altNear = pid;
}

void AutoPilot::setPitchFarPID(PID_SETTING pid)
{
	m_pitchFar = pid;
}

void AutoPilot::setPitchNearPID(PID_SETTING pid)
{
	m_pitchNear = pid;
}

void AutoPilot::setYawFarPID(PID_SETTING pid)
{
	m_yawFar = pid;
}

void AutoPilot::setYawNearPID(PID_SETTING pid)
{
	m_yawNear = pid;
}

void AutoPilot::setDelayTime(double dT)
{
	if (dT < 0.0)
	{
		m_bAutoCalcDelay = true;
		return;
	}

	m_bAutoCalcDelay = false;
	m_dT = dT;
}

double AutoPilot::getDelayTime(void)
{
	return m_dT;
}

CONTROL_AXIS* AutoPilot::getRollAxis(void)
{
	return &m_roll;
}

CONTROL_AXIS* AutoPilot::getPitchAxis(void)
{
	return &m_pitch;
}

CONTROL_AXIS* AutoPilot::getAltAxis(void)
{
	return &m_alt;
}


/*
void AutoPilot::setRollPID(fVector3 pid)
{
}

void AutoPilot::setAltPID(fVector3 pid)
{

}

void AutoPilot::setPitchPID(fVector3 pid)
{

}
*/
