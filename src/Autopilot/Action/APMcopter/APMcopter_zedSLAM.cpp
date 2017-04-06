#include "APMcopter_zedSLAM.h"

namespace kai
{

APMcopter_zedSLAM::APMcopter_zedSLAM()
{
	m_pAPM = NULL;
	m_pZED = NULL;

	m_mT.init();
	m_mR.init();
}

APMcopter_zedSLAM::~APMcopter_zedSLAM()
{
}

bool APMcopter_zedSLAM::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APMcopter_zedSLAM::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void APMcopter_zedSLAM::update(void)
{
	this->ActionBase::update();

	updateZEDtracking();
}

void APMcopter_zedSLAM::updateZEDtracking(void)
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
	m_pAPM->m_pMavlink->zedVisionPositionDelta(dT, &m_mR, &m_mT, confidence);

	LOG_I("dT=" << dT << ", forward=" << m_mT.x << ", right=" << m_mT.y << ", down=" << m_mT.z << "; roll=" << m_mR.x << ", pitch=" << m_mR.y << ", yaw=" << m_mR.z);
}

bool APMcopter_zedSLAM::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
