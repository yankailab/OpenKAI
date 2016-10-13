#include "HM.h"

namespace kai
{

HM::HM()
{
	ActionBase();

	m_pROITracker = NULL;
	m_pCtrl = NULL;
}

HM::~HM()
{
}

bool HM::init(Config* pConfig, AUTOPILOT_CONTROL* pAC)
{
	if (this->ActionBase::init(pConfig, pAC) == false)
		return false;

	m_pCtrl = pAC;

//	F_ERROR_F(pConfig->v("targetX", &m_roll.m_targetPos));
//	F_ERROR_F(pConfig->v("targetY", &m_pitch.m_targetPos));

//link ROI tracker
//	string roiName = "";
//	F_ERROR_F(pConfig->v("ROItracker", &roiName));
//	m_pROITracker = (_ROITracker*) (pConfig->root()->getChildInstByName(&roiName));

//setup UI
//	Config* pC;
//	pC = pConfig->o("assist");
//	if (!pC->empty())
//	{
//		m_pUIassist = new UI();
//		F_FATAL_F(m_pUIassist->init(pC));
//	}

	pConfig->m_pInst = this;

	return true;
}

void HM::update(void)
{
	this->ActionBase::update();

	NULL_(m_pCtrl);
	NULL_(m_pCtrl->m_pCAN);
//	NULL_(m_pROITracker);

//	if (m_pROITracker->m_bTracking == false)
//	{
//		return;
//	}

//RC output
//	m_pCtrl->m_pRC->rc_overide(m_pCtrl->m_nRC, m_pCtrl->m_pRC);
	return;

}

bool HM::draw(Frame* pFrame, iVec4* pTextPos)
{
	NULL_F(pFrame);
//	NULL_F(m_pROITracker);
	Mat* pMat = pFrame->getCMat();

	putText(*pMat,
			"HM",
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}
