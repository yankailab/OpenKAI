#include "VisualFollow.h"

namespace kai
{

VisualFollow::VisualFollow()
{
	ActionBase();

	m_pFD = NULL;
	m_pROITracker = NULL;
	m_pMD = NULL;

	m_pRC = NULL;
	m_nRC = 0;
	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pYaw = NULL;
	m_pAlt = NULL;

}

VisualFollow::~VisualFollow()
{
}

bool VisualFollow::init(Config* pConfig, string name)
{
	if (this->ActionBase::init(pConfig, name) == false)
		return false;

	int i;
	Config* pC = pConfig->o(name);
	Config* pCC = pC->o("visualVisualFollow");
	if (pCC->empty())
		return false;


	resetAllControl();

	return true;
}

void VisualFollow::update(void)
{
	this->ActionBase::update();

	followROI();
}

void VisualFollow::followROI(void)
{
	if (m_pVI == NULL)
		return;
	if (m_pROITracker == NULL)
		return;
	if (m_pRC == NULL)
		return;

	if (m_pROITracker->m_bTracking == false)
	{
		resetAllControl();
		m_pVI->rc_overide(m_nRC, m_pRC);
		return;
	}

	double posRoll;
	double posPitch;
	double ovDTime;

	ovDTime = (1.0 / m_pROITracker->m_dTime) * 1000; //ms
	posRoll = m_pRoll->m_pos;
	posPitch = m_pPitch->m_pos;

	m_pRoll->m_pos = m_pROITracker->m_ROI.x + m_pROITracker->m_ROI.width * 0.5;
	m_pPitch->m_pos = m_pROITracker->m_ROI.y
			+ m_pROITracker->m_ROI.height * 0.5;

	//Update current position with trajectory estimation
	posRoll = m_pRoll->m_pos
			+ (m_pRoll->m_pos - posRoll) * m_pRoll->m_pid.m_dT * ovDTime;
	posPitch = m_pPitch->m_pos
			+ (m_pPitch->m_pos - posPitch) * m_pPitch->m_pid.m_dT * ovDTime;

	//Roll
	m_pRoll->m_errOld = m_pRoll->m_err;
	m_pRoll->m_err = m_pRoll->m_targetPos - posRoll;
	m_pRoll->m_errInteg += m_pRoll->m_err;
	m_pRoll->m_RC.m_pwm = m_pRoll->m_RC.m_pwmN
			+ m_pRoll->m_pid.m_P * m_pRoll->m_err
			+ m_pRoll->m_pid.m_D * (m_pRoll->m_err - m_pRoll->m_errOld)
					* ovDTime
			+ confineVal(m_pRoll->m_pid.m_I * m_pRoll->m_errInteg,
					m_pRoll->m_pid.m_Imax, -m_pRoll->m_pid.m_Imax);
	m_pRC[m_pRoll->m_RC.m_iCh] = constrain(m_pRoll->m_RC.m_pwm,
			m_pRoll->m_RC.m_pwmL, m_pRoll->m_RC.m_pwmH);

	//Pitch
	m_pPitch->m_errOld = m_pPitch->m_err;
	m_pPitch->m_err = m_pPitch->m_targetPos - posPitch;
	m_pPitch->m_errInteg += m_pPitch->m_err;
	m_pPitch->m_RC.m_pwm = m_pPitch->m_RC.m_pwmN
			+ m_pPitch->m_pid.m_P * m_pPitch->m_err
			+ m_pPitch->m_pid.m_D * (m_pPitch->m_err - m_pPitch->m_errOld)
					* ovDTime
			+ confineVal(m_pPitch->m_pid.m_I * m_pPitch->m_errInteg,
					m_pPitch->m_pid.m_Imax, -m_pPitch->m_pid.m_Imax);
	m_pRC[m_pPitch->m_RC.m_iCh] = constrain(m_pPitch->m_RC.m_pwm,
			m_pPitch->m_RC.m_pwmL, m_pPitch->m_RC.m_pwmH);

	//Mavlink
	m_pVI->rc_overide(m_nRC, m_pRC);
	return;

}

void VisualFollow::resetAllControl(void)
{
	m_pRoll->reset();
	m_pAlt->reset();
	m_pPitch->reset();
	m_pYaw->reset();

	//RC
	m_pRC[m_pRoll->m_RC.m_iCh] = m_pRoll->m_RC.m_pwmN;
	m_pRC[m_pPitch->m_RC.m_iCh] = m_pPitch->m_RC.m_pwmN;
	m_pRC[m_pYaw->m_RC.m_iCh] = m_pYaw->m_RC.m_pwmN;
	m_pRC[m_pAlt->m_RC.m_iCh] = m_pAlt->m_RC.m_pwmN;
}


bool VisualFollow::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	return true;
}

}
