#include "../../Controller/RC/RC_visualFollow.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

RC_visualFollow::RC_visualFollow()
{
	m_pRCIO = NULL;
	m_pRCbase = NULL;
	m_pROITracker = NULL;
	m_pFlow = NULL;

	m_bUI = false;
	m_pUIassist = NULL;
	m_pUIdrawRect = NULL;

	m_ROI.init();
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
	IF_F(!this->ActionBase::init(pKiss));
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
	IF_F(pC->empty());
	m_pUIassist = new UI();
	F_FATAL_F(m_pUIassist->init(pC));

	pC = pK->o("drawRect");
	IF_F(pC->empty());
	m_pUIdrawRect = new UI();
	F_FATAL_F(m_pUIdrawRect->init(pC));

	pK->m_pInst = this;

	//link
	string iName = "";

	F_INFO(pK->v("RC_base", &iName));
	m_pRCbase = (RC_base*) (pK->parent()->getChildInstByName(&iName));

	F_ERROR_F(pK->v("ROItracker", &iName));
	m_pROITracker = (_SingleTracker*) (pK->root()->getChildInstByName(&iName));

	IF_F(!m_pUIassist->link());
	IF_F(!m_pUIdrawRect->link());

	return true;
}

void RC_visualFollow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pRCbase);
	m_pRCIO = m_pRCbase->m_pRC;
	NULL_(m_pRCIO);
	NULL_(m_pROITracker);

//	if (m_pROITracker->m_bTracking == false)
//	{
//		m_pRCbase->m_rcRoll.neutral();
//		m_pRCbase->m_rcPitch.neutral();
//		m_roll.resetErr();
//		m_pitch.resetErr();
//		m_pRCIO->rc_overide(NUM_RC_CHANNEL, m_pRCbase->m_pPWM);
//		return;
//	}

	double posRoll;
	double posPitch;
	double ovDTime;

	RC_PID* pidRoll = &m_pRCbase->m_pidRoll;
	RC_PID* pidPitch = &m_pRCbase->m_pidPitch;
	RC_PID* pidAlt = &m_pRCbase->m_pidAlt;
	RC_PID* pidYaw = &m_pRCbase->m_pidYaw;

	RC_CHANNEL* pRCroll = &m_pRCbase->m_rcRoll;
	RC_CHANNEL* pRCpitch = &m_pRCbase->m_rcPitch;
	RC_CHANNEL* pRCalt = &m_pRCbase->m_rcAlt;
	RC_CHANNEL* pRCyaw = &m_pRCbase->m_rcYaw;

	ovDTime = (1.0 / m_pROITracker->m_dTime) * 1000; //ms
	posRoll = m_roll.m_pos;
	posPitch = m_pitch.m_pos;

	//Update pos from ROI tracker
	m_roll.m_pos = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width * 0.5;
	m_pitch.m_pos = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height * 0.5;

	//Update current position with trajectory estimation
	posRoll = m_roll.m_pos + (m_roll.m_pos - posRoll) * pidRoll->m_dT * ovDTime;
	posPitch = m_pitch.m_pos + (m_pitch.m_pos - posPitch) * pidPitch->m_dT * ovDTime;

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

	m_pRCbase->m_pPWM[pRCroll->m_iCh] = pRCroll->m_pwm;
	m_pRCbase->m_pPWM[pRCpitch->m_iCh] = pRCpitch->m_pwm;

	//RC output
	m_pRCIO->rc_overide(NUM_RC_CHANNEL, m_pRCbase->m_pPWM);
	return;

}

bool RC_visualFollow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	NULL_F(m_pROITracker);

	Rect2d roi;

	// draw the tracked object
//	if (m_bSelect)
//	{
//		roi = getMouseROI(m_ROI);
//		if (roi.height > 0 || roi.width > 0)
//		{
//			rectangle(*pMat, roi, Scalar(0, 255, 0), 2);
//		}
//	}
//	else if (m_pROITracker->m_bTracking)
//	{
//		roi = m_pROITracker->m_ROI;
//		if (roi.height > 0 || roi.width > 0)
//		{
//			rectangle(*pMat, roi, Scalar(0, 0, 255), 2);
//		}
//	}

	if (m_ROImode == MODE_ASSIST)
	{
		m_pUIassist->draw();
	}
	else if (m_ROImode == MODE_DRAWRECT)
	{
		m_pUIdrawRect->draw();
	}

	circle(*pMat, Point(m_roll.m_targetPos, m_pitch.m_targetPos), 50, Scalar(0, 255, 0), 2);

	return true;
}

