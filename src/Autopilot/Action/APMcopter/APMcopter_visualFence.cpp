#include "APMcopter_visualFence.h"

namespace kai
{

APMcopter_visualFence::APMcopter_visualFence()
{
	m_pAPM = NULL;
	m_pIN = NULL;
	m_pTerrain = NULL;

	m_terrainBox.init();

}

APMcopter_visualFence::~APMcopter_visualFence()
{
}

bool APMcopter_visualFence::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	Kiss* pT;
	pT = pK->o("terrainBox");
	IF_F(pT->empty());
	F_INFO(pT->v("left", &m_terrainBox.x));
	F_INFO(pT->v("top", &m_terrainBox.y));
	F_INFO(pT->v("right", &m_terrainBox.z));
	F_INFO(pT->v("bottom", &m_terrainBox.w));

	return true;
}

bool APMcopter_visualFence::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	if (!m_pIN)
	{
		LOG_E("_ImageNet not found");
		return false;
	}

	OBJECT oBox;
	oBox.init();
	oBox.m_name = "";
	oBox.m_fBBox = m_terrainBox;
	m_pTerrain = m_pIN->add(&oBox);
	NULL_F(m_pTerrain);

	return true;
}

void APMcopter_visualFence::update(void)
{
	this->ActionBase::update();

	NULL_(m_pIN);


	NULL_(m_pAPM);
	NULL_(m_pAPM->m_pMavlink);
//	m_pAPM->m_pMavlink->zedVisionPositionDelta(m_dTime);
}

bool APMcopter_visualFence::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
