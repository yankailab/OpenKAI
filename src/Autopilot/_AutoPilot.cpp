#include "../Utility/util.h"
#include "_AutoPilot.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	_ThreadBase();

	m_pRecvMsg = NULL;

	m_pOD = NULL;
	m_pFD = NULL;
	m_pVI = NULL;
	m_pMavlink = NULL;
	m_pMavlink = NULL;
	m_pROITracker = NULL;
	m_pMarkerDetector = NULL;

}

_AutoPilot::~_AutoPilot()
{
}

bool _AutoPilot::init(JSON* pJson, string pilotName)
{
	if (!pJson)
		return false;

	string cName;
	CONTROL_PID cPID;
	RC_CHANNEL RC;

	cName = "ROLL_";

	CHECK_ERROR(pJson->getVal(cName + "P" + pilotName, &cPID.m_P));
	CHECK_ERROR(pJson->getVal(cName + "I" + pilotName, &cPID.m_I));
	CHECK_ERROR(pJson->getVal(cName + "IMAX" + pilotName, &cPID.m_Imax));
	CHECK_ERROR(pJson->getVal(cName + "D" + pilotName, &cPID.m_D));
	CHECK_ERROR(pJson->getVal(cName + "dT" + pilotName, &cPID.m_dT));

	CHECK_ERROR(pJson->getVal(cName + "PWM_LOW" + pilotName, &RC.m_pwmLow));
	CHECK_ERROR(pJson->getVal(cName + "PWM_HIGH" + pilotName, &RC.m_pwmHigh));
	CHECK_ERROR(
			pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
	CHECK_ERROR(pJson->getVal(cName + "PWM_IDX" + pilotName, &RC.m_idx));

	m_roll.m_pid = cPID;
	m_roll.m_RC = RC;

	cName = "PITCH_";

	CHECK_ERROR(pJson->getVal(cName + "P" + pilotName, &cPID.m_P));
	CHECK_ERROR(pJson->getVal(cName + "I" + pilotName, &cPID.m_I));
	CHECK_ERROR(pJson->getVal(cName + "IMAX" + pilotName, &cPID.m_Imax));
	CHECK_ERROR(pJson->getVal(cName + "D" + pilotName, &cPID.m_D));
	CHECK_ERROR(pJson->getVal(cName + "dT" + pilotName, &cPID.m_dT));

	CHECK_ERROR(pJson->getVal(cName + "PWM_LOW" + pilotName, &RC.m_pwmLow));
	CHECK_ERROR(pJson->getVal(cName + "PWM_HIGH" + pilotName, &RC.m_pwmHigh));
	CHECK_ERROR(
			pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
	CHECK_ERROR(pJson->getVal(cName + "PWM_IDX" + pilotName, &RC.m_idx));

	m_pitch.m_pid = cPID;
	m_pitch.m_RC = RC;

	cName = "ALT_";

	CHECK_ERROR(pJson->getVal(cName + "P" + pilotName, &cPID.m_P));
	CHECK_ERROR(pJson->getVal(cName + "I" + pilotName, &cPID.m_I));
	CHECK_ERROR(pJson->getVal(cName + "IMAX" + pilotName, &cPID.m_Imax));
	CHECK_ERROR(pJson->getVal(cName + "D" + pilotName, &cPID.m_D));
	CHECK_ERROR(pJson->getVal(cName + "dT" + pilotName, &cPID.m_dT));

	CHECK_ERROR(pJson->getVal(cName + "PWM_LOW" + pilotName, &RC.m_pwmLow));
	CHECK_ERROR(pJson->getVal(cName + "PWM_HIGH" + pilotName, &RC.m_pwmHigh));
	CHECK_ERROR(
			pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
	CHECK_ERROR(pJson->getVal(cName + "PWM_IDX" + pilotName, &RC.m_idx));

	m_alt.m_pid = cPID;
	m_alt.m_RC = RC;

	cName = "YAW_";

	CHECK_ERROR(pJson->getVal(cName + "P" + pilotName, &cPID.m_P));
	CHECK_ERROR(pJson->getVal(cName + "I" + pilotName, &cPID.m_I));
	CHECK_ERROR(pJson->getVal(cName + "IMAX" + pilotName, &cPID.m_Imax));
	CHECK_ERROR(pJson->getVal(cName + "D" + pilotName, &cPID.m_D));
	CHECK_ERROR(pJson->getVal(cName + "dT" + pilotName, &cPID.m_dT));

	CHECK_ERROR(pJson->getVal(cName + "PWM_LOW" + pilotName, &RC.m_pwmLow));
	CHECK_ERROR(pJson->getVal(cName + "PWM_HIGH" + pilotName, &RC.m_pwmHigh));
	CHECK_ERROR(
			pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
	CHECK_ERROR(pJson->getVal(cName + "PWM_IDX" + pilotName, &RC.m_idx));

	m_yaw.m_pid = cPID;
	m_yaw.m_RC = RC;

	//For visual position locking
	CHECK_ERROR(pJson->getVal("ROLL_TARGET_POS" + pilotName, &m_roll.m_targetPos));
	CHECK_ERROR(pJson->getVal("PITCH_TARGET_POS" + pilotName, &m_pitch.m_targetPos));

	resetAllControl();

	this->setTargetFPS(30.0);

	m_landingTarget.m_angleX = 0;
	m_landingTarget.m_angleY = 0;

	return true;
}

bool _AutoPilot::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in AutoPilot::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "AutoPilot.start()";

	return true;
}

void _AutoPilot::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		camROILock();

		camMarkerLock();

		if (m_pVI)
		{
			m_pVI->readMessages();
		}

		this->autoFPSto();

	}

}

void _AutoPilot::resetAllControl(void)
{
	m_roll.m_errOld = 0;
	m_roll.m_err = 0;
	m_roll.m_errInteg = 0;
	m_roll.m_pos = 0;
//	m_roll.m_targetPos = 0;

	m_alt.m_errOld = 0;
	m_alt.m_err = 0;
	m_alt.m_errInteg = 0;
	m_alt.m_pos = 0;
//	m_alt.m_targetPos = 0;

	m_pitch.m_errOld = 0;
	m_pitch.m_err = 0;
	m_pitch.m_errInteg = 0;
	m_pitch.m_pos = 0;
//	m_pitch.m_targetPos = 0;

	m_yaw.m_errOld = 0;
	m_yaw.m_err = 0;
	m_yaw.m_errInteg = 0;
	m_yaw.m_pos = 0;
//	m_yaw.m_targetPos = 0;

//RC
	m_RC[m_roll.m_RC.m_idx] = m_roll.m_RC.m_pwmCenter;
	m_RC[m_pitch.m_RC.m_idx] = m_pitch.m_RC.m_pwmCenter;
	m_RC[m_yaw.m_RC.m_idx] = m_yaw.m_RC.m_pwmCenter;
	m_RC[m_alt.m_RC.m_idx] = m_alt.m_RC.m_pwmCenter;
}

void _AutoPilot::camROILock(void)
{
	if (m_pVI == NULL)return;
	if (m_pROITracker == NULL)return;

	if (m_pROITracker->m_bTracking == false)
	{
		resetAllControl();
		m_pVI->rc_overide(NUM_RC_CHANNEL, m_RC);
		return;
	}

	double posRoll;
	double posPitch;
	double ovDTime;

	ovDTime = (1.0 / m_pROITracker->m_dTime) * 1000; //ms
	posRoll = m_roll.m_pos;
	posPitch = m_pitch.m_pos;

	m_roll.m_pos = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width * 0.5;
	m_pitch.m_pos = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height * 0.5;

	//Update current position with trajectory estimation
	posRoll = m_roll.m_pos
			+ (m_roll.m_pos - posRoll) * m_roll.m_pid.m_dT * ovDTime;
	posPitch = m_pitch.m_pos
			+ (m_pitch.m_pos - posPitch) * m_pitch.m_pid.m_dT * ovDTime;

	//Roll
	m_roll.m_errOld = m_roll.m_err;
	m_roll.m_err = m_roll.m_targetPos - posRoll;
	m_roll.m_errInteg += m_roll.m_err;
	m_roll.m_RC.m_pwm = m_roll.m_RC.m_pwmCenter
			+ m_roll.m_pid.m_P * m_roll.m_err
			+ m_roll.m_pid.m_D * (m_roll.m_err - m_roll.m_errOld) * ovDTime
			+ confineVal(m_roll.m_pid.m_I * m_roll.m_errInteg,
					m_roll.m_pid.m_Imax, -m_roll.m_pid.m_Imax);
	m_RC[m_roll.m_RC.m_idx] = constrain(m_roll.m_RC.m_pwm, m_roll.m_RC.m_pwmLow,
			m_roll.m_RC.m_pwmHigh);

	//Pitch
	m_pitch.m_errOld = m_pitch.m_err;
	m_pitch.m_err = m_pitch.m_targetPos - posPitch;
	m_pitch.m_errInteg += m_pitch.m_err;
	m_pitch.m_RC.m_pwm = m_pitch.m_RC.m_pwmCenter
			+ m_pitch.m_pid.m_P * m_pitch.m_err
			+ m_pitch.m_pid.m_D * (m_pitch.m_err - m_pitch.m_errOld) * ovDTime
			+ confineVal(m_pitch.m_pid.m_I * m_pitch.m_errInteg,
					m_pitch.m_pid.m_Imax, -m_pitch.m_pid.m_Imax);
	m_RC[m_pitch.m_RC.m_idx] = constrain(m_pitch.m_RC.m_pwm,
			m_pitch.m_RC.m_pwmLow, m_pitch.m_RC.m_pwmHigh);

	//Mavlink
	m_pVI->rc_overide(NUM_RC_CHANNEL, m_RC);
	return;

}

void _AutoPilot::camMarkerLock(void)
{
	CamInput* pCamInput;
	fVector2 markerCenter;

	if (m_pMavlink == NULL)return;
	if (m_pMarkerDetector == NULL)return;

	//Return if not detected
	if(!m_pMarkerDetector->getCircleCenter(&markerCenter))
	{
		return;
	}

	pCamInput = m_pMarkerDetector->m_pCamStream->getCameraInput();

	//Change position to angles
	m_landingTarget.m_angleX = ((markerCenter.m_x - pCamInput->m_centerH)/pCamInput->m_width) * pCamInput->m_angleH * DEG_RADIAN;
	m_landingTarget.m_angleY = ((markerCenter.m_y - pCamInput->m_centerV)/pCamInput->m_height) * pCamInput->m_angleV * DEG_RADIAN;

	//Send Mavlink command
	m_pMavlink->landing_target(MAV_DATA_STREAM_ALL,MAV_FRAME_BODY_NED, m_landingTarget.m_angleX, m_landingTarget.m_angleY, 0,0,0);


}

void _AutoPilot::setVehicleInterface(_VehicleInterface* pVehicle)
{
	if (!pVehicle)
		return;

	m_pVI = pVehicle;
}

void _AutoPilot::setMavlinkInterface(_MavlinkInterface* pMavlink)
{
	if (!pMavlink)
		return;

	m_pMavlink = pMavlink;
}

void _AutoPilot::remoteMavlinkMsg(MESSAGE* pMsg)
{
	int i;
	unsigned int val;

	switch (pMsg->m_pBuf[2])
	//Command
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
//		m_remoteSystem.m_mode = val;
		break;

	default:

		break;
	}
}

int* _AutoPilot::getPWMOutput(void)
{
	return m_RC;
}

}