Rect2d RC_visualFollow::getMouseROI(vInt4 mouseROI)
{
	Rect2d roi;

	roi.x = min(mouseROI.x, mouseROI.z);
	roi.y = min(mouseROI.y, mouseROI.w);
	roi.width = abs(mouseROI.z - mouseROI.x);
	roi.height = abs(mouseROI.w - mouseROI.y);

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
			m_ROI.x = pMouse->m_x - ROIhalf;
			m_ROI.y = pMouse->m_y - ROIhalf;
			m_ROI.z = pMouse->m_x + ROIhalf;
			m_ROI.w = pMouse->m_y + ROIhalf;
			roi = getMouseROI(m_ROI);
//			m_pROITracker->setROI(roi);
//			m_pROITracker->tracking(true);
			m_bSelect = true;
			return;
		}

		if (pBtn->m_name == "CLR")
		{
			//Clear ROI
//			m_pROITracker->tracking(false);
			m_bSelect = false;
			return;
		}

		if (pBtn->m_name == "+")
		{
			//Magnify the ROI size
			m_bSelect = false;

			IF_(m_ROIsize >= m_ROIsizeTo);
			m_ROIsize += m_ROIsizeStep;
//			IF_(!m_pROITracker->m_bTracking);

			roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width / 2;
			roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height / 2;
			ROIhalf = m_ROIsize / 2;

			m_ROI.x = roi.x - ROIhalf;
			m_ROI.y = roi.y - ROIhalf;
			m_ROI.z = roi.x + ROIhalf;
			m_ROI.w = roi.y + ROIhalf;
			roi = getMouseROI(m_ROI);
//			m_pROITracker->setROI(roi);
			return;
		}

		if (pBtn->m_name == "-")
		{
			//Shrink the ROI size
			m_bSelect = false;

			IF_(m_ROIsize <= m_ROIsizeFrom);
			m_ROIsize -= m_ROIsizeStep;
//			IF_(!m_pROITracker->m_bTracking);

			roi.x = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width / 2;
			roi.y = m_pROITracker->m_ROI.y + m_pROITracker->m_ROI.height / 2;
			ROIhalf = m_ROIsize / 2;

			m_ROI.x = roi.x - ROIhalf;
			m_ROI.y = roi.y - ROIhalf;
			m_ROI.z = roi.x + ROIhalf;
			m_ROI.w = roi.y + ROIhalf;
			roi = getMouseROI(m_ROI);
//			m_pROITracker->setROI(roi);
			return;
		}

		if (pBtn->m_name == "MODE")
		{
			m_ROI.init();
//			m_pROITracker->tracking(false);
			m_bSelect = false;
			m_ROImode = MODE_DRAWRECT;
			return;
		}

		break;
	case EVENT_MOUSEMOVE:
		IF_(!m_bSelect);

		ROIhalf = m_ROIsize / 2;
		m_ROI.x = pMouse->m_x - ROIhalf;
		m_ROI.y = pMouse->m_y - ROIhalf;
		m_ROI.z = pMouse->m_x + ROIhalf;
		m_ROI.w = pMouse->m_y + ROIhalf;
		roi = getMouseROI(m_ROI);
//		m_pROITracker->setROI(roi);
//		m_pROITracker->tracking(true);
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

	switch (pMouse->m_event)
	{
	case EVENT_LBUTTONDOWN:
		if (!pBtn)
		{
//			m_pROITracker->tracking(false);
			m_ROI.x = pMouse->m_x;
			m_ROI.y = pMouse->m_y;
			m_ROI.z = pMouse->m_x;
			m_ROI.w = pMouse->m_y;
			m_bSelect = true;
			return;
		}

		if (pBtn->m_name == "MODE")
		{
			m_ROI.init();
//			m_pROITracker->tracking(false);
			m_bSelect = false;
			m_ROImode = MODE_ASSIST;
			return;
		}

		break;
	case EVENT_MOUSEMOVE:
		IF_(!m_bSelect);

		m_ROI.z = pMouse->m_x;
		m_ROI.w = pMouse->m_y;
		break;
	case EVENT_LBUTTONUP:
		roi = getMouseROI(m_ROI);
		if (roi.width < m_ROIsizeFrom || roi.height < m_ROIsizeFrom)
		{
			m_ROI.init();
		}
		else
		{
//			m_pROITracker->setROI(roi);
//			m_pROITracker->tracking(true);
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
