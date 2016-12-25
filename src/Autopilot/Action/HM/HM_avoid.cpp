#include "HM_avoid.h"

namespace kai
{

HM_avoid::HM_avoid()
{
	ActionBase();

	m_pHM = NULL;
	m_pStream = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;
	m_pwmL = 0;
	m_pwmR = 0;

	m_avoidRegion.m_x = 0.0;
	m_avoidRegion.m_y = 0.0;
	m_avoidRegion.m_z = 1.0;
	m_avoidRegion.m_w = 1.0;
	m_avoidMinSize = 0.0;
	m_alertDist = 0.0;

	m_pFdist = NULL;
	m_pObj = NULL;
}

HM_avoid::~HM_avoid()
{
	DEL(m_pFdist);
}

bool HM_avoid::init(void* pKiss)
{
	CHECK_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));

	F_INFO(pK->v("avoidLeft", &m_avoidRegion.m_x));
	F_INFO(pK->v("avoidRight", &m_avoidRegion.m_z));
	F_INFO(pK->v("avoidTop", &m_avoidRegion.m_y));
	F_INFO(pK->v("avoidBottom", &m_avoidRegion.m_w));
	F_INFO(pK->v("avoidMinSize", &m_avoidMinSize));
	F_INFO(pK->v("alertDist", &m_alertDist));

	int filterLen = 3;
	F_INFO(pK->v("depthFilterLen", &filterLen));
	m_pFdist = new Filter();
	m_pFdist->startMedian(filterLen);

	m_pObj = new Object();

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

	//forward speed
	int rpmSpeed = m_speedP;
	int rpmSteer = 0;

	m_pObj->reset();
	m_pStream->findObjects(&m_avoidRegion, m_pObj, m_alertDist, m_avoidMinSize);

	//make turn when object is within a certain distance
	if(m_pObj->size()>0)
	{
		rpmSpeed = 0;
		rpmSteer = m_steerP;
		m_pHM->m_bSpeaker = true;
	}

	m_pwmL = rpmSpeed + rpmSteer;
	m_pwmR = rpmSpeed - rpmSteer;
	m_pHM->m_motorPwmL = m_pwmL;
	m_pHM->m_motorPwmR = m_pwmR;

/*
	double objSize = m_avoidMinSize;
	static double objDist = 0;

	m_pStream->distNearest(&m_avoidRegion, &objDist, &objSize);
	m_pFdist->input(objDist);
	objDist = m_pFdist->v();

	//forward speed
	int rpmSpeed = objDist * m_speedP;

	//make turn when object is within a certain distance
	int rpmSteer = 0;
	if(objDist <= m_alertDist)
	{
		rpmSteer = ((m_alertDist - objDist)/m_alertDist)* m_steerP;
		m_pHM->m_bSpeaker = true;
	}

	m_pwmL = rpmSpeed + rpmSteer;
	m_pwmR = rpmSpeed - rpmSteer;
	m_pHM->m_motorPwmL = m_pwmL;
	m_pHM->m_motorPwmR = m_pwmR;
*/
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
			*this->getName()+": rpmL=" + i2str(m_pwmL) + ", rpmR=" + i2str(m_pwmR),
			*pWin->getTextPos(), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pWin->lineNext();

	Rect r;
	r.x = m_avoidRegion.m_x * ((double)pMat->cols);
	r.y = m_avoidRegion.m_y * ((double)pMat->rows);
	r.width = m_avoidRegion.m_z * ((double)pMat->cols) - r.x;
	r.height = m_avoidRegion.m_w * ((double)pMat->rows) - r.y;

	Scalar col = Scalar(0,255,0);
	int bold = 1;

//	if(m_pFdist->v() < m_alertDist)
	if(m_pObj->size()>0)
	{
		col = Scalar(0,0,255);
		bold = 2;
	}

	rectangle(*pMat, r, col, bold);
//	putText(*pMat, f2str(m_pFdist->v()),
//			Point(r.x + r.width / 2, r.y + r.height / 2),
//			FONT_HERSHEY_SIMPLEX, 1.0, col, bold);

	return true;
}

}
