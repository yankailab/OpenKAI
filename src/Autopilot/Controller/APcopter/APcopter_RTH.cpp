#include "APcopter_RTH.h"

namespace kai
{

APcopter_RTH::APcopter_RTH()
{
	m_pAP = NULL;
	m_pDS = NULL;
	m_kZsensor = 1.0;
}

APcopter_RTH::~APcopter_RTH()
{
}

bool APcopter_RTH::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,kZsensor);

	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_DistSensorBase", &iName);
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));

	return true;
}

int APcopter_RTH::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_RTH::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!bActive());
	RTH* pRTH = (RTH*)m_pMC->getCurrentMission();
	NULL_(pRTH);

	vDouble3 p;

	IF_(!m_pAP->getHomePos(&p));
	pRTH->setHome(p);

	p = m_pAP->getPos();
	pRTH->setPos(p);

	vDouble3 pHome = pRTH->getHome();
	double alt = p.z;
	if(m_pDS)
	{
		double dS = m_pDS->dAvr();
		if(dS > 0)
			alt += (pHome.z - dS) * m_kZsensor;
	}
	else
	{
		alt = pHome.z;
	}

	mavlink_set_position_target_global_int_t spt;
	spt.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;

	//target position
	spt.lat_int = pHome.x*1e7;
	spt.lon_int = pHome.y*1e7;
	spt.alt = (float)alt;

	//velocity, ignored at the moment
	spt.vx = 0.0;//(float)pWP->m_speedH;		//forward
	spt.vy = 0.0;//(float)pWP->m_speedH;		//right
	spt.vz = 0.0;//(float)vZ;					//down

	//heading
	spt.yaw_rate = (float)180.0 * DEG_RAD;
	spt.yaw = m_pAP->m_pMavlink->m_msg.attitude.yaw;
	double hdg = pRTH->getHdg();
	if(hdg >= 0)
		spt.yaw = (float)hdg * DEG_RAD;

	spt.type_mask = 0b0000000111111000;
	m_pAP->m_pMavlink->setPositionTargetGlobalINT(spt);
}

bool APcopter_RTH::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	pWin->tabNext();

	if(!bActive())
		pWin->addMsg("Inactive");
	else
		pWin->addMsg("RTH");

	pWin->tabPrev();

	return true;
}

bool APcopter_RTH::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(!bActive())
	{
		C_MSG("Inactive");
	}
	else
	{
		C_MSG("RTH");
	}

	return true;
}

}
