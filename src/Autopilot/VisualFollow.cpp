#include "VisualFollow.h"

namespace kai
{

VisualFollow::VisualFollow()
{
	ActionBase();

	m_pFD = NULL;
	m_pROITracker = NULL;


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

bool VisualFollow::init(Config* pConfig, string* pName)
{
	if (this->ActionBase::init(pConfig, pName) == false)
		return false;

	int i;
	Config* pC = pConfig->o(*pName);
	Config* pCC = pC->o("visualFollow");
	if (pCC->empty())
		return false;

	//TODO: setup UI
	m_pUI = new UI();



	m_pCtrl->reset();

	return true;
}

void VisualFollow::update(void)
{
	this->ActionBase::update();

	followROI();
}

void VisualFollow::followROI(void)
{
	NULL_(m_pVI);
	NULL_(m_pROITracker);
	NULL_(m_pCtrl);

	if (m_pROITracker->m_bTracking == false)
	{
		m_pCtrl->resetErr();
		m_pCtrl->resetRC();
		m_pVI->rc_overide(m_pCtrl->m_nRC, m_pCtrl->m_pRC);
		return;
	}

	double posRoll;
	double posPitch;
	double ovDTime;

	CONTROL_CHANNEL* pRoll = &m_pCtrl->m_roll;
	CONTROL_CHANNEL* pPitch = &m_pCtrl->m_pitch;
	CONTROL_CHANNEL* pAlt = &m_pCtrl->m_alt;
	CONTROL_CHANNEL* pYaw = &m_pCtrl->m_yaw;

	ovDTime = (1.0 / m_pROITracker->m_dTime) * 1000; //ms
	posRoll = pRoll->m_pos;
	posPitch = pPitch->m_pos;

	//Update pos from ROI tracker
	pRoll->m_pos = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width * 0.5;
	pPitch->m_pos = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height * 0.5;

	//Update current position with trajectory estimation
	posRoll = pRoll->m_pos
			+ (pRoll->m_pos - posRoll) * pRoll->m_pid.m_dT * ovDTime;
	posPitch = pPitch->m_pos
			+ (pPitch->m_pos - posPitch) * pPitch->m_pid.m_dT * ovDTime;

	//Roll
	pRoll->m_errOld = pRoll->m_err;
	pRoll->m_err = pRoll->m_targetPos - posRoll;
	pRoll->m_errInteg += pRoll->m_err;
	pRoll->m_RC.m_pwm = pRoll->m_RC.m_pwmN + pRoll->m_pid.m_P * pRoll->m_err
			+ pRoll->m_pid.m_D * (pRoll->m_err - pRoll->m_errOld) * ovDTime
			+ confineVal(pRoll->m_pid.m_I * pRoll->m_errInteg,
					pRoll->m_pid.m_Imax, -pRoll->m_pid.m_Imax);
	m_pCtrl->m_pRC[pRoll->m_RC.m_iCh] = constrain(pRoll->m_RC.m_pwm,
			pRoll->m_RC.m_pwmL, pRoll->m_RC.m_pwmH);

	//Pitch
	pPitch->m_errOld = pPitch->m_err;
	pPitch->m_err = pPitch->m_targetPos - posPitch;
	pPitch->m_errInteg += pPitch->m_err;
	pPitch->m_RC.m_pwm = pPitch->m_RC.m_pwmN + pPitch->m_pid.m_P * pPitch->m_err
			+ pPitch->m_pid.m_D * (pPitch->m_err - pPitch->m_errOld) * ovDTime
			+ confineVal(pPitch->m_pid.m_I * pPitch->m_errInteg,
					pPitch->m_pid.m_Imax, -pPitch->m_pid.m_Imax);
	m_pCtrl->m_pRC[pPitch->m_RC.m_iCh] = constrain(pPitch->m_RC.m_pwm,
			pPitch->m_RC.m_pwmL, pPitch->m_RC.m_pwmH);

	//RC output
	m_pVI->rc_overide(m_pCtrl->m_nRC, m_pCtrl->m_pRC);
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
