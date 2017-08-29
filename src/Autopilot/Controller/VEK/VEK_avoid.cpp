#include "VEK_avoid.h"

namespace kai
{

VEK_avoid::VEK_avoid()
{
	m_pVEK = NULL;
	m_pObs = NULL;
	m_iMarkerClass = -1;

	m_obsBoxF.x = 0.0;
	m_obsBoxF.y = 0.0;
	m_obsBoxF.z = 1.0;
	m_obsBoxF.w = 1.0;
	m_obsBoxL.init();
	m_obsBoxR.init();

	m_alertDist = 0.0;
	m_distM = 0.0;
	m_rpmSteer = 0;

}

VEK_avoid::~VEK_avoid()
{
}

bool VEK_avoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("rpmSteer", &m_rpmSteer));
	F_INFO(pK->v("alertDist", &m_alertDist));

	F_INFO(pK->v("iMarkerClass", &m_iMarkerClass));

	Kiss* pG;

	pG = pK->o("obsBoxL");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_obsBoxL.x));
	F_INFO(pG->v("top", &m_obsBoxL.y));
	F_INFO(pG->v("right", &m_obsBoxL.z));
	F_INFO(pG->v("bottom", &m_obsBoxL.w));

	pG = pK->o("obsBoxF");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_obsBoxF.x));
	F_INFO(pG->v("top", &m_obsBoxF.y));
	F_INFO(pG->v("right", &m_obsBoxF.z));
	F_INFO(pG->v("bottom", &m_obsBoxF.w));

	pG = pK->o("obsBoxR");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_obsBoxR.x));
	F_INFO(pG->v("top", &m_obsBoxR.y));
	F_INFO(pG->v("right", &m_obsBoxR.z));
	F_INFO(pG->v("bottom", &m_obsBoxR.w));

	return true;
}

bool VEK_avoid::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("VEK_base", &iName));
	m_pVEK = (VEK_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void VEK_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pVEK);
	NULL_(m_pAM);
	NULL_(m_pObs);
	if(!isActive())
	{
		return;
	}

	uint64_t tNow = get_time_usec();

	//do nothing if no obstacle inside alert distance
	m_distM = m_pObs->d(&m_obsBoxF, &m_posMin);
	IF_(m_distM > m_alertDist);

	//decide direction by obstacles in left and right
	double dL = m_pObs->d(&m_obsBoxL, NULL);
	double dR = m_pObs->d(&m_obsBoxR, NULL);

	m_rpmSteer = abs(m_rpmSteer);
	if (dL > dR)
			m_rpmSteer *= -1;

}

bool VEK_avoid::draw(void)
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
	msg += *this->getName() + ": dist=" + f2str(m_distM);
	pWin->addMsg(&msg);

	//draw obstacle detection boxes
	Rect r;
	r.x = m_obsBoxF.x * ((double) pMat->cols);
	r.y = m_obsBoxF.y * ((double) pMat->rows);
	r.width = m_obsBoxF.z * ((double) pMat->cols) - r.x;
	r.height = m_obsBoxF.w * ((double) pMat->rows) - r.y;

	Scalar col = Scalar(0, 255, 0);
	int bold = 1;
	if (m_distM < m_alertDist)
	{
		col = Scalar(0, 0, 255);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	r.x = m_obsBoxL.x * ((double) pMat->cols);
	r.y = m_obsBoxL.y * ((double) pMat->rows);
	r.width = m_obsBoxL.z * ((double) pMat->cols) - r.x;
	r.height = m_obsBoxL.w * ((double) pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0, 255, 0), 1);

	r.x = m_obsBoxR.x * ((double) pMat->cols);
	r.y = m_obsBoxR.y * ((double) pMat->rows);
	r.width = m_obsBoxR.z * ((double) pMat->cols) - r.x;
	r.height = m_obsBoxR.w * ((double) pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0, 255, 0), 1);

	//draw obstacle indicator
	if(m_pObs)
	{
		vInt2 mDim = m_pObs->matrixDim();
		circle(*pMat,
				Point((m_posMin.x + 0.5) * (pMat->cols / mDim.x),
						(m_posMin.y + 0.5) * (pMat->rows / mDim.y)),
				0.000025 * pMat->cols * pMat->rows, Scalar(0, 255, 255), 2);
	}

	return true;
}

}
