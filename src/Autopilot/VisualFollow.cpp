#include "VisualFollow.h"

namespace kai
{

VisualFollow::VisualFollow()
{
	ActionBase();

	m_pFD = NULL;
	m_pROITracker = NULL;
	m_pFlow = NULL;

	m_ROI.m_x = 0;
	m_ROI.m_y = 0;
	m_ROI.m_z = 0;
	m_ROI.m_w = 0;
	m_bSelect = false;
	m_ROImode = MODE_ASSIST;
	m_ROIsize = 100;
	m_ROIsizeFrom = 50;
	m_ROIsizeTo = 300;

	m_btnSize = 100;
	m_btnROIClear = 100;
	m_btnROIBig = 200;
	m_btnROISmall = 300;
	m_btnMode = 980;
}

VisualFollow::~VisualFollow()
{
}

bool VisualFollow::init(Config* pConfig, AUTOPILOT_CONTROL* pAC)
{
	if (this->ActionBase::init(pConfig, pAC) == false)
		return false;

	m_pCtrl = pAC;

	m_roll.reset();
	m_pitch.reset();
	m_yaw.reset();
	m_alt.reset();

	//For visual position locking
	F_ERROR_F(pConfig->v("targetX", &m_roll.m_targetPos));
	F_ERROR_F(pConfig->v("targetY", &m_pitch.m_targetPos));

	//TODO: Link ROI tracker etc.

	//TODO: setup UI
	m_pUI = new UI();

	return true;
}

void VisualFollow::update(void)
{
	this->ActionBase::update();

	followROI();
}

void VisualFollow::followROI(void)
{
	NULL_(m_pCtrl);
	NULL_(m_pCtrl->m_pRC);
	NULL_(m_pROITracker);

	if (m_pROITracker->m_bTracking == false)
	{
		m_pCtrl->RCneutral();
		m_roll.resetErr();
		m_pitch.resetErr();
//		m_pCtrl->m_pRC->rc_overide(m_pCtrl->m_nRC, m_pCtrl->m_pRC);
		return;
	}

	double posRoll;
	double posPitch;
	double ovDTime;

	CONTROL_PARAM* pRoll = &m_pCtrl->m_roll;
	CONTROL_PARAM* pPitch = &m_pCtrl->m_pitch;
	CONTROL_PARAM* pAlt = &m_pCtrl->m_alt;
	CONTROL_PARAM* pYaw = &m_pCtrl->m_yaw;

	CONTROL_PID* pidRoll = &pRoll->m_pid;
	CONTROL_PID* pidPitch = &pPitch->m_pid;
	CONTROL_PID* pidAlt = &pAlt->m_pid;
	CONTROL_PID* pidYaw = &pYaw->m_pid;

	RC_CHANNEL* pRCroll = &pRoll->m_RC;
	RC_CHANNEL* pRCpitch = &pPitch->m_RC;
	RC_CHANNEL* pRCalt = &pAlt->m_RC;
	RC_CHANNEL* pRCyaw = &pYaw->m_RC;


	ovDTime = (1.0 / m_pROITracker->m_dTime) * 1000; //ms
	posRoll = m_roll.m_pos;
	posPitch = m_pitch.m_pos;

	//Update pos from ROI tracker
	m_roll.m_pos = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width * 0.5;
	m_pitch.m_pos = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height * 0.5;

	//Update current position with trajectory estimation
	posRoll = m_roll.m_pos
			+ (m_roll.m_pos - posRoll) * pidRoll->m_dT * ovDTime;
	posPitch = m_pitch.m_pos
			+ (m_pitch.m_pos - posPitch) * pidPitch->m_dT * ovDTime;

	//Roll
	m_roll.m_errOld = m_roll.m_err;
	m_roll.m_err = m_roll.m_targetPos - posRoll;
	m_roll.m_errInteg += m_roll.m_err;
	pRCroll->m_pwm = pRCroll->m_pwmN + pidRoll->m_P * m_roll.m_err
			+ pidRoll->m_D * (m_roll.m_err - m_roll.m_errOld) * ovDTime
			+ confineVal(pidRoll->m_I * m_roll.m_errInteg,
					pidRoll->m_Imax, -pidRoll->m_Imax);
	pRCroll->m_pwm = constrain(pRCroll->m_pwm, pRCroll->m_pwmL, pRCroll->m_pwmH);

	//Pitch
	m_pitch.m_errOld = m_pitch.m_err;
	m_pitch.m_err = m_pitch.m_targetPos - posPitch;
	m_pitch.m_errInteg += m_pitch.m_err;
	pRCpitch->m_pwm = pRCpitch->m_pwmN + pidPitch->m_P * m_pitch.m_err
			+ pidPitch->m_D * (m_pitch.m_err - m_pitch.m_errOld) * ovDTime
			+ confineVal(pidPitch->m_I * m_pitch.m_errInteg,
					pidPitch->m_Imax, - pidPitch->m_Imax);
	pRCpitch->m_pwm = constrain(pRCpitch->m_pwm, pRCpitch->m_pwmL, pRCpitch->m_pwmH);

	//RC output
//	m_pCtrl->m_pRC->rc_overide(m_pCtrl->m_nRC, m_pCtrl->m_pRC);
	return;

}

bool VisualFollow::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	Rect2d roi;

/*
	 // draw the tracked object
	if(m_bSelect)
	{
		roi = getROI(m_ROI);
		if(roi.height>0 || roi.width>0)
		{
			rectangle( imMat, roi, Scalar( 0, 255, 0 ), 2 );
		}
	}
	else if(m_pROITracker->m_bTracking)
	{
		roi = m_pROITracker->m_ROI;
		if(roi.height>0 || roi.width>0)
		{
			rectangle( imMat, roi, Scalar( 0, 0, 255 ), 2 );
		}
	}

	if(m_ROImode == MODE_ASSIST)
	{
		putText(imMat, "CLR", cv::Point(1825,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);
		putText(imMat, "+", cv::Point(1825,150), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);
		putText(imMat, "-", cv::Point(1825,250), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);

		roi.x = imMat.cols - m_btnSize;
		roi.y = 0;
		roi.width = m_btnSize;
		roi.height = m_btnSize;
		rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );

		roi.y = m_btnROIClear;
		rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );

		roi.y = m_btnROIBig;
		rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );
	}

	putText(imMat, "MODE", cv::Point(1825,1035), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1);

	roi.x = imMat.cols - m_btnSize;
	roi.y = m_btnMode;
	roi.width = m_btnSize;
	roi.height = m_btnSize;
	rectangle( imMat, roi, Scalar( 0, 255, 0 ), 1 );


	circle(imMat, Point(m_pAP->m_roll.m_targetPos, m_pAP->m_pitch.m_targetPos), 50, Scalar(0,255,0), 2);

*/
	return true;
}

}
