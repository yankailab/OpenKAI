#include "_AutoPilot.h"

#include "../Utility/util.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	_ThreadBase();

	m_pFD = NULL;
	m_pVI = NULL;
	m_pMavlink = NULL;
	m_pROITracker = NULL;
	m_pMD = NULL;
	m_pAT = NULL;
	m_pAM = NULL;

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;
	m_numATagsLandingTarget = 0;

}

_AutoPilot::~_AutoPilot()
{
}

bool _AutoPilot::init(Config* pConfig, string name)
{
	if (this->_ThreadBase::init(pConfig,name)==false)
		return false;

	Config* pC = pConfig->o(name);

	CONTROL_PID cPID;
	RC_CHANNEL RC;
	int i;
	Config* pCC;

	pCC = pC->o("roll");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmLow));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmHigh));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmCenter));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_idx));

	m_roll.m_pid = cPID;
	m_roll.m_RC = RC;

	pCC = pC->o("pitch");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmLow));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmHigh));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmCenter));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_idx));

	m_pitch.m_pid = cPID;
	m_pitch.m_RC = RC;

	pCC = pC->o("alt");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmLow));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmHigh));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmCenter));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_idx));

	m_alt.m_pid = cPID;
	m_alt.m_RC = RC;

	pCC = pC->o("yaw");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmLow));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmHigh));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmCenter));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_idx));

	m_yaw.m_pid = cPID;
	m_yaw.m_RC = RC;

	pCC = pC->o("visualFollow");
	if(pCC->empty())return false;

	//For visual position locking
	CHECK_ERROR(pCC->v("targetX", &m_roll.m_targetPos));
	CHECK_ERROR(pCC->v("targetY", &m_pitch.m_targetPos));

	resetAllControl();

	m_landingTarget.m_angleX = 0;
	m_landingTarget.m_angleY = 0;
	m_landingTarget.m_orientX = 1.0;
	m_landingTarget.m_orientY = 1.0;
	m_landingTarget.m_ROIstarted = 0;
	m_landingTarget.m_ROItimeLimit = 0;

	pCC = pC->o("visualLanding");
	if(pCC->empty())return false;

	CHECK_INFO(pCC->v("orientationX", &m_landingTarget.m_orientX));
	CHECK_INFO(pCC->v("orientationY", &m_landingTarget.m_orientY));
	CHECK_INFO(pCC->v("roiTimeLimit", &m_landingTarget.m_ROItimeLimit));

	pCC = pCC->o("AprilTags");
	if(pCC->empty())return false;

	CHECK_INFO(pCC->v("num", &m_numATagsLandingTarget));
	for(i=0; i<m_numATagsLandingTarget; i++)
	{
		CHECK_ERROR(pCC->v("tag"+i2str(i), &m_pATagsLandingTarget[i]));
	}

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;


	//Setup parameters to Automaton pointers
	if(m_pAM==NULL)return false;
	//TODO: add Mavlink status


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

		sendHeartbeat();

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

void _AutoPilot::setVehicleInterface(_RC* pVehicle)
{
	if (!pVehicle)
		return;

	m_pVI = pVehicle;
}

void _AutoPilot::setMavlinkInterface(_Mavlink* pMavlink)
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

	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "AutoPilot FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}



/*
void _AutoPilot::landingTarget(void)
{
	CamBase* pCamInput;
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
*/


}
