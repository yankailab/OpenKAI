#include "VEK_avoid.h"

namespace kai
{

VEK_avoid::VEK_avoid()
{
	m_pVEK = NULL;
//	m_pObs = NULL;

	m_obsBox.x = 0.0;
	m_obsBox.y = 0.0;
	m_obsBox.z = 1.0;
	m_obsBox.w = 1.0;

	m_dAlert = 0.0;
	m_distM = 0.0;
	m_vSteer = 0.5;
}

VEK_avoid::~VEK_avoid()
{
}

bool VEK_avoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,vSteer);
	KISSm(pK,dAlert);

	Kiss* pG;
	pG = pK->o("obsBox");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_obsBox.x));
	F_INFO(pG->v("top", &m_obsBox.y));
	F_INFO(pG->v("right", &m_obsBox.z));
	F_INFO(pG->v("bottom", &m_obsBox.w));

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("VEK_base", &iName));
	m_pVEK = (VEK_base*) (pK->parent()->getChildInst(iName));

//	iName = "";
//	F_INFO(pK->v("_ZEDobstacle", &iName));
//	m_pObs = (_ZEDdistance*) (pK->root()->getChildInst(iName));

	return true;
}

void VEK_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pVEK);
	NULL_(m_pMC);
//	NULL_(m_pObs);
	IF_(!bActive());

	string stateName = "VEK_AVOID";

//	if(!m_pObs->bReady())
//	{
//		m_pAM->transit(&stateName);
//		m_pVEK->m_vL = 0.0;
//		m_pVEK->m_vR = 0.0;
//		return;
//	}

	//do nothing if no obstacle inside alert distance
//	m_distM = m_pObs->d(&m_obsBox, &m_posMin);
//	IF_(m_distM > m_dAlert);

	m_pMC->transit(stateName);
	m_pVEK->m_vL = -m_vSteer;
	m_pVEK->m_vR = m_vSteer;

}

bool VEK_avoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	//draw messages
	string msg;
	if (bActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName() + ": dist=" + f2str(m_distM);
	pWin->addMsg(&msg);

	//draw obstacle detection boxes
	Rect r;
	r.x = m_obsBox.x * ((double) pMat->cols);
	r.y = m_obsBox.y * ((double) pMat->rows);
	r.width = m_obsBox.z * ((double) pMat->cols) - r.x;
	r.height = m_obsBox.w * ((double) pMat->rows) - r.y;

	Scalar col = Scalar(0, 255, 0);
	int bold = 1;
	if (m_distM < m_dAlert)
	{
		col = Scalar(0, 0, 255);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	//draw obstacle indicator
//	if(m_pObs)
//	{
//		vInt2 mDim = m_pObs->matrixDim();
//		circle(*pMat,
//				Point((m_posMin.x + 0.5) * (pMat->cols / mDim.x),
//						(m_posMin.y + 0.5) * (pMat->rows / mDim.y)),
//				0.000025 * pMat->cols * pMat->rows, Scalar(0, 255, 255), 2);
//	}

	return true;
}

}
