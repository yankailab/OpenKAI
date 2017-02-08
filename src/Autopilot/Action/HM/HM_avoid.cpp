#include "HM_avoid.h"

namespace kai
{

HM_avoid::HM_avoid()
{
	m_pHM = NULL;
	m_pStream = NULL;
	m_pObs = NULL;
	m_pMarkerDN = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_avoidArea.m_x = 0.0;
	m_avoidArea.m_y = 0.0;
	m_avoidArea.m_z = 1.0;
	m_avoidArea.m_w = 1.0;
	m_alertDist = 0.0;
	m_distM = 0.0;

	m_minProb = 0.5;
	m_objLifetime = 100000;
	m_markerTurnStart = 0;
	m_markerTurnTimer = USEC_1SEC;
	m_rpmSteer = 0;
	m_pMarker = NULL;
}

HM_avoid::~HM_avoid()
{
}

bool HM_avoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));

	F_INFO(pK->v("avoidLeft", &m_avoidArea.m_x));
	F_INFO(pK->v("avoidRight", &m_avoidArea.m_z));
	F_INFO(pK->v("avoidTop", &m_avoidArea.m_y));
	F_INFO(pK->v("avoidBottom", &m_avoidArea.m_w));
	F_INFO(pK->v("alertDist", &m_alertDist));

	F_INFO(pK->v("minProb", &m_minProb));
	F_INFO(pK->v("objLifetime", &m_objLifetime));
	F_INFO(pK->v("markerTurnTimer", &m_markerTurnTimer));

	return true;
}

bool HM_avoid::link(void)
{
	IF_F(!this->ActionBase::link());
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

	iName = "";
	F_INFO(pK->v("_DetectNet", &iName));
	m_pMarkerDN = (_DetectNet*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pStream);
	NULL_(m_pObs);
	IF_(!isActive());

	uint64_t tNow = get_time_usec();

	//keep turning until the time
	if(tNow - m_markerTurnStart < m_markerTurnTimer)
	{
		m_pHM->m_motorPwmL = m_rpmSteer;
		m_pHM->m_motorPwmR = -m_rpmSteer;
		return;
	}
	m_markerTurnStart = 0;

	//do nothing if no obstacle inside alert distance
	m_distM = m_pObs->dist(&m_avoidArea,&m_posMin);
	IF_(m_distM > m_alertDist);

	//speaker alert if object is within a certain distance
	m_pHM->m_bSpeaker = true;

	//see if marker is around
	m_pMarker = NULL;
	uint64_t t = tNow - m_objLifetime;
	int i;
	for(i=0; i<m_pMarkerDN->size(); i++)
	{
		OBJECT* pObj = m_pMarkerDN->get(i, t);
		if(!pObj)continue;
		if(pObj->m_prob < m_minProb)continue;

		m_pMarker = pObj;
		break;
	}

	//if found marker, start turn for the timer duration
	if(m_pMarker)
	{
		m_markerTurnStart = tNow;
	}

	//decide which direction to turn
	m_rpmSteer = m_steerP;
	if(m_pHM->m_motorPwmL < m_pHM->m_motorPwmR)
		m_rpmSteer = -m_steerP;

	m_pHM->m_motorPwmL = m_rpmSteer;
	m_pHM->m_motorPwmR = -m_rpmSteer;
}

bool HM_avoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	string msg;
	if(isActive())msg="* ";
	else msg="- ";
	msg += *this->getName() +": dist=" + f2str(m_distM) + ", markerTurn:" + i2str((int)m_markerTurnStart);
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
