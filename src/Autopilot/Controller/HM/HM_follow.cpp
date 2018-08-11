#include "HM_follow.h"

namespace kai
{

HM_follow::HM_follow()
{
	m_pHM = NULL;
	m_pDet = NULL;

	m_rpmSteer = 0;
	m_rpmT = 0.0;

	m_obj.init();
	m_mTargetClass = 0;
	m_targetX = 0.5;
	m_rTargetX = 0.05;
	m_minSize = 0.0;
	m_maxSize = 0.5;
}

HM_follow::~HM_follow()
{
}

bool HM_follow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,rpmSteer);
	KISSm(pK,rpmT);
	KISSm(pK,targetX);
	KISSm(pK,rTargetX);
	KISSm(pK,minSize);
	KISSm(pK,maxSize);

	int pTargetList[OBJECT_N_CLASS];
	int nTarget = pK->array("targetList", pTargetList, OBJECT_N_CLASS);
	m_mTargetClass = 0;
	for(int i=0; i<nTarget; i++)
	{
		m_mTargetClass |= (1 << pTargetList[i]);
	}

	return true;
}

bool HM_follow::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_DetectorBase", &iName));
	m_pDet = (_ObjectBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_follow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pDet);
	IF_(!isActive());
	IF_(m_iPriority < m_pHM->m_iPriority);

	//standby until Detector is ready
	if(!m_pDet->bReady())
	{
		m_pHM->m_rpmL = 0;
		m_pHM->m_rpmR = 0;
		return;
	}

	m_obj.init();
	for(int i=0; i<m_pDet->size(); i++)
	{
		OBJECT* pO = m_pDet->at(i);
		IF_CONT(!pO->bClassMask(m_mTargetClass));
		IF_CONT(pO->m_bb.area() < m_minSize);
		IF_CONT(pO->m_bb.area() > m_maxSize);
		IF_CONT(pO->m_bb.area() < m_obj.m_bb.area());

		m_obj = *pO;
	}

	IF_(m_obj.m_topClass < 0);

	double pX = m_targetX - m_obj.m_bb.midX();
	if(abs(pX) > m_rTargetX)
	{
		int rpmSteer = m_rpmSteer * pX;
		m_pHM->m_rpmL = -rpmSteer;
		m_pHM->m_rpmR = rpmSteer;
		m_pHM->m_bSpeaker = true;
	}
}

bool HM_follow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName();
	pWin->addMsg(&msg);

	IF_T(!m_obj.bClassMask(m_mTargetClass));

	vInt2 cSize;
	cSize.x = pMat->cols;
	cSize.y = pMat->rows;
	Rect r;
	vInt42rect(m_obj.iBBox(cSize), r);
	rectangle(*pMat, r, Scalar(0, 255, 0), 10);

	return true;
}

}
