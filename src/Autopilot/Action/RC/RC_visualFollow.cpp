#include "RC_visualFollow.h"

#ifndef USE_OPENCV4TEGRA

namespace kai
{

RC_visualFollow::RC_visualFollow()
{
	m_pRC = NULL;
	m_pRCconfig = NULL;
	m_pROITracker = NULL;
	m_pFlow = NULL;

	m_bUI = false;
	m_pUIassist = NULL;
	m_pUIdrawRect = NULL;

	m_ROI.m_x = 0;
	m_ROI.m_y = 0;
	m_ROI.m_z = 0;
	m_ROI.m_w = 0;
	m_bSelect = false;
	m_ROImode = MODE_ASSIST;
	m_ROIsize = 100;
	m_ROIsizeFrom = 50;
	m_ROIsizeTo = 300;
	m_ROIsizeStep = 10;
}

RC_visualFollow::~RC_visualFollow()
{
}

bool RC_visualFollow::init(void* pKiss)
{
	CHECK_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	m_roll.reset();
	m_pitch.reset();
	m_yaw.reset();
	m_alt.reset();

	F_ERROR_F(pK->v("targetX", &m_roll.m_targetPos));
	F_ERROR_F(pK->v("targetY", &m_pitch.m_targetPos));
	F_INFO(pK->v("ROIsizeFrom", &m_ROIsizeFrom));
	F_INFO(pK->v("ROIsizeTo", &m_ROIsizeTo));
	F_INFO(pK->v("ROIsizeStep", &m_ROIsizeStep));

	//setup UI
	Kiss* pC;

	pC = pK->o("assist");
	if (!pC->empty())
	{
		m_pUIassist = new UI();
		F_FATAL_F(m_pUIassist->init(pC));
	}

	pC = pK->o("drawRect");
	if (!pC->empty())
	{
		m_pUIdrawRect = new UI();
		F_FATAL_F(m_pUIdrawRect->init(pC));
	}

	pK->m_pInst = this;

	return true;
}

bool RC_visualFollow::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";

	F_INFO(pK->v("_RC", &iName));
	m_pRC = (_RC*) (pK->root()->getChildInstByName(&iName));

	F_INFO(pK->v("RC_base", &iName));
	m_pRCconfig = (RC_base*) (pK->root()->getChildInstByName(&iName));

	F_ERROR_F(pK->v("ROItracker", &iName));
	m_pROITracker = (_ROITracker*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void RC_visualFollow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pRC);
	NULL_(m_pROITracker);
	CHECK_(isActive()==false);

	if (m_pROITracker->m_bTracking == false)
	{
		m_pRCconfig->m_rcRoll.neutral();
		m_pRCconfig->m_rcPitch.neutral();
		m_roll.resetErr();
		m_pitch.resetErr();
//		m_pCtrl->m_pRC->rc_overide(m_pCtrl->m_nRC, m_pCtrl->m_pRC);
		return;
	}

	double posRoll;
	double posPitch;
	double ovDTime;

	RC_PID* pidRoll = &m_pRCconfig->m_pidRoll;
	RC_PID* pidPitch = &m_pRCconfig->m_pidPitch;
	RC_PID* pidAlt = &m_pRCconfig->m_pidAlt;
	RC_PID* pidYaw = &m_pRCconfig->m_pidYaw;

	RC_CHANNEL* pRCroll = &m_pRCconfig->m_rcRoll;
	RC_CHANNEL* pRCpitch = &m_pRCconfig->m_rcPitch;
	RC_CHANNEL* pRCalt = &m_pRCconfig->m_rcAlt;
	RC_CHANNEL* pRCyaw = &m_pRCconfig->m_rcYaw;

	ovDTime = (1.0 / m_pROITracker->m_dTime) * 1000; //ms
	posRoll = m_roll.m_pos;
	posPitch = m_pitch.m_pos;

	//Update pos from ROI tracker
	m_roll.m_pos = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width * 0.5;
	m_pitch.m_pos = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height * 0.5;

	//Update current position with trajectory estimation
	posRoll = m_roll.m_pos + (m_roll.m_pos - posRoll) * pidRoll->m_dT * ovDTime;
	posPitch = m_pitch.m_pos
			+ (m_pitch.m_pos - posPitch) * pidPitch->m_dT * ovDTime;

