#include "HM_marker.h"

namespace kai
{

HM_marker::HM_marker()
{
	ActionBase();

	m_pHM = NULL;
	m_pStream = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_pTarget = NULL;
	m_pTargetX = new kai::Filter();
	m_pTargetY = new kai::Filter();
	m_pTargetW = new kai::Filter();
	m_pTargetH = new kai::Filter();
	m_targetDist = 0;
	m_targetClass = 0;
}

HM_marker::~HM_marker()
{
	DEL(m_pTargetX);
	DEL(m_pTargetY);
	DEL(m_pTargetW);
	DEL(m_pTargetH);
}

bool HM_marker::init(void* pKiss)
{
	CHECK_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("targetClass", &m_targetClass));

	int param = 3;
	F_INFO(pK->v("filterLen", &param));
	m_pTargetX->startMedian(param);
	m_pTargetY->startMedian(param);
	m_pTargetW->startMedian(param);
	m_pTargetH->startMedian(param);

	return true;
}

bool HM_marker::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;
	string iName = "";

	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	F_INFO(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

#ifdef USE_TENSORRT
	F_INFO(pK->v("_DetectNet", &iName));
	m_pDetector = (_DetectNet*) (pK->root()->getChildInstByName(&iName));
#elif defined USE_SSD
	F_INFO(pK->v("_SSD", &iName));
	m_pDetector = (_SSD*) (pK->root()->getChildInstByName(&iName));
#endif

	return true;
}

void HM_marker::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pStream);
//	NULL_(m_pDetector);
	CHECK_(m_pAM->getCurrentStateIdx() != m_iActiveState);

	//get visual target and decide motion
//	m_pTarget = m_pUniv->getByClass(m_targetClass);
//
//	if (m_pTarget == NULL)
//	{
//		//no target found, stop and standby TODO: go back to work
//		m_pHM->m_motorPwmL = 0;
//		m_pHM->m_motorPwmR = 0;
//		m_pHM->m_bSpeaker = false;
//	}
//	else
//	{
//		m_pTargetX->input(m_pTarget->m_bbox.midX());
//		m_pTargetY->input(m_pTarget->m_bbox.midY());
//		m_pTargetArea->input(m_pTarget->m_bbox.area());
//
//		//forward or backward
//		int rpmSpeed = (m_destArea*m_pTarget->m_camSize.area() - m_pTargetArea->v()) * m_speedP;
//
//		//steering
//		int rpmSteer = (m_destX*m_pTarget->m_camSize.m_x - m_pTargetX->v()) * m_steerP;
//
//		m_pHM->m_motorPwmL = rpmSpeed - rpmSteer;
//		m_pHM->m_motorPwmR = rpmSpeed + rpmSteer;
//		m_pHM->m_bSpeaker = true;
//	}

}

bool HM_marker::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;

	string msg = "HM: rpmL=" + i2str(m_pHM->m_motorPwmL) + ", rpmR="
			+ i2str(m_pHM->m_motorPwmR);
	pWin->addMsg(&msg);

	CHECK_T(m_pTarget==NULL);
	Mat* pMat = pWin->getFrame()->getCMat();
	circle(*pMat, Point(m_pTarget->m_bbox.midX(), m_pTarget->m_bbox.midY()), 10,
			Scalar(0, 0, 255), 2);

	return true;
}

}
