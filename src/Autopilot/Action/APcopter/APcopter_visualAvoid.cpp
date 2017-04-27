#include "APcopter_visualAvoid.h"

namespace kai
{

APcopter_visualAvoid::APcopter_visualAvoid()
{
	m_pAP = NULL;
	m_pMN = NULL;
	m_nTerrain = 0;

}

APcopter_visualAvoid::~APcopter_visualAvoid()
{
}

bool APcopter_visualAvoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//create state instances
	Kiss** pItr = pK->getChildItr();

	m_nTerrain = 0;
	while (pItr[m_nTerrain])
	{
		Kiss* pKT = pItr[m_nTerrain];
		IF_F(m_nTerrain >= N_STATE);

		TERRAIN* pTerrain = &m_pTerrain[m_nTerrain];
		F_ERROR_F(pKT->v("iClass", (int*)&pTerrain->m_iClass));
		F_ERROR_F(pKT->v("action", (int*)&pTerrain->m_action));
		m_nTerrain++;
	}

	return true;
}

bool APcopter_visualAvoid::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_MatrixNet", &iName));
	m_pMN = (_MatrixNet*) (pK->root()->getChildInstByName(&iName));

	if (!m_pMN)
	{
		LOG_E(iName << " not found");
		return false;
	}

	return true;
}

void APcopter_visualAvoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pMN);
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

	int i, j;
	for (i = 0; i < m_nTerrain; i++)
	{

//		pMavlink->distanceSensor(0, //type
//			pSeg->m_orient,	//orientation
//			range.y, range.x, pDS->m_pSensor->d(&pSeg->m_roi, NULL)*100);
	}


}

bool APcopter_visualAvoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
