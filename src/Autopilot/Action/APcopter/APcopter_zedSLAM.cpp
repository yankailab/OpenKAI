#include "../APcopter/APcopter_zedSLAM.h"

namespace kai
{

APcopter_zedSLAM::APcopter_zedSLAM()
{
	m_pAPM = NULL;
	m_pZED = NULL;

	m_mT.init();
	m_mR.init();
}

APcopter_zedSLAM::~APcopter_zedSLAM()
{
}

bool APcopter_zedSLAM::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_zedSLAM::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void APcopter_zedSLAM::update(void)
{
	this->ActionBase::update();

	updateZEDtracking();
}

void APcopter_zedSLAM::updateZEDtracking(void)
{
	NULL_(m_pZED);
	if(!m_pZED->isTracking())
	{
		m_pZED->startTracking();
	}

	vDouble3 mT,mR;
	uint64_t dT;
	int confidence = m_pZED->getMotionDelta(&mT, &mR, &dT);
	IF_(confidence < 0);	//not tracking or ZED fps is too low

    m_mT.x = mT.z;
    m_mT.y = mT.x;
    m_mT.z = mT.y;

    m_mR.x = mR.x;
    m_mR.y = mR.z;
    m_mR.z = -mR.y;

	NULL_(m_pAPM);
	NULL_(m_pAPM->m_pMavlink);
	m_pAPM->m_pMavlink->visionPositionDelta(dT, &m_mR, &m_mT, confidence);
}

bool APcopter_zedSLAM::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
