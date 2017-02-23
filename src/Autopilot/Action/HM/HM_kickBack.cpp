#include "HM_kickBack.h"

namespace kai
{

HM_kickBack::HM_kickBack()
{
	m_pHM = NULL;
	m_pObs = NULL;
	m_pPath = NULL;
	m_rpmBack = 0;

	m_obsBox.init();
	m_kickBackDist = 0.0;
	m_distM = 0.0;

	reset();
}

HM_kickBack::~HM_kickBack()
{
}

void HM_kickBack::reset(void)
{
	m_wpStation.init();
	m_wpApproach.init();
	m_bSetStation = false;
}

bool HM_kickBack::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("rpmBack", &m_rpmBack));
	F_INFO(pK->v("kickBackDist", &m_kickBackDist));

	Kiss* pG;

	pG = pK->o("obsBox");
	IF_T(pG->empty());
	F_INFO(pG->v("left", &m_obsBox.m_x));
	F_INFO(pG->v("top", &m_obsBox.m_y));
	F_INFO(pG->v("right", &m_obsBox.m_z));
	F_INFO(pG->v("bottom", &m_obsBox.m_w));

	return true;
}

bool HM_kickBack::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Path", &iName));
	m_pPath = (_Path*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_kickBack::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pObs);
	NULL_(m_pPath);
	IF_(!isActive());

	if(!m_bSetStation)
	{
		//remember station position
		UTM_POS* pWP = m_pPath->getCurrentPos();
		NULL_(pWP);
		m_wpStation = *pWP;
		m_bSetStation = true;
	}

	UTM_POS* pNew = m_pPath->getCurrentPos();
	NULL_(pNew);
	double d = pNew->dist(&m_wpStation);

	if(d >= m_kickBackDist)
	{
		//arrived at approach position
		m_wpStation = *pNew;
		string stateName = "HM_WORK";
		m_pAM->transit(m_pAM->getStateIdx(&stateName));
		return;
	}

	m_pHM->m_motorPwmL = m_rpmBack;
	m_pHM->m_motorPwmR = m_rpmBack;
}

bool HM_kickBack::draw(void)
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
	r.x = m_obsBox.m_x * ((double) pMat->cols);
	r.y = m_obsBox.m_y * ((double) pMat->rows);
	r.width = m_obsBox.m_z * ((double) pMat->cols) - r.x;
	r.height = m_obsBox.m_w * ((double) pMat->rows) - r.y;

	Scalar col = Scalar(0, 255, 0);
	int bold = 1;
	if (m_distM >= m_kickBackDist)
	{
		col = Scalar(0, 0, 255);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	return true;
}

}
