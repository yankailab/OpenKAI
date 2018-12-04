#include "APcopter_WP.h"

namespace kai
{

APcopter_WP::APcopter_WP()
{
	m_pAP = NULL;
	m_pPIDalt = NULL;
	m_vPos.init();
	m_vTarget.init();
}

APcopter_WP::~APcopter_WP()
{
}

bool APcopter_WP::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("PIDalt", &iName);
	m_pPIDalt = (PIDctrl*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPIDalt, iName + ": not found");

	return true;
}

int APcopter_WP::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pPIDalt,-1);

	return this->ActionBase::check();
}

void APcopter_WP::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!bActive());
	Waypoint* pWP = (Waypoint*)m_pMC->getCurrentMission();
	NULL_(pWP);

	double vZ = 0;
	if(pWP->m_dSensor >= 0)
		vZ = m_pPIDalt->update(pWP->m_dSensor, pWP->m_vWP.z);

	mavlink_set_position_target_global_int_t spt;
	spt.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;

	spt.lat_int = 1e7;
	spt.lon_int = 1e7;
	spt.alt = (float)(pWP->m_vPos.z - vZ);

	spt.vx = (float)pWP->m_speedH;		//forward
	spt.vy = (float)pWP->m_speedH;		//right
	spt.vz = (float)vZ;					//down

	spt.yaw = (float)pWP->m_hdg * DEG_RAD;
	spt.yaw_rate = (float)180.0 * DEG_RAD;
	spt.type_mask = 0b0000000111000000;

	m_pAP->m_pMavlink->setPositionTargetGlobalINT(spt);
}

bool APcopter_WP::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string* pMission = m_pMC->getCurrentMissionName();
	string msg;

	pWin->tabNext();

	if(!bActive())
	{
		msg = "Inactive";
		pWin->addMsg(&msg);
	}
	else
	{
		msg = "Waypoint";
		pWin->addMsg(&msg);
	}

	msg = "Pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
					           + f2str(m_vPos.z) + ", "
				           	   + f2str(m_vPos.w) + ")";
	pWin->addMsg(&msg);

	msg = "Target Pos = (" + f2str(m_vTarget.x) + ", "
							   + f2str(m_vTarget.y) + ", "
					           + f2str(m_vTarget.z) + ", "
				           	   + f2str(m_vTarget.w) + ")";
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_WP::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string* pState = m_pMC->getCurrentMissionName();
	string msg;

	if(!bActive())
	{
		msg = "Inactive";
	}
	else
	{
		msg = "Waypoint";
	}
	C_MSG;

	msg = "Pos = (" + f2str(m_vPos.x) + ", "
				     	 	   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ", "
							   + f2str(m_vPos.w) + ")";
	C_MSG;

	msg = "Target Pos = (" + f2str(m_vTarget.x) + ", "
							   + f2str(m_vTarget.y) + ", "
					           + f2str(m_vTarget.z) + ", "
				           	   + f2str(m_vTarget.w) + ")";
	C_MSG;

	return true;
}

}
