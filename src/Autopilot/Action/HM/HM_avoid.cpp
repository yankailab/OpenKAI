#include "HM_avoid.h"

namespace kai
{

HM_avoid::HM_avoid()
{
	ActionBase();

	m_pHM = NULL;
	m_pAM = NULL;
	m_pStream = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;
}

HM_avoid::~HM_avoid()
{
}

bool HM_avoid::init(void* pKiss)
{
	CHECK_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));

	return true;
}

bool HM_avoid::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;
	string iName = "";

	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	F_INFO(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->parent()->getChildInstByName(&iName));

	return true;
}

void HM_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);

	//Obstacle always on except for ChargeStation
	if(m_pAM->getCurrentStateIdx() != m_iActiveState)
	{
		//Working state
	}
	else
	{
		//RTH
	}

	//no target found, stop and standby TODO: go back to work
	m_pHM->m_motorPwmL = 0;
	m_pHM->m_motorPwmR = 0;
	m_pHM->m_bSpeaker = false;

//	//forward or backward
//	int rpmSpeed = (m_destArea*m_pTarget->m_camSize.area() - m_pTargetArea->v()) * m_speedP;
//	//steering
//	int rpmSteer = (m_destX*m_pTarget->m_camSize.m_x - m_pTargetX->v()) * m_steerP;

//	m_pHM->m_motorPwmL = rpmSpeed - rpmSteer;
//	m_pHM->m_motorPwmR = rpmSpeed + rpmSteer;
//	m_pHM->m_bSpeaker = true;


	m_pHM->updateCAN();
}

bool HM_avoid::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	putText(*pMat,
			*this->getName()+": rpmL=" + i2str(m_pHM->m_motorPwmL) + ", rpmR="
					+ i2str(m_pHM->m_motorPwmR),
			*pWin->getTextPos(), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pWin->lineNext();

//	CHECK_T(m_pTarget==NULL);
//	circle(*pMat, Point(m_pTarget->m_bbox.midX(), m_pTarget->m_bbox.midY()), 10,
//			Scalar(0, 0, 255), 2);

	return true;
}

}
