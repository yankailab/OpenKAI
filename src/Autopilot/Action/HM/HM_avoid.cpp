#include "HM_avoid.h"

namespace kai
{

HM_avoid::HM_avoid()
{
	m_pHM = NULL;
	m_pStream = NULL;
	m_pObs = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_avoidArea.m_x = 0.0;
	m_avoidArea.m_y = 0.0;
	m_avoidArea.m_z = 1.0;
	m_avoidArea.m_w = 1.0;
	m_alertDist = 0.0;
	m_distM = 0.0;
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

	F_INFO(pK->v("avoidLeft", &m_avoidArea.m_x));
	F_INFO(pK->v("avoidRight", &m_avoidArea.m_z));
	F_INFO(pK->v("avoidTop", &m_avoidArea.m_y));
	F_INFO(pK->v("avoidBottom", &m_avoidArea.m_w));
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

	iName = "";
	F_INFO(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pStream);
	NULL_(m_pObs);

	//do nothing if no obstacle inside alert distance
	m_distM = m_pObs->dist(&m_avoidArea,&m_posMin);
	CHECK_(m_distM > m_alertDist);

	//make turn when object is within a certain distance
	m_pHM->m_bSpeaker = true;

	CHECK_(!isActive());

	int rpmSteer = m_steerP;
	if(m_pHM->m_motorPwmL < m_pHM->m_motorPwmR)
		rpmSteer = -m_steerP;

	m_pHM->m_motorPwmL = rpmSteer;
	m_pHM->m_motorPwmR = -rpmSteer;
}

bool HM_avoid::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	CHECK_F(pMat->empty());

	string msg;
	if(isActive())msg="* ";
	else msg="- ";
	msg += *this->getName() +": dist=" + f2str(m_distM);
	pWin->addMsg(&msg);

	Rect r;
	r.x = m_avoidArea.m_x * ((double)pMat->cols);
	r.y = m_avoidArea.m_y * ((double)pMat->rows);
	r.width = m_avoidArea.m_z * ((double)pMat->cols) - r.x;
	r.height = m_avoidArea.m_w * ((double)pMat->rows) - r.y;

	Scalar col = Scalar(0,255,0);
	int bold = 1;
	if(m_distM < m_alertDist)
	{
		col = Scalar(0,0,255);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	NULL_F(m_pObs);
	vInt2 mDim = m_pObs->matrixDim();
	circle(*pMat, Point((m_posMin.m_x+0.5)*(pMat->cols/mDim.m_x), (m_posMin.m_y+0.5)*(pMat->rows/mDim.m_y)),
			0.000025*pMat->cols*pMat->rows,
			Scalar(0, 255, 255), 2);

	return true;
}

}
