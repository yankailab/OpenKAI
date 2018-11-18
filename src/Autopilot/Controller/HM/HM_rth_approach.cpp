#include "HM_rth_approach.h"

namespace kai
{

HM_rth_approach::HM_rth_approach()
{
	m_pHM = NULL;
	m_pMN = NULL;

	m_rpmSteer = 0.0;
	m_rpmT = 0.0;
	m_targetX = 0.5;
	m_rTargetX = 0.05;
	m_pTarget = NULL;
	m_iTargetClass = 0;
	m_targetName = "";

}

HM_rth_approach::~HM_rth_approach()
{
}

bool HM_rth_approach::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	F_INFO(pK->v("rpmT", &m_rpmT));
	F_INFO(pK->v("rpmSteer", &m_rpmSteer));
	F_INFO(pK->v("targetX", &m_targetX));
	F_INFO(pK->v("rTargetX", &m_rTargetX));
	F_INFO(pK->v("iTargetClass", &m_iTargetClass));
	F_INFO(pK->v("targetName", &m_targetName));

	//link
	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInst(iName));

	iName = "";
	F_INFO(pK->v("_MatrixNet", &iName));
	m_pMN = (_ClusterNet*) (pK->root()->getChildInst(iName));

	if (!m_pMN)
	{
		LOG_E("_MatrixNet not found");
		return false;
	}

	return true;
}

void HM_rth_approach::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pMC);
	NULL_(m_pMN);
	IF_(m_iPriority < m_pHM->m_iPriority);

	if(!isActive())
	{
		m_pMN->goSleep();
		return;
	}

	m_pMN->wakeUp();

	m_pTarget = NULL;
	OBJECT* pO;
	int i=0;
	while((pO = m_pMN->at(i++)) != NULL)
	{
		if(m_targetName=="")
		{
			IF_CONT(pO->m_topClass != m_iTargetClass);
		}
		else
		{
//			IF_CONT(pO->m_name != m_targetName);
		}

		if (!m_pTarget)
		{
			m_pTarget = pO;
			continue;
		}

//		if (m_pTarget->m_prob < pO->m_prob)
//		{
//			m_pTarget = pO;
//			continue;
//		}
	}

	if(!m_pTarget)
	{
//		m_pHM->m_rpmL = m_rpmSteer;
//		m_pHM->m_rpmR = -m_rpmSteer;
		return;
	}

	m_pHM->m_bSpeaker = true;

	double pX = m_targetX - m_pTarget->m_bb.midX();
	if(abs(pX) > m_rTargetX)
	{
		int rpmSteer = m_rpmSteer * pX;
//		m_pHM->m_rpmL = -rpmSteer;
//		m_pHM->m_rpmR = rpmSteer;
		return;
	}

//	m_pHM->m_rpmL = m_rpmT;
//	m_pHM->m_rpmR = m_rpmT;
}

bool HM_rth_approach::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName();

	if(m_pTarget)
		msg += ": dist=" + f2str(m_pTarget->m_dist);
	pWin->addMsg(&msg);

	NULL_T(m_pTarget);
	vInt2 cSize;
	cSize.x = pMat->cols;
	cSize.y = pMat->rows;
	Rect r;
	vInt42rect(m_pTarget->iBBox(cSize), r);
	rectangle(*pMat, r, Scalar(0, 0, 255), 2);

	return true;
}

}
