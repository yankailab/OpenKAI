#include "HM_avoid.h"

namespace kai
{

HM_avoid::HM_avoid()
{
	m_pHM = NULL;
	m_pDV = NULL;

	m_dROIf.x = 0.0;
	m_dROIf.y = 0.0;
	m_dROIf.z = 1.0;
	m_dROIf.w = 1.0;
	m_dROIl.init();
	m_dROIr.init();

	m_alertDist = 0.0;
	m_distM = 0.0;
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

	F_INFO(pK->v("rpmSteer", &m_rpmSteer));
	F_INFO(pK->v("alertDist", &m_alertDist));

	Kiss* pG;

	pG = pK->o("dROIl");
	IF_F(pG->empty());
	F_INFO(pG->v("l", &m_dROIl.x));
	F_INFO(pG->v("t", &m_dROIl.y));
	F_INFO(pG->v("r", &m_dROIl.z));
	F_INFO(pG->v("b", &m_dROIl.w));

	pG = pK->o("dROIf");
	IF_F(pG->empty());
	F_INFO(pG->v("l", &m_dROIf.x));
	F_INFO(pG->v("t", &m_dROIf.y));
	F_INFO(pG->v("r", &m_dROIf.z));
	F_INFO(pG->v("b", &m_dROIf.w));

	pG = pK->o("dROIr");
	IF_F(pG->empty());
	F_INFO(pG->v("l", &m_dROIr.x));
	F_INFO(pG->v("t", &m_dROIr.y));
	F_INFO(pG->v("r", &m_dROIr.z));
	F_INFO(pG->v("b", &m_dROIr.w));

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pDV);
	IF_(m_iPriority < m_pHM->m_iPriority);
	IF_(!isActive());

	uint64_t tNow = getTimeUsec();

	//do nothing if no obstacle inside alert distance
	m_distM = m_pDV->d(&m_dROIf, &m_posMin);
	IF_(m_distM > m_alertDist);

	m_rpmSteer = abs(m_rpmSteer);
	m_pHM->m_rpmL = m_rpmSteer;
	m_pHM->m_rpmR = -m_rpmSteer;
	m_pHM->m_bSpeaker = true;

}

bool HM_avoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
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
	r.x = m_dROIf.x * ((double) pMat->cols);
	r.y = m_dROIf.y * ((double) pMat->rows);
	r.width = m_dROIf.z * ((double) pMat->cols) - r.x;
	r.height = m_dROIf.w * ((double) pMat->rows) - r.y;

	Scalar col = Scalar(0, 255, 0);
	int bold = 1;
	if (m_distM < m_alertDist)
	{
		col = Scalar(0, 0, 255);
		bold = 10;
	}
	rectangle(*pMat, r, col, bold);

	r.x = m_dROIl.x * ((double) pMat->cols);
	r.y = m_dROIl.y * ((double) pMat->rows);
	r.width = m_dROIl.z * ((double) pMat->cols) - r.x;
	r.height = m_dROIl.w * ((double) pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0, 255, 0), 1);

	r.x = m_dROIr.x * ((double) pMat->cols);
	r.y = m_dROIr.y * ((double) pMat->rows);
	r.width = m_dROIr.z * ((double) pMat->cols) - r.x;
	r.height = m_dROIr.w * ((double) pMat->rows) - r.y;
	rectangle(*pMat, r, Scalar(0, 255, 0), 1);

	//draw obstacle indicator
	if(m_pDV)
	{
		vInt2 mDim = m_pDV->matrixDim();
		circle(*pMat,
				Point((m_posMin.x + 0.5) * (pMat->cols / mDim.x),
						(m_posMin.y + 0.5) * (pMat->rows / mDim.y)),
				0.000025 * pMat->cols * pMat->rows, Scalar(0, 255, 255), 2);
	}

	return true;
}

}
