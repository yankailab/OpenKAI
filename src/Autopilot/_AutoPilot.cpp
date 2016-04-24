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
	m_pROITracker = NULL;
	m_bThreadON = false;
	m_threadID = 0;

}

_AutoPilot::~_AutoPilot()
{
}

bool _AutoPilot::init(JSON* pJson, string pilotName)
{
	if (!pJson)return false;

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
	CHECK_ERROR(pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
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
	CHECK_ERROR(pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
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
	CHECK_ERROR(pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
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
	CHECK_ERROR(pJson->getVal(cName + "PWM_CENTER" + pilotName, &RC.m_pwmCenter));
	CHECK_ERROR(pJson->getVal(cName + "PWM_IDX" + pilotName, &RC.m_idx));

	m_yaw.m_pid = cPID;
	m_yaw.m_RC = RC;


	//For visual position locking
	CHECK_ERROR(pJson->getVal("ROLL_TARGET_POS" + pilotName, &m_roll.m_targetPos));
	CHECK_ERROR(pJson->getVal("PITCH_TARGET_POS" + pilotName, &m_pitch.m_targetPos));

	resetAllControl();


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
	m_tSleep = TRD_INTERVAL_AUTOPILOT_USEC;

	while (m_bThreadON)
	{
		this->updateTime();

		camROILock();

		if (m_pVI)
		{
			m_pVI->readMessages();
		}

		if (m_tSleep > 0)
		{
			//sleepThread can be woke up by this->wakeupThread()
			this->sleepThread(0, m_tSleep);
		}
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
	if(m_pVI==NULL)return;
	if(m_pROITracker==NULL)return;

	if (m_pROITracker->m_bTracking==false)
	{
		resetAllControl();
		m_pVI->rc_overide(NUM_RC_CHANNEL, m_RC);
		return;
	}

	double posRoll;
	double posPitch;

	posRoll = m_roll.m_pos;
	posPitch = m_pitch.m_pos;

	m_roll.m_pos = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width*0.5;
	m_pitch.m_pos = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height*0.5;

	//Update current position with trajectory estimation
	posRoll = m_roll.m_pos + (m_roll.m_pos - posRoll) * m_roll.m_pid.m_dT;
	posPitch = m_pitch.m_pos + (m_pitch.m_pos - posPitch) * m_pitch.m_pid.m_dT;

	//Roll
	m_roll.m_errOld = m_roll.m_err;
	m_roll.m_err = m_roll.m_targetPos - posRoll;
	m_roll.m_errInteg += m_roll.m_err;
	m_roll.m_RC.m_pwm =
			m_roll.m_RC.m_pwmCenter
			+ m_roll.m_pid.m_P * m_roll.m_err
			+ m_roll.m_pid.m_D * (m_roll.m_err - m_roll.m_errOld)
			+ confineVal(m_roll.m_pid.m_I * m_roll.m_errInteg, m_roll.m_pid.m_Imax, -m_roll.m_pid.m_Imax);
	m_RC[m_roll.m_RC.m_idx] = constrain(m_roll.m_RC.m_pwm, m_roll.m_RC.m_pwmLow, m_roll.m_RC.m_pwmHigh);

	//Pitch
	m_pitch.m_errOld = m_pitch.m_err;
	m_pitch.m_err = m_pitch.m_targetPos - posPitch;
	m_pitch.m_errInteg += m_pitch.m_err;
	m_pitch.m_RC.m_pwm =
			m_pitch.m_RC.m_pwmCenter
			+ m_pitch.m_pid.m_P * m_pitch.m_err
			+ m_pitch.m_pid.m_D * (m_pitch.m_err - m_pitch.m_errOld)
			+ confineVal(m_pitch.m_pid.m_I * m_pitch.m_errInteg, m_pitch.m_pid.m_Imax, -m_pitch.m_pid.m_Imax);
	m_RC[m_pitch.m_RC.m_idx] = constrain(m_pitch.m_RC.m_pwm, m_pitch.m_RC.m_pwmLow, m_pitch.m_RC.m_pwmHigh);


	//Mavlink
	m_pVI->rc_overide(NUM_RC_CHANNEL, m_RC);
	return;

}

void _AutoPilot::setVehicleInterface(_VehicleInterface* pVehicle)
{
	if (!pVehicle)
		return;

	m_pVI = pVehicle;
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

/*
 #include "../Utility/util.h"
 #include "_AutoPilot.h"

 namespace kai
 {

 _AutoPilot::_AutoPilot()
 {
 _ThreadBase();

 m_dT = 0.0;
 m_lockLevel = LOCK_LEVEL_NONE;
 m_hostSystem.m_mode = OPE_BOOT;
 m_remoteSystem.m_mode = OPE_BOOT;

 m_bAutoCalcDelay = true;
 m_iFlowFrame = 0;

 m_flowTotal.m_x = 0;
 m_flowTotal.m_y = 0;
 m_flowTotal.m_z = 0;
 m_flowTotal.m_w = 0;

 m_pRecvMsg = NULL;
 m_resetFactor = 0.0;
 m_resetFlowFrame = 0;

 m_numCamStream = 0;
 m_pOD = NULL;
 m_pFD = NULL;
 m_pVI = NULL;
 m_bThreadON = false;
 m_threadID = 0;

 int i;
 for(i=0;i<NUM_CAM_STREAM;i++)
 {
 m_pCamStream[i].m_frameID = 0;
 m_pCamStream[i].m_pCam = NULL;
 }

 }

 bool _AutoPilot::start(void)
 {
 //Start thread
 m_bThreadON = true;
 int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
 if (retCode != 0)
 {
 LOG(ERROR) << "Return code: "<< retCode << " in AutoPilot::start().pthread_create()";
 m_bThreadON = false;
 return false;
 }

 LOG(INFO) << "AutoPilot.start()";

 return true;
 }

 void _AutoPilot::update(void)
 {
 CAMERA_STREAM* pCam;
 CamFrame* pFrame;

 m_tSleep = TRD_INTERVAL_AUTOPILOT_USEC;

 while (m_bThreadON)
 {
 this->updateTime();

 pCam = &m_pCamStream[CAM_FRONT];
 pFrame = pCam->m_pCam->getFrame();

 //TODO
 //		if(pFrame->isNewThan(pCam->m_frameID))
 //		{
 //New frame arrived
 //			pCam->m_frameID = pFrame->m_frameID;

 //			m_pVI->readMessages();

 //Action
 //			markerLock(pCam->m_pCam->m_pMarkerDetect);

 //		}


 m_pVI->readMessages();

 if(m_tSleep>0)
 {
 //sleepThread can be woke up by this->wakeupThread()
 this->sleepThread(0,m_tSleep);
 }
 }

 }


 bool _AutoPilot::setCamStream(_CamStream* pCamStream, int camPosition)
 {
 CHECK_ERROR(pCamStream);
 if(camPosition > NUM_CAM_STREAM)return false;

 m_pCamStream[camPosition].m_pCam = pCamStream;

 return true;
 }



 void _AutoPilot::markerLock(_MarkerDetector* pCMD)
 {
 double zRatio;
 double cvSize;

 m_lockLevel = pCMD->getObjLockLevel();

 if (m_lockLevel >= LOCK_LEVEL_POS)
 {
 //Update current position with trajectory estimation
 pCMD->getObjPosition(&m_cvPos);

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
 m_roll.m_cvErrInteg += m_roll.m_cvErr;
 m_roll.m_pwm = m_roll.m_pwmCenter +
 m_roll.m_P * m_roll.m_cvErr
 + m_roll.m_D * (m_roll.m_cvErr - m_roll.m_cvErrOld)
 + confineVal(m_roll.m_I * m_roll.m_cvErrInteg, m_roll.m_Imax, -m_roll.m_Imax);

 m_RC[m_roll.m_RCChannel] = constrain(m_roll.m_pwm, m_roll.m_pwmLow, m_roll.m_pwmHigh);

 //Alt = Y axis
 m_alt.m_P = m_altNear.m_P + (m_altFar.m_P - m_altNear.m_P)*zRatio;
 m_alt.m_I = m_altNear.m_I + (m_altFar.m_I - m_altNear.m_I)*zRatio;
 m_alt.m_D = m_altNear.m_D + (m_altFar.m_D - m_altNear.m_D)*zRatio;

 m_alt.m_cvErrOld = m_alt.m_cvErr;
 m_alt.m_cvErr = m_alt.m_targetPos - m_alt.m_pos;
 m_alt.m_cvErrInteg += m_alt.m_cvErr;
 m_alt.m_pwm = m_alt.m_pwmCenter +
 m_alt.m_P * m_alt.m_cvErr
 + m_alt.m_D * (m_alt.m_cvErr - m_alt.m_cvErrOld)
 + confineVal(m_alt.m_I * m_alt.m_cvErrInteg, m_alt.m_Imax, -m_alt.m_Imax);

 m_RC[m_alt.m_RCChannel] = constrain(m_alt.m_pwm, m_alt.m_pwmLow, m_alt.m_pwmHigh);

 //Pitch = Z axis (Depth)
 if (m_lockLevel >= LOCK_LEVEL_SIZE)
 {
 m_pitch.m_P = m_pitchNear.m_P + (m_pitchFar.m_P - m_pitchNear.m_P)*zRatio;
 m_pitch.m_I = m_pitchNear.m_I + (m_pitchFar.m_I - m_pitchNear.m_I)*zRatio;
 m_pitch.m_D = m_pitchNear.m_D + (m_pitchFar.m_D - m_pitchNear.m_D)*zRatio;

 m_pitch.m_cvErrOld = m_pitch.m_cvErr;
 m_pitch.m_cvErr = m_pitch.m_targetPos - m_pitch.m_pos;
 m_pitch.m_cvErrInteg += m_pitch.m_cvErr;
 m_pitch.m_pwm = m_pitch.m_pwmCenter +
 m_pitch.m_P * m_pitch.m_cvErr
 + m_pitch.m_D * (m_pitch.m_cvErr - m_pitch.m_cvErrOld)
 + confineVal(m_pitch.m_I * m_pitch.m_cvErrInteg, m_pitch.m_Imax, -m_pitch.m_Imax);

 m_RC[m_pitch.m_RCChannel] = constrain(m_pitch.m_pwm, m_pitch.m_pwmLow, m_pitch.m_pwmHigh);
 }
 else
 {
 m_pitch.m_cvErrOld = 0;
 m_pitch.m_cvErr = 0;
 m_pitch.m_cvErrInteg = 0;
 m_RC[m_pitch.m_RCChannel] = m_pitch.m_pwmCenter;
 }

 //Yaw axis
 if (m_lockLevel >= LOCK_LEVEL_ATT)
 {
 m_yaw.m_P = m_yawNear.m_P + (m_yawFar.m_P - m_yawNear.m_P)*zRatio;
 m_yaw.m_I = m_yawNear.m_I + (m_yawFar.m_I - m_yawNear.m_I)*zRatio;
 m_yaw.m_D = m_yawNear.m_D + (m_yawFar.m_D - m_yawNear.m_D)*zRatio;

 pCMD->getObjAttitude(&m_cvAtt);
 m_yaw.m_pos = m_cvAtt.m_z;
 m_yaw.m_targetPos = 1.0;

 m_yaw.m_cvErrOld = m_yaw.m_cvErr;
 m_yaw.m_cvErr = m_yaw.m_targetPos - m_yaw.m_pos;
 m_yaw.m_cvErrInteg += m_yaw.m_cvErr;
 m_yaw.m_pwm = m_yaw.m_pwmCenter +
 m_yaw.m_P * m_yaw.m_cvErr
 + m_yaw.m_D * (m_yaw.m_cvErr - m_yaw.m_cvErrOld)
 + m_yaw.m_I * m_yaw.m_cvErrInteg;

 m_RC[m_yaw.m_RCChannel] = constrain(m_yaw.m_pwm, m_yaw.m_pwmLow, m_yaw.m_pwmHigh);
 }
 else
 {
 m_yaw.m_cvErrOld = 0;
 m_yaw.m_cvErr = 0;
 m_yaw.m_cvErrInteg = 0;
 m_RC[m_yaw.m_RCChannel] = m_yaw.m_pwmCenter;
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
 m_RC[m_roll.m_RCChannel] = m_roll.m_pwmCenter;
 m_RC[m_alt.m_RCChannel] = m_alt.m_pwmCenter;
 m_RC[m_pitch.m_RCChannel] = m_pitch.m_pwmCenter;
 m_RC[m_yaw.m_RCChannel] = m_yaw.m_pwmCenter;
 }

 //Mavlink
 m_pVI->rc_overide(NUM_RC_CHANNEL, m_RC);
 return;

 }

 void _AutoPilot::flowLock(_MarkerDetector* pCMD)
 {
 //	fVector4 vFlow;
 //
 //	//To avoid accumulated error
 //	if (++m_iFlowFrame >= m_resetFlowFrame)
 //	{
 //		m_flowTotal.m_x *= m_resetFactor;
 //		m_flowTotal.m_y *= m_resetFactor;
 //		m_flowTotal.m_z *= m_resetFactor;
 //		m_flowTotal.m_w *= m_resetFactor;
 //		m_iFlowFrame = 0;
 //	}
 //
 //	pCMD->getOpticalFlowVelocity(&vFlow);
 //	m_flowTotal.m_x += vFlow.m_x * (1.0 + m_dT);
 //	m_flowTotal.m_y += vFlow.m_y * (1.0 + m_dT);
 //	m_flowTotal.m_z += vFlow.m_z * (1.0 + m_dT);
 //	m_flowTotal.m_w += vFlow.m_w * (1.0 + m_dT);
 //
 //	//Roll = X axis
 //	m_roll.m_P = m_rollNear.m_P;
 //	m_roll.m_I = m_rollNear.m_I;
 //	m_roll.m_D = m_rollNear.m_D;
 //
 //	m_roll.m_cvErrOld = m_roll.m_cvErr;
 //	m_roll.m_cvErr = m_flowTotal.m_x;
 //	m_roll.m_cvErrInteg += m_roll.m_cvErr;
 //	m_roll.m_pwm = m_roll.m_pwmCenter +
 //		m_roll.m_P * m_roll.m_cvErr
 //		+ m_roll.m_D * (m_roll.m_cvErr - m_roll.m_cvErrOld)
 //		+ confineVal(m_roll.m_I * m_roll.m_cvErrInteg, m_roll.m_Imax, -m_roll.m_Imax);
 //
 //	m_RC[m_roll.m_RCChannel] = constrain(m_roll.m_pwm, m_roll.m_pwmLow, m_roll.m_pwmHigh);
 //
 //	//Pitch = Y axis
 //	m_pitch.m_P = m_pitchNear.m_P;
 //	m_pitch.m_I = m_pitchNear.m_I;
 //	m_pitch.m_D = m_pitchNear.m_D;
 //
 //	m_pitch.m_cvErrOld = m_pitch.m_cvErr;
 //	m_pitch.m_cvErr = m_flowTotal.m_y;
 //	m_pitch.m_cvErrInteg += m_pitch.m_cvErr;
 //	m_pitch.m_pwm = m_pitch.m_pwmCenter +
 //		m_pitch.m_P * m_pitch.m_cvErr
 //		+ m_pitch.m_D * (m_pitch.m_cvErr - m_pitch.m_cvErrOld)
 //		+ confineVal(m_pitch.m_I * m_pitch.m_cvErrInteg, m_pitch.m_Imax, -m_pitch.m_Imax);
 //
 //	m_RC[m_pitch.m_RCChannel] = constrain(m_pitch.m_pwm, m_pitch.m_pwmLow, m_pitch.m_pwmHigh);
 //



 //
 //	//Alt = Z axis
 //	m_alt.m_P = m_altNear.m_P;
 //	m_alt.m_I = m_altNear.m_I;
 //	m_alt.m_D = m_altNear.m_D;
 //
 //	m_alt.m_cvErrOld = m_alt.m_cvErr;
 //	m_alt.m_cvErr = m_flowTotal.m_z;
 //	m_alt.m_cvErrInteg += m_alt.m_cvErr;
 //	m_alt.m_pwm = PWM_CENTER +
 //		m_alt.m_P * m_alt.m_cvErr
 //		+ m_alt.m_D * (m_alt.m_cvErr - m_alt.m_cvErrOld)
 //		+ confineVal(m_alt.m_I * m_alt.m_cvErrInteg, m_alt.m_Imax, -m_alt.m_Imax);
 //
 //	m_RC[RC_THROTTLE] = constrain(m_alt.m_pwm, PWM_LOW, PWM_HIGH);
 //
 //	//Yaw axis
 //	m_yaw.m_P = m_yawNear.m_P;
 //	m_yaw.m_I = m_yawNear.m_I;
 //	m_yaw.m_D = m_yawNear.m_D;
 //
 //	pCMD->getObjAttitude(&m_cvAtt);
 //	m_yaw.m_pos = m_cvAtt.m_z;
 //	m_yaw.m_targetPos = 1.0;
 //
 //	m_yaw.m_cvErrOld = m_yaw.m_cvErr;
 //	m_yaw.m_cvErr = m_flowTotal.m_w;
 //	m_yaw.m_cvErrInteg += m_yaw.m_cvErr;
 //	m_yaw.m_pwm = PWM_CENTER +
 //		m_yaw.m_P * m_yaw.m_cvErr
 //		+ m_yaw.m_D * (m_yaw.m_cvErr - m_yaw.m_cvErrOld)
 //		+ m_yaw.m_I * m_yaw.m_cvErrInteg;
 //
 //	m_RC[RC_YAW] = constrain(m_yaw.m_pwm, PWM_LOW, PWM_HIGH);
 //

 m_RC[m_alt.m_RCChannel] = m_alt.m_pwmCenter;
 m_RC[m_yaw.m_RCChannel] = m_yaw.m_pwmCenter;

 //Mavlink
 m_pVI->rc_overide(NUM_RC_CHANNEL, m_RC);

 }


 void _AutoPilot::setVehicleInterface(_VehicleInterface* pVehicle)
 {
 if(!pVehicle)return;

 m_pVI = pVehicle;
 }

 void _AutoPilot::remoteMavlinkMsg(MESSAGE* pMsg)
 {
 int i;
 unsigned int val;

 switch (pMsg->m_pBuf[2]) //Command
 {
 //	case CMD_RC_UPDATE:
 //		if (*m_pOprMode != OPE_RC_BRIDGE)break;
 //
 //		numChannel = m_hostCMD.m_pBuf[3];
 //		if (m_numChannel > RC_CHANNEL_NUM)
 //		{
 //			numChannel = RC_CHANNEL_NUM;
 //		}
 //
 //		for (i = 0; i<numChannel; i++)
 //		{
 //			val = (int)makeWord(m_hostCMD.m_pBuf[4 + i * 2 + 1], m_hostCMD.m_pBuf[4 + i * 2]);
 //			m_channelValues[i] = val;
 //		}
 //
 //		break;

 case CMD_OPERATE_MODE:
 val = pMsg->m_pBuf[3];
 m_remoteSystem.m_mode = val;
 break;

 default:

 break;
 }
 }


 }

 */
