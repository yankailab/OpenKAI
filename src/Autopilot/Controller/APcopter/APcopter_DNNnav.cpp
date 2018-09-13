#include "../../Controller/APcopter/APcopter_DNNnav.h"

namespace kai
{

APcopter_DNNnav::APcopter_DNNnav()
{
#ifdef USE_TENSORRT
	m_pIN = NULL;
#endif
	m_pAP = NULL;
	m_nTerrain = 0;

}

APcopter_DNNnav::~APcopter_DNNnav()
{
}

bool APcopter_DNNnav::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));

#ifdef USE_TENSORRT
	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInst(iName));
	if (!m_pIN)
	{
		LOG_E(iName << " not found");
		return false;
	}
#endif

	return true;
}

void APcopter_DNNnav::update(void)
{
	this->ActionBase::update();

#ifdef USE_TENSORRT
	NULL_(m_pIN);
#endif
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

}

bool APcopter_DNNnav::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

}
