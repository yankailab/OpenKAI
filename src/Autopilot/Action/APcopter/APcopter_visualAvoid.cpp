#include "APcopter_visualAvoid.h"

namespace kai
{

APcopter_visualAvoid::APcopter_visualAvoid()
{
	m_pAP = NULL;
	m_pIN = NULL;
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
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));
	if (!m_pIN)
	{
		LOG_E(iName << " not found");
		return false;
	}

	Kiss** pItr = pK->getChildItr();

	OBJECT tO;
	tO.init();
	tO.m_fBBox.z = 1.0;
	tO.m_fBBox.w = 1.0;

	m_nTerrain = 0;
	while (pItr[m_nTerrain])
	{
		Kiss* pKT = pItr[m_nTerrain];
		IF_F(m_nTerrain >= N_TERRAIN);

		TERRAIN* pT = &m_pTerrain[m_nTerrain];
		m_nTerrain++;
		pT->init();

		F_ERROR_F(pKT->v("orientation", (int*)&pT->m_orientation));
		F_INFO(pKT->v("l", &tO.m_fBBox.x));
		F_INFO(pKT->v("t", &tO.m_fBBox.y));
		F_INFO(pKT->v("r", &tO.m_fBBox.z));
		F_INFO(pKT->v("b", &tO.m_fBBox.w));

		pT->m_pObj = m_pIN->add(&tO);
		NULL_F(pT->m_pObj);

		Kiss** pItrAct = pKT->getChildItr();

		pT->m_nAction = 0;
		while (pItrAct[pT->m_nAction])
		{
			Kiss* pKA = pItrAct[pT->m_nAction];
			IF_F(pT->m_nAction >= N_TERRAIN_ACTION);
			TERRAIN_ACTION* pA = &pT->m_action[pT->m_nAction];
			pT->m_nAction++;

			F_ERROR_F(pKA->v("action", (int*)&pA->m_action));
			F_ERROR_F(pKA->array("class", pA->m_pClass, N_TERRAIN_CLASS));
		}

	}

	return true;
}

void APcopter_visualAvoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pIN);
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

	int i;
	for (i = 0; i < m_nTerrain; i++)
	{
		TERRAIN* pT = &m_pTerrain[i];

//		IF_CONT(pT->);


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
