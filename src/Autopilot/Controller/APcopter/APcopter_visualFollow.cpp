#include "../../Controller/APcopter/APcopter_visualFollow.h"

namespace kai
{

APcopter_visualFollow::APcopter_visualFollow()
{
	m_pAP = NULL;

}

APcopter_visualFollow::~APcopter_visualFollow()
{
}

bool APcopter_visualFollow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//create state instances
	Kiss** pItr = pK->getChildItr();

//	m_nTerrain = 0;
//	while (pItr[m_nTerrain])
//	{
//		Kiss* pKT = pItr[m_nTerrain];
//		IF_F(m_nTerrain >= N_STATE);
//
//		TERRAIN* pTerrain = &m_pTerrain[m_nTerrain];
//		F_ERROR_F(pKT->v("iClass", (int*)&pTerrain->m_iClass));
//		F_ERROR_F(pKT->v("action", (int*)&pTerrain->m_action));
//		m_nTerrain++;
//	}

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	return true;
}

void APcopter_visualFollow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
//	m_pAPM->m_pMavlink->zedVisionPositionDelta(m_dTime);
}

bool APcopter_visualFollow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

}
