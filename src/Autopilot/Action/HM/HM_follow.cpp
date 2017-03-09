#include "HM_follow.h"

namespace kai
{

HM_follow::HM_follow()
{
	m_pHM = NULL;
	m_pAI = NULL;
	m_pObs = NULL;

	m_distMin = 0.0;
	m_distMax = 10.0;
	m_rpmSteer = 0.0;
	m_rpmT = 0.0;
	m_targetX = 0.5;
	m_objLifetime = 100000;
	m_pTarget = NULL;
	m_iTargetClass = 0;
	m_targetName = "";
}

HM_follow::~HM_follow()
{
}

bool HM_follow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("distMin", &m_distMin));
	F_INFO(pK->v("distMax", &m_distMax));
	F_INFO(pK->v("rpmT", &m_rpmT));
	F_INFO(pK->v("rpmSteer", &m_rpmSteer));
	F_INFO(pK->v("targetX", &m_targetX));
	F_INFO(pK->v("iTargetClass", &m_iTargetClass));
	F_INFO(pK->v("targetName", &m_targetName));
	F_INFO(pK->v("objLifetime", &m_objLifetime));

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
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_AIbase", &iName));
	m_pAI = (_AIbase*) (pK->root()->getChildInstByName(&iName));

	if (!m_pAI)
	{
		LOG_E("_DetectNet not found");
		return true;
	}

	return true;
}

void HM_follow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pAI);
	NULL_(m_pObs);
	IF_(!isActive());

	m_pTarget = NULL;

	int i;
	for (i = 0; i < m_pAI->size(); i++)
	{
		OBJECT* pObj = m_pAI->get(i, 0);
		IF_CONT(!pObj);
		if(m_targetName=="")
		{
			IF_CONT(pObj->m_iClass != m_iTargetClass);
		}
		else
		{
			IF_CONT(pObj->m_name != m_targetName);
		}

		pObj->m_dist = m_pObs->dist(&pObj->m_fBBox, NULL);

		if (!m_pTarget)
		{
			m_pTarget = pObj;
			continue;
		}

		if (m_pTarget->m_dist > pObj->m_dist)
		{
			m_pTarget = pObj;
			continue;
		}
	}

	NULL_(m_pTarget);
	IF_(m_pTarget->m_dist > m_distMax);

	m_pHM->m_bSpeaker = true;

	double pX = m_targetX - m_pTarget->m_fBBox.midX();
	int rpmSteer = m_rpmSteer * pX;

	m_pHM->m_rpmL = -rpmSteer;
	m_pHM->m_rpmR = rpmSteer;

	IF_(m_pTarget->m_dist < m_distMin);

	m_pHM->m_rpmL += m_rpmT;
	m_pHM->m_rpmR += m_rpmT;
}

void HM_follow::active(bool bActive)
{

}

bool HM_follow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
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
	Rect r;
	vInt42rect(&m_pTarget->m_bbox, &r);
	rectangle(*pMat, r, Scalar(0, 0, 255), 2);

	return true;
}

}
