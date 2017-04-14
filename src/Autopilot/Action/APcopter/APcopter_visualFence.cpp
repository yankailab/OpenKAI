#include "../APcopter/APcopter_visualFence.h"

namespace kai
{

APcopter_visualFence::APcopter_visualFence()
{
	m_pAPM = NULL;
	m_pMN = NULL;
	m_nTerrain = 0;

}

APcopter_visualFence::~APcopter_visualFence()
{
}

bool APcopter_visualFence::init(void* pKiss)
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

bool APcopter_visualFence::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

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

void APcopter_visualFence::update(void)
{
	this->ActionBase::update();

	NULL_(m_pMN);


	NULL_(m_pAPM);
	NULL_(m_pAPM->m_pMavlink);
//	m_pAPM->m_pMavlink->zedVisionPositionDelta(m_dTime);
}

bool APcopter_visualFence::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