	//Roll
	m_roll.m_errOld = m_roll.m_err;
	m_roll.m_err = m_roll.m_targetPos - posRoll;
	m_roll.m_errInteg += m_roll.m_err;
	pRCroll->m_pwm = pRCroll->m_pwmN + pidRoll->m_P * m_roll.m_err
			+ pidRoll->m_D * (m_roll.m_err - m_roll.m_errOld) * ovDTime
			+ constrain(pidRoll->m_I * m_roll.m_errInteg, pidRoll->m_Imax,
					-pidRoll->m_Imax);
	pRCroll->m_pwm = constrain(pRCroll->m_pwm, pRCroll->m_pwmL,
			pRCroll->m_pwmH);

	//Pitch
	m_pitch.m_errOld = m_pitch.m_err;
	m_pitch.m_err = m_pitch.m_targetPos - posPitch;
	m_pitch.m_errInteg += m_pitch.m_err;
	pRCpitch->m_pwm = pRCpitch->m_pwmN + pidPitch->m_P * m_pitch.m_err
			+ pidPitch->m_D * (m_pitch.m_err - m_pitch.m_errOld) * ovDTime
			+ constrain(pidPitch->m_I * m_pitch.m_errInteg, pidPitch->m_Imax,
					-pidPitch->m_Imax);
	pRCpitch->m_pwm = constrain(pRCpitch->m_pwm, pRCpitch->m_pwmL,
			pRCpitch->m_pwmH);

	//RC output
//	m_pCtrl->m_pRC->rc_overide(m_pCtrl->m_nRC, m_pCtrl->m_pRC);
	return;

}

bool RC_visualFollow::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	NULL_F(m_pROITracker);

	Rect2d roi;

	// draw the tracked object
	if (m_bSelect)
	{
		roi = getMouseROI(m_ROI);
		if (roi.height > 0 || roi.width > 0)
		{
			rectangle(*pMat, roi, Scalar(0, 255, 0), 2);
		}
	}
	else if (m_pROITracker->m_bTracking)
	{
		roi = m_pROITracker->m_ROI;
		if (roi.height > 0 || roi.width > 0)
		{
			rectangle(*pMat, roi, Scalar(0, 0, 255), 2);
		}
	}

	if (m_ROImode == MODE_ASSIST)
	{
		m_pUIassist->draw();
	}
	else if (m_ROImode == MODE_DRAWRECT)
	{
		m_pUIdrawRect->draw();
	}

	circle(*pMat, Point(m_roll.m_targetPos, m_pitch.m_targetPos), 50,
			Scalar(0, 255, 0), 2);

	return true;
}

Rect2d RC_visualFollow::getMouseROI(vInt4 mouseROI)
{
	Rect2d roi;

	roi.x = min(mouseROI.m_x, mouseROI.m_z);
	roi.y = min(mouseROI.m_y, mouseROI.m_w);
	roi.width = abs(mouseROI.m_z - mouseROI.m_x);
	roi.height = abs(mouseROI.m_w - mouseROI.m_y);

	return roi;
}

void RC_visualFollow::onMouse(MOUSE* pMouse)
{
	NULL_(pMouse);

	//update UI
	if (m_ROImode == MODE_ASSIST)
	{
		NULL_(m_pUIassist);
		onMouseAssist(pMouse, m_pUIassist->onMouse(pMouse));
	}
	else if (m_ROImode == MODE_DRAWRECT)
	{
		NULL_(m_pUIdrawRect);
		onMouseDrawRect(pMouse, m_pUIdrawRect->onMouse(pMouse));
	}
}

