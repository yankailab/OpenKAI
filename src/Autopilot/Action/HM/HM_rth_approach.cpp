#include "HM_rth_approach.h"

namespace kai
{

HM_rth_approach::HM_rth_approach()
{
	m_pHM = NULL;
	m_pAM = NULL;

	m_steerP = 0.0;
	m_pBase = NULL;

}

HM_rth_approach::~HM_rth_approach()
{
}

bool HM_rth_approach::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("targetClass", &m_iBaseClass));

	return true;
}

bool HM_rth_approach::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_rth_approach::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pObs);
	NULL_(m_pAM);
	IF_(!isActive());

	uint64_t tNow = get_time_usec();

	if (m_pMN->bFound(m_iBaseClass))
	{

	}

	m_pHM->m_motorPwmL = m_rpmSteer;
	m_pHM->m_motorPwmR = -m_rpmSteer;

	//decide which direction to turn based on previous actions' decision
//	if (m_rpmSteer == 0)
//	{
//		m_rpmSteer = m_steerP;
//		if (m_pHM->m_motorPwmL < m_pHM->m_motorPwmR)
//		{
//			m_rpmSteer = -m_steerP;
//		}
//		else if (m_pHM->m_motorPwmL == m_pHM->m_motorPwmR)
//		{
//			//decide direction by obstacles in left and right
//			double dL = m_pObs->dist(&m_obsBoxL, NULL);
//			double dR = m_pObs->dist(&m_obsBoxR, NULL);
//
//			if (dL > dR)
//				m_rpmSteer = -m_steerP;
//		}
//	}


}

bool HM_rth_approach::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;

	string msg = "HM: rpmL=" + i2str(m_pHM->m_motorPwmL) + ", rpmR="
			+ i2str(m_pHM->m_motorPwmR);
	pWin->addMsg(&msg);

//	IF_T(m_pTarget==NULL);
//	Mat* pMat = pWin->getFrame()->getCMat();
//	circle(*pMat, Point(m_pTarget->m_bbox.midX(), m_pTarget->m_bbox.midY()), 10,
//			Scalar(0, 0, 255), 2);

	return true;
}

}
