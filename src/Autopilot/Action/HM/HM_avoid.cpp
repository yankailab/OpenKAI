#include "HM_avoid.h"

namespace kai
{

HM_avoid::HM_avoid()
{
	m_pHM = NULL;
	m_pObs = NULL;
	m_pMN = NULL;
	m_iMarkerClass = -1;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_obsBoxF.m_x = 0.0;
	m_obsBoxF.m_y = 0.0;
	m_obsBoxF.m_z = 1.0;
	m_obsBoxF.m_w = 1.0;
	m_obsBoxL.init();
	m_obsBoxR.init();

	m_alertDist = 0.0;
	m_distM = 0.0;

	m_objLifetime = 100000;
	m_markerTurnStart = 0;
	m_markerTurnTimer = USEC_1SEC;
	m_rpmSteer = 0;

}

HM_avoid::~HM_avoid()
{
}

bool HM_avoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("alertDist", &m_alertDist));

	F_INFO(pK->v("objLifetime", &m_objLifetime));
	F_INFO(pK->v("markerTurnTimer", &m_markerTurnTimer));
	F_INFO(pK->v("iMarkerClass", &m_iMarkerClass));

	Kiss* pG;

	pG = pK->o("obsBoxL");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_obsBoxL.m_x));
	F_INFO(pG->v("top", &m_obsBoxL.m_y));
	F_INFO(pG->v("right", &m_obsBoxL.m_z));
	F_INFO(pG->v("bottom", &m_obsBoxL.m_w));

	pG = pK->o("obsBoxF");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_obsBoxF.m_x));
	F_INFO(pG->v("top", &m_obsBoxF.m_y));
	F_INFO(pG->v("right", &m_obsBoxF.m_z));
	F_INFO(pG->v("bottom", &m_obsBoxF.m_w));

	pG = pK->o("obsBoxR");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_obsBoxR.m_x));
	F_INFO(pG->v("top", &m_obsBoxR.m_y));
	F_INFO(pG->v("right", &m_obsBoxR.m_z));
	F_INFO(pG->v("bottom", &m_obsBoxR.m_w));

	return true;
}

bool HM_avoid::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_MatrixNet", &iName));
	m_pMN = (_MatrixNet*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pObs);
	NULL_(m_pMN);
	IF_(!isActive());

	uint64_t tNow = get_time_usec();

	//keep turning until the time
	if (tNow - m_markerTurnStart < m_markerTurnTimer)
	{
		m_pHM->m_motorPwmL = m_rpmSteer;
		m_pHM->m_motorPwmR = -m_rpmSteer;
		return;
	}
	m_markerTurnStart = 0;

	//do nothing if no obstacle inside alert distance
	m_distM = m_pObs->dist(&m_obsBoxF, &m_posMin);
	if (m_distM > m_alertDist)
	{
		m_rpmSteer = 0;
		return;
	}

	//speaker alert if object is within a certain distance
	m_pHM->m_bSpeaker = true;

	//if found marker, start turn for the timer duration
	if (m_pMN->bFound(m_iMarkerClass))
		m_markerTurnStart = tNow;

	//decide which direction to turn based on previous actions' decision
	if (m_rpmSteer == 0)
	{
		m_rpmSteer = m_steerP;
		if (m_pHM->m_motorPwmL < m_pHM->m_motorPwmR)
		{
			m_rpmSteer = -m_steerP;
		}
		else if (m_pHM->m_motorPwmL == m_pHM->m_motorPwmR)
		{
			//decide direction by obstacles in left and right
			double dL = m_pObs->dist(&m_obsBoxL, NULL);
			double dR = m_pObs->dist(&m_obsBoxR, NULL);

			if (dL > dR)
				m_rpmSteer = -m_steerP;
		}
	}

	m_pHM->m_motorPwmL = m_rpmSteer;
	m_pHM->m_motorPwmR = -m_rpmSteer;
}

bool HM_avoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	//draw messages
	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName() + ": dist=" + f2str(m_distM) + ", markerTurn:"
			+ i2str((int) m_markerTurnStart);
	pWin->addMsg(&msg);

	//draw obstacle detection boxes
	Rect r;
	r.x = m_obsBoxF.m_x * ((double) pMat->cols);
	r.y = m_obsBoxF.m_y * ((double) pMat->rows);
	r.width = m_obsBoxF.m_z * ((double) pMat->cols) - r.x;
	r.height = m_obsBoxF.m_w * ((double) pMat->rows) - r.y;

	Scalar col = Scalar(0, 255, 0);
	int bold = 1;
	if (m_distM < m_alertDist)
	{
		col = Scalar(0, 0, 255);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	r.x = m_obsBoxL.m_x * ((double) pMat->cols);
	r.y = m_obsBoxL.m_y * ((double) pMat->rows);
	r.width = m_obsBoxL.m_z * ((double) pMat->cols) - r.x;
	r.height = m_obsBoxL.m_w * ((double) pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0, 255, 0), 1);

	r.x = m_obsBoxR.m_x * ((double) pMat->cols);
	r.y = m_obsBoxR.m_y * ((double) pMat->rows);
	r.width = m_obsBoxR.m_z * ((double) pMat->cols) - r.x;
	r.height = m_obsBoxR.m_w * ((double) pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0, 255, 0), 1);

	//draw obstacle indicator
	if(m_pObs)
	{
		vInt2 mDim = m_pObs->matrixDim();
		circle(*pMat,
				Point((m_posMin.m_x + 0.5) * (pMat->cols / mDim.m_x),
						(m_posMin.m_y + 0.5) * (pMat->rows / mDim.m_y)),
				0.000025 * pMat->cols * pMat->rows, Scalar(0, 255, 255), 2);
	}

	return true;
}

}
