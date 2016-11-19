#include "APMrover_follow.h"

namespace kai
{

APMrover_follow::APMrover_follow()
{
	ActionBase();

	m_pAPM = NULL;
	m_pAM = NULL;
	m_pUniv = NULL;

	m_destX = 0.5;
	m_destY = 0.5;
	m_destArea = 0.1;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_pTarget = NULL;
	m_pTargetX = new kai::Filter();
	m_pTargetY = new kai::Filter();
	m_pTargetArea = new kai::Filter();
	m_filterWindow = 3;
	m_targetClass = 0;

	m_pAPM = NULL;
}

APMrover_follow::~APMrover_follow()
{
}

bool APMrover_follow::init(Config* pConfig)
{
	CHECK_F(this->ActionBase::init(pConfig)==false);
	pConfig->m_pInst = this;

	F_INFO(pConfig->v("targetX", &m_destX));
	F_INFO(pConfig->v("targetY", &m_destY));
	F_INFO(pConfig->v("targetArea", &m_destArea));
	F_INFO(pConfig->v("targetClass", &m_targetClass));

	F_INFO(pConfig->v("speedP", &m_speedP));
	F_INFO(pConfig->v("steerP", &m_steerP));

	F_INFO(pConfig->v("filterWindow", &m_filterWindow));
	m_pTargetX->startMedian(m_filterWindow);
	m_pTargetY->startMedian(m_filterWindow);
	m_pTargetArea->startMedian(m_filterWindow);

	return true;
}

bool APMrover_follow::link(void)
{
	NULL_F(m_pConfig);
	string iName = "";

	F_INFO(m_pConfig->v("APMrover_base", &iName));
	m_pAPM = (APMrover_base*) (m_pConfig->parent()->getChildInstByName(&iName));

	F_INFO(m_pConfig->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (m_pConfig->root()->getChildInstByName(&iName));

	F_INFO(m_pConfig->v("_Universe", &iName));
	m_pUniv = (_Universe*) (m_pConfig->root()->getChildInstByName(&iName));

	return true;
}

void APMrover_follow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAPM);
	NULL_(m_pUniv);

	//get visual target and decide motion
	m_pTarget = m_pUniv->getObjectByClass(m_targetClass);

	if (m_pTarget == NULL)
	{
		//no target found, stop and standby TODO: go back to work
//		m_pAPM->m_motorPwmL = 0;
//		m_pAPM->m_motorPwmR = 0;
//		m_pAPM->m_bSpeaker = false;
	}
	else
	{
		m_pTargetX->input(m_pTarget->m_bbox.midX());
		m_pTargetY->input(m_pTarget->m_bbox.midY());
		m_pTargetArea->input(m_pTarget->m_bbox.area());

		//forward or backward
		int rpmSpeed = (m_destArea*m_pTarget->m_camSize.area() - m_pTargetArea->v()) * m_speedP;

		//steering
		int rpmSteer = (m_destX*m_pTarget->m_camSize.m_x - m_pTargetX->v()) * m_steerP;

//		m_pAPM->m_motorPwmL = rpmSpeed - rpmSteer;
//		m_pAPM->m_motorPwmR = rpmSpeed + rpmSteer;
//		m_pAPM->m_bSpeaker = true;
	}

//	m_pAPM->updateCAN();


}

bool APMrover_follow::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);
	Mat* pMat = pFrame->getCMat();

//	putText(*pMat,
//			"APMrover: rpmL=" + i2str(m_pAPM->m_motorPwmL) + ", rpmR="
//					+ i2str(m_pAPM->m_motorPwmR),
//			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
//			Scalar(0, 255, 0), 1);
//	pTextPos->m_y += pTextPos->m_w;

	CHECK_T(m_pTarget==NULL);
	circle(*pMat, Point(m_pTarget->m_bbox.midX(), m_pTarget->m_bbox.midY()), 10,
			Scalar(0, 0, 255), 2);

	return true;
}


}
