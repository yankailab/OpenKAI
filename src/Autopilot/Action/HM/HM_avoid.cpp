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

	m_avoidRegion.m_x = 0.0;
	m_avoidRegion.m_y = 0.0;
	m_avoidRegion.m_z = 1.0;
	m_avoidRegion.m_w = 1.0;

	m_avoidMinSize = 0.0;
	m_alertDist = 0.0;
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

	F_INFO(pK->v("avoidLeft", &m_avoidRegion.m_x));
	F_INFO(pK->v("avoidRight", &m_avoidRegion.m_w));
	F_INFO(pK->v("avoidTop", &m_avoidRegion.m_y));
	F_INFO(pK->v("avoidBottom", &m_avoidRegion.m_z));
	F_INFO(pK->v("avoidMinSize", &m_avoidMinSize));
	F_INFO(pK->v("alertDist", &m_alertDist));

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
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pStream);

	double dist;	//normalized relevant distance: 0.0 ~ 1.0
	double objSize = m_avoidMinSize;

	CHECK_(!m_pStream->distNearest(&m_avoidRegion, &dist, &objSize));

	//forward speed
	int rpmSpeed = dist * m_speedP;

	m_pHM->m_bSpeaker = false;

	//make turn when object is within a certain distance
	int rpmSteer = 0;
	if(dist <= m_alertDist)
	{
		rpmSteer = ((m_alertDist - dist)/m_alertDist)* m_steerP;
		m_pHM->m_bSpeaker = true;
	}

	m_pHM->m_motorPwmL = rpmSpeed - rpmSteer;
	m_pHM->m_motorPwmR = rpmSpeed + rpmSteer;

	m_pHM->updateCAN();

	//Obstacle avoidance always on except for ChargeStation
//	if(m_pAM->getCurrentStateIdx() != m_iActiveState)
//	{
//		//Working state
//	}
//	else
//	{
//		//RTH
//	}

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

	return true;
}

}
