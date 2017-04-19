#include "../APcopter/APcopter_zedSLAM.h"

namespace kai
{

APcopter_zedSLAM::APcopter_zedSLAM()
{
	m_pAP = NULL;
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
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

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
	IF_(!m_pZED->isOpened());

	vDouble3 mT,mR;
	uint64_t dT;
	int confidence = m_pZED->getMotionDelta(&mT, &mR, &dT);
	IF_(confidence < 0);	//not tracking or ZED fps is too low
	IF_(dT == 0);
	IF_(dT > USEC_1SEC);

    m_mT.x = mT.z;	//forward
    m_mT.y = mT.x;	//right
    m_mT.z = mT.y;	//down

    m_mR.x = -mR.x;  //roll
    m_mR.y = -mR.z;  //pitch
    m_mR.z = -mR.y;  //yaw

	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	m_pAP->m_pMavlink->visionPositionDelta(m_dTime, &m_mR, &m_mT, confidence);

//    static double tx=0,ty=0,tz=0;
//    static double rx=0,ry=0,rz=0;
//    tx += m_mT.x;
//    ty += m_mT.y;
//    tz += m_mT.z;
//    rx += m_mR.x;
//    ry += m_mR.y;
//    rz += m_mR.z;
//    printf("zedSLAM:  f=%.5lf\t r=%.5lf\t d=%.5lf\t | r=%.5lf\t p=%.5lf\t y=%.5lf\n",tx,ty,tz,rx,ry,rz);

}

bool APcopter_zedSLAM::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
