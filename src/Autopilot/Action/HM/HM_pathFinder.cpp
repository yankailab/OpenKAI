#include "HM_pathFinder.h"

namespace kai
{

HM_pathFinder::HM_pathFinder()
{
	m_pHM = NULL;
	m_pObs = NULL;

	m_rpmSteer = 0;
	m_searchArea.m_x = 0.0;
	m_searchArea.m_y = 0.0;
	m_searchArea.m_z = 1.0;
	m_searchArea.m_w = 1.0;
	m_distDiff = 0.0;

	m_turnStart = 0;
	m_turnTimer = USEC_1SEC;
	m_tLastTurn = 0;
	m_turnInterval = USEC_1SEC*10;
}

HM_pathFinder::~HM_pathFinder()
{
}

bool HM_pathFinder::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("steerP", &m_rpmSteer));
	m_rpmSteer = abs(m_rpmSteer);
	F_INFO(pK->v("searchLeft", &m_searchArea.m_x));
	F_INFO(pK->v("searchRight", &m_searchArea.m_z));
	F_INFO(pK->v("searchTop", &m_searchArea.m_y));
	F_INFO(pK->v("searchBottom", &m_searchArea.m_w));
	F_INFO(pK->v("distDiff", &m_distDiff));

	F_INFO(pK->v("turnTimer", &m_turnTimer));
	F_INFO(pK->v("turnInterval", &m_turnInterval));

	return true;
}

bool HM_pathFinder::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_pathFinder::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pObs);
	IF_(!isActive());

	uint64_t tNow = get_time_usec();

	//keep turning until the time limit
	if(tNow - m_turnStart < m_turnTimer)
	{
		//path not found yet, keep turning
		if(!isPathFoundFront())
		{
			m_pHM->m_motorPwmL = m_rpmSteer;
			m_pHM->m_motorPwmR = -m_rpmSteer;
			return;
		}

		//finish turning motion once available space is found
		m_turnStart = 0;
		m_tLastTurn = tNow;
	}
	else if(m_turnStart > 0)
	{
		//recover the direction if no path is found before time limit
		m_pHM->m_motorPwmL = -m_rpmSteer;
		m_pHM->m_motorPwmR = m_rpmSteer;
		m_turnStart = 0;
		m_tLastTurn = tNow;
	}

	//do not turn within interval
	IF_(tNow - m_tLastTurn < m_turnInterval);

	//check side to see if possible path
	if(isPathFoundSide())
	{
		m_turnStart = tNow;
		return;
	}

	//TODO: radomly turn left or right in interval


}

bool HM_pathFinder::isPathFoundFront(void)
{
	//	m_distM = m_pObs->dist(&m_searchArea,&m_posMin);

	return true;
}

bool HM_pathFinder::isPathFoundSide(void)
{

	return true;
}

bool HM_pathFinder::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

//	string msg;
//	if(isActive())msg="* ";
//	else msg="- ";
//	msg += *this->getName() +": dist=" + f2str(m_distM) + ", markerTurn:" + i2str((int)m_markerTurnStart);
//	pWin->addMsg(&msg);
//
//	Rect r;
//	r.x = m_searchArea.m_x * ((double)pMat->cols);
//	r.y = m_searchArea.m_y * ((double)pMat->rows);
//	r.width = m_searchArea.m_z * ((double)pMat->cols) - r.x;
//	r.height = m_searchArea.m_w * ((double)pMat->rows) - r.y;
//
//	Scalar col = Scalar(0,255,0);
//	int bold = 1;
//	if(m_distM < m_distDiff)
//	{
//		col = Scalar(0,0,255);
//		bold = 2;
//	}
//	rectangle(*pMat, r, col, bold);
//
//	NULL_F(m_pObs);
//	vInt2 mDim = m_pObs->matrixDim();
//	circle(*pMat, Point((m_posMin.m_x+0.5)*(pMat->cols/mDim.m_x), (m_posMin.m_y+0.5)*(pMat->rows/mDim.m_y)),
//			0.000025*pMat->cols*pMat->rows,
//			Scalar(0, 255, 255), 2);

	return true;
}

}
