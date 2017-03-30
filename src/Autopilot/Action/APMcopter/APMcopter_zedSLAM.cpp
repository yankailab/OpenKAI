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
	int confidence = m_pZED->getMotionDelta(&mT, &mR);
	IF_(confidence < 0);

    m_mT.m_x = mT.m_z;
    m_mT.m_y = mT.m_x;
    m_mT.m_z = mT.m_y;

    m_mR.m_x = mR.m_x;
    m_mR.m_y = mR.m_z;
    m_mR.m_z = -mR.m_y;

	NULL_(m_pAPM);
	NULL_(m_pAPM->m_pMavlink);
	m_pAPM->m_pMavlink->zedVisionPositionDelta(m_dTime, &m_mR, &m_mT, confidence);

	LOG_I("forward=" << m_mT.m_x << ", right=" << m_mT.m_y << ", down=" << m_mT.m_z << "; roll=" << m_mR.m_x << ", pitch=" << m_mR.m_y << ", yaw=" << m_mR.m_z);
}

bool APMcopter_zedSLAM::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
