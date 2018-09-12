#include "VEK_follow.h"

namespace kai
{

VEK_follow::VEK_follow()
{
	m_pVEK = NULL;
#ifdef USE_TENSORRT
	m_pCN = NULL;
#endif
//	m_pObs = NULL;
	m_vSteer = 0.5;

	m_distMin = 0.0;
	m_distMax = 10.0;
	m_targetX = 0.5;
	m_rTargetX = 0.05;
	m_pTarget = NULL;
	m_iTargetClass = 0;

}

VEK_follow::~VEK_follow()
{
}

bool VEK_follow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,vSteer);
	KISSm(pK,distMin);
	KISSm(pK,distMax);
	KISSm(pK,targetX);
	KISSm(pK,rTargetX);
	KISSm(pK,iTargetClass);

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("VEK_base", &iName));
	m_pVEK = (VEK_base*) (pK->parent()->getChildInstByName(iName));

//	iName = "";
//	F_INFO(pK->v("_ZEDobstacle", &iName));
//	m_pObs = (_ZEDdistance*) (pK->root()->getChildInstByName(iName));

#ifdef USE_TENSORRT
	iName = "";
	F_INFO(pK->v("_ClusterNet", &iName));
	m_pCN = (_ClusterNet*) (pK->root()->getChildInstByName(iName));

	if (!m_pCN)
	{
		LOG_E(iName << " not found");
		return false;
	}
#endif

	return true;
}

void VEK_follow::update(void)
{
	this->ActionBase::update();
	m_pTarget = NULL;

#ifdef USE_TENSORRT
	NULL_(m_pVEK);
	NULL_(m_pAM);
	NULL_(m_pCN);
//	NULL_(m_pObs);
	IF_(!isActive());

	OBJECT* pO;
	int i;
	for(i=0; i<m_pCN->size(); i++)
	{
		pO = m_pCN->at(i);
		IF_(!pO);
		IF_CONT(pO->m_iClass != m_iTargetClass);
//		pO->m_dist = m_pObs->d(&pO->m_fBBox, NULL);

		if (!m_pTarget)
		{
			m_pTarget = pO;
			continue;
		}

		if (m_pTarget->m_dist > pO->m_dist)
			m_pTarget = pO;
	}

	NULL_(m_pTarget);

	double pX = m_targetX - m_pTarget->m_fBBox.midX();
	if(abs(pX) > m_rTargetX)
	{
		double vSteer = m_vSteer * ((pX>0)?1.0:-1.0);
		m_pVEK->m_vL = vSteer;
		m_pVEK->m_vR = -vSteer;
	}

	string stateName = "VEK_FOLLOW";
	m_pAM->transit(&stateName);
#else
	LOG_I("VEK_follow requires USE_TENSORRT to be turned ON");
#endif
}

bool VEK_follow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
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
