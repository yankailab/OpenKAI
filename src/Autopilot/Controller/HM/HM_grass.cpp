#include "HM_grass.h"

namespace kai
{

HM_grass::HM_grass()
{
	m_pDet = NULL;
	m_pHM = NULL;
	m_rpmSteer = 0;
	m_dir = 0;

	m_grassBoxL.init();
	m_grassBoxF.init();
	m_grassBoxR.init();

	m_iGrassClass = 1;
	m_pGrassL = NULL;
	m_pGrassF = NULL;
	m_pGrassR = NULL;

}

HM_grass::~HM_grass()
{
}

bool HM_grass::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, rpmSteer);
	KISSm(pK, iGrassClass);

	Kiss* pG;

	pG = pK->o("grassBoxL");
	IF_F(pG->empty());
	F_INFO(pG->v("l", &m_grassBoxL.x));
	F_INFO(pG->v("t", &m_grassBoxL.y));
	F_INFO(pG->v("r", &m_grassBoxL.z));
	F_INFO(pG->v("b", &m_grassBoxL.w));

	pG = pK->o("grassBoxF");
	IF_F(pG->empty());
	F_INFO(pG->v("l", &m_grassBoxF.x));
	F_INFO(pG->v("t", &m_grassBoxF.y));
	F_INFO(pG->v("r", &m_grassBoxF.z));
	F_INFO(pG->v("b", &m_grassBoxF.w));

	pG = pK->o("grassBoxR");
	IF_F(pG->empty());
	F_INFO(pG->v("l", &m_grassBoxR.x));
	F_INFO(pG->v("t", &m_grassBoxR.y));
	F_INFO(pG->v("r", &m_grassBoxR.z));
	F_INFO(pG->v("b", &m_grassBoxR.w));

	return true;
}

bool HM_grass::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_DetectorBase", &iName));
	m_pDet = (_ObjectBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pDet,"_DetectorBase not found");

	OBJECT gO;

	gO.init();
	gO.m_bb = m_grassBoxL;
	m_pGrassL = m_pDet->add(&gO);
	NULL_F(m_pGrassL);

	gO.init();
	gO.m_bb = m_grassBoxF;
	m_pGrassF = m_pDet->add(&gO);
	NULL_F(m_pGrassF);

	gO.init();
	gO.m_bb = m_grassBoxR;
	m_pGrassR = m_pDet->add(&gO);
	NULL_F(m_pGrassR);

	m_pDet->m_obj.update();

	return true;
}

void HM_grass::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pDet);
	IF_(!isActive());
	IF_(m_iPriority < m_pHM->m_iPriority);

	//standby until Detector is ready
	if(m_pGrassF->m_topClass < 0)
	{
		m_pHM->m_rpmL = 0;
		m_pHM->m_rpmR = 0;
		return;
	}

	//on grass, do nothing
	if(m_pGrassF->bClass(m_iGrassClass))
	{
		m_dir = 0;
		return;
	}

	//already in turn, do nothing
	if(m_pHM->m_rpmL != m_pHM->m_rpmR)
	{
		m_dir = 0;
		return;
	}

	//make new decision on direction
	if(m_dir == 0)
	{
		m_dir = (m_pGrassL->bClass(m_iGrassClass))?1:-1;
	}

	m_rpmSteer = abs(m_rpmSteer) * m_dir;
	m_pHM->m_rpmL = m_rpmSteer;
	m_pHM->m_rpmR = -m_rpmSteer;
}

void HM_grass::bSetActive(bool bActive)
{
}

bool HM_grass::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	NULL_F(pMat);
	IF_F(pMat->empty());

	//draw messages
	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName();
	pWin->addMsg(&msg);

	NULL_T(m_pDet);

	Rect r;
	Scalar col;
	int bold = 2;
	vInt2 cSize;
	cSize.x = pMat->cols;
	cSize.y = pMat->rows;

	vInt42rect(m_pGrassL->iBBox(cSize), r);
	col = Scalar(200, 200, 200);
	if (m_pGrassL->bClass(m_iGrassClass))
		col = Scalar(0, 255, 0);
	rectangle(*pMat, r, col, bold);

	vInt42rect(m_pGrassF->iBBox(cSize), r);
	col = Scalar(200, 200, 200);
	if (m_pGrassF->bClass(m_iGrassClass))
		col = Scalar(0, 255, 0);
	rectangle(*pMat, r, col, bold);

	vInt42rect(m_pGrassR->iBBox(cSize), r);
	col = Scalar(200, 200, 200);
	if (m_pGrassR->bClass(m_iGrassClass))
		col = Scalar(0, 255, 0);
	rectangle(*pMat, r, col, bold);

	return true;
}

}
