#include "../Utility/util.h"
#include "_AutoPilot.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	_ThreadBase();

	m_pFD = NULL;
	m_pVI = NULL;
	m_pMavlink = NULL;
	m_pMavlink = NULL;
	m_pROITracker = NULL;
	m_pMD = NULL;

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

	pilotName = "_"+pilotName;

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
	CHECK_ERROR(
			pJson->getVal("ROLL_TARGET_POS" + pilotName, &m_roll.m_targetPos));
	CHECK_ERROR(
			pJson->getVal("PITCH_TARGET_POS" + pilotName, &m_pitch.m_targetPos));

	resetAllControl();

	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("AUTOPILOT_MAIN_FPS", &FPS));
	this->setTargetFPS(FPS);

	m_landingTarget.m_angleX = 0;
	m_landingTarget.m_angleY = 0;
	m_landingTarget.m_orientX = 1.0;
	m_landingTarget.m_orientY = 1.0;
	m_landingTarget.m_ROIstarted = 0;
	m_landingTarget.m_ROItimeLimit = 0;

	CHECK_INFO(
			pJson->getVal("MARKER_LANDING_ORIENTATION_X",
					&m_landingTarget.m_orientX));
	CHECK_INFO(
			pJson->getVal("MARKER_LANDING_ORIENTATION_Y",
					&m_landingTarget.m_orientY));
	CHECK_INFO(
			pJson->getVal("MARKER_LANDING_ROI_TIME_LIMIT",
					&m_landingTarget.m_ROItimeLimit));

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

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

//		camROILock();

		landingTarget();

		sendHeartbeat();

//		if (m_pVI)
//		{
//			m_pVI->readMessages();
//		}

		this->autoFPSto();

	}

}

void _AutoPilot::sendHeartbeat(void)
{
	if (m_pMavlink == NULL)
		return;

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
	if (m_pVI == NULL)
		return;
	if (m_pROITracker == NULL)
		return;

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

void _AutoPilot::landingTarget(void)
{
	Camera* pCamInput;
	fVector3 markerCenter;

	if (m_pMD == NULL)
		return;
	if (m_pROITracker == NULL)
		return;

	if (m_pMD->getCircleCenter(&markerCenter))
	{
		//Update Tracker
		Rect roi;
		roi.x = markerCenter.m_x - markerCenter.m_z;
		roi.y = markerCenter.m_y - markerCenter.m_z;
		roi.width = markerCenter.m_z * 2;
		roi.height = markerCenter.m_z * 2;
		m_pROITracker->setROI(roi);
		m_pROITracker->tracking(true);

		m_landingTarget.m_ROIstarted = 0;
	}
	else
	{
		if (!m_pROITracker->m_bTracking)
			return;

		uint64_t timeNow = get_time_usec();

		//ROI tracker is already in use
		if (m_landingTarget.m_ROIstarted > 0)
		{
			//Disable sending landing target if the marker is not seen for a certain time
			if (timeNow - m_landingTarget.m_ROIstarted
					> m_landingTarget.m_ROItimeLimit)
			{
				m_pROITracker->tracking(false);
				return;
			}
		}
		else
		{
			//Start to use ROI tracker if marker is not detected
			m_landingTarget.m_ROIstarted = timeNow;
		}

		markerCenter.m_x = m_pROITracker->m_ROI.x
				+ m_pROITracker->m_ROI.width * 0.5;
		markerCenter.m_y = m_pROITracker->m_ROI.y
				+ m_pROITracker->m_ROI.height * 0.5;
	}

	pCamInput = m_pMD->m_pCamStream->getCameraInput();

	//Change position to angles
	m_landingTarget.m_angleX = ((markerCenter.m_x - pCamInput->m_centerH)
			/ pCamInput->m_width) * pCamInput->m_angleH * DEG_RADIAN
			* m_landingTarget.m_orientX;
	m_landingTarget.m_angleY = ((markerCenter.m_y - pCamInput->m_centerV)
			/ pCamInput->m_height) * pCamInput->m_angleV * DEG_RADIAN
			* m_landingTarget.m_orientY;

	if (m_pMavlink == NULL)
		return;

	//Send Mavlink command
	m_pMavlink->landing_target(MAV_DATA_STREAM_ALL, MAV_FRAME_BODY_NED,
			m_landingTarget.m_angleX, m_landingTarget.m_angleY, 0, 0, 0);

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

int* _AutoPilot::getPWMOutput(void)
{
	return m_RC;
}

bool _AutoPilot::draw(Frame* pFrame, iVector4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	if(m_pMD)
	{
		Mat* pMat = pFrame->getCMat();
		fVector3 markerCenter;
		markerCenter.m_x = 0;
		markerCenter.m_y = 0;
		markerCenter.m_z = 0;

		if (m_pMD->getCircleCenter(&markerCenter))
		{
			circle(*pMat, Point(markerCenter.m_x, markerCenter.m_y),
					markerCenter.m_z, Scalar(0, 0, 255), 5);
		}
		else if (m_pROITracker->m_bTracking)
		{
			rectangle(*pMat, Point(m_pROITracker->m_ROI.x, m_pROITracker->m_ROI.y),
					Point(m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width,
							m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height),
					Scalar(0, 0, 255), 3);
		}

		putText(*pMat, "Marker FPS: " + f2str(getFrameRate()),
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 1);

		pTextPos->m_y += pTextPos->m_w;

		putText(*pMat,
				"Landing_Target: (" + f2str(m_landingTarget.m_angleX) + " , "
						+ f2str(m_landingTarget.m_angleY) + ")",
				cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 1);

		pTextPos->m_y += pTextPos->m_w;
	}

	return true;
}

}