void RC_visualFollow::onMouseAssist(MOUSE* pMouse, BUTTON* pBtn)
{
	NULL_(pMouse);

	Rect2d roi;
	int ROIhalf;

	switch (pMouse->m_event)
	{
	case EVENT_LBUTTONDOWN:
		if (!pBtn)
		{
			ROIhalf = m_ROIsize / 2;
			m_ROI.m_x = pMouse->m_x - ROIhalf;
			m_ROI.m_y = pMouse->m_y - ROIhalf;
			m_ROI.m_z = pMouse->m_x + ROIhalf;
			m_ROI.m_w = pMouse->m_y + ROIhalf;
			roi = getMouseROI(m_ROI);
			m_pROITracker->setROI(roi);
			m_pROITracker->tracking(true);
			m_bSelect = true;
			return;
		}

		if (pBtn->m_name == "CLR")
		{
			//Clear ROI
			m_pROITracker->tracking(false);
			m_bSelect = false;
			return;
		}

		if (pBtn->m_name == "+")
		{
			//Magnify the ROI size
			m_bSelect = false;

			CHECK_(m_ROIsize >= m_ROIsizeTo);
			m_ROIsize += m_ROIsizeStep;
			CHECK_(!m_pROITracker->m_bTracking);

			roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width / 2;
			roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height / 2;
			ROIhalf = m_ROIsize / 2;

			m_ROI.m_x = roi.x - ROIhalf;
			m_ROI.m_y = roi.y - ROIhalf;
			m_ROI.m_z = roi.x + ROIhalf;
			m_ROI.m_w = roi.y + ROIhalf;
			roi = getMouseROI(m_ROI);
			m_pROITracker->setROI(roi);
			return;
		}

		if (pBtn->m_name == "-")
		{
			//Shrink the ROI size
			m_bSelect = false;

			CHECK_(m_ROIsize <= m_ROIsizeFrom);
			m_ROIsize -= m_ROIsizeStep;
			CHECK_(!m_pROITracker->m_bTracking);

			roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width / 2;
			roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height / 2;
			ROIhalf = m_ROIsize / 2;

			m_ROI.m_x = roi.x - ROIhalf;
			m_ROI.m_y = roi.y - ROIhalf;
			m_ROI.m_z = roi.x + ROIhalf;
			m_ROI.m_w = roi.y + ROIhalf;
			roi = getMouseROI(m_ROI);
			m_pROITracker->setROI(roi);
			return;
		}

		if (pBtn->m_name == "MODE")
		{
			m_ROI.m_x = 0;
			m_ROI.m_y = 0;
			m_ROI.m_z = 0;
			m_ROI.m_w = 0;
			m_pROITracker->tracking(false);
			m_bSelect = false;
			m_ROImode = MODE_DRAWRECT;
			return;
		}

		break;
	case EVENT_MOUSEMOVE:
		CHECK_(!m_bSelect);

		ROIhalf = m_ROIsize / 2;
		m_ROI.m_x = pMouse->m_x - ROIhalf;
		m_ROI.m_y = pMouse->m_y - ROIhalf;
		m_ROI.m_z = pMouse->m_x + ROIhalf;
		m_ROI.m_w = pMouse->m_y + ROIhalf;
		roi = getMouseROI(m_ROI);
		m_pROITracker->setROI(roi);
		m_pROITracker->tracking(true);
		break;
	case EVENT_LBUTTONUP:
		m_bSelect = false;
		break;
	case EVENT_RBUTTONDOWN:
		break;
	default:
		break;
	}

}

void RC_visualFollow::onMouseDrawRect(MOUSE* pMouse, BUTTON* pBtn)
{
	NULL_(pMouse);

	Rect2d roi;
	int ROIhalf;

	switch (pMouse->m_event)
	{
	case EVENT_LBUTTONDOWN:
		if (!pBtn)
		{
			m_pROITracker->tracking(false);
			m_ROI.m_x = pMouse->m_x;
			m_ROI.m_y = pMouse->m_y;
			m_ROI.m_z = pMouse->m_x;
			m_ROI.m_w = pMouse->m_y;
			m_bSelect = true;
			return;
		}

		if (pBtn->m_name == "MODE")
		{
			m_ROI.m_x = 0;
			m_ROI.m_y = 0;
			m_ROI.m_z = 0;
			m_ROI.m_w = 0;
			m_pROITracker->tracking(false);
			m_bSelect = false;
			m_ROImode = MODE_ASSIST;
			return;
		}

		break;
	case EVENT_MOUSEMOVE:
		CHECK_(!m_bSelect);

		m_ROI.m_z = pMouse->m_x;
		m_ROI.m_w = pMouse->m_y;
		break;
	case EVENT_LBUTTONUP:
		roi = getMouseROI(m_ROI);
		if (roi.width < m_ROIsizeFrom || roi.height < m_ROIsizeFrom)
		{
			m_ROI.m_x = 0;
			m_ROI.m_y = 0;
			m_ROI.m_z = 0;
			m_ROI.m_w = 0;
		}
		else
		{
			m_pROITracker->setROI(roi);
			m_pROITracker->tracking(true);
		}
		m_bSelect = false;
		break;
	case EVENT_RBUTTONDOWN:
		break;
	default:
		break;
	}

}

}

#endif

