#include "APcopter_WP.h"

namespace kai
{

APcopter_WP::APcopter_WP()
{
	m_pAP = NULL;
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

	return true;
}

int APcopter_WP::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_WP::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!isActive());
	Waypoint* pWP = (Waypoint*)m_pMC->getCurrentMission();
	NULL_(pWP);

	vDouble3 vErr = pWP->m_vErr;
	double dZ = 0.0;
	if(vErr.z > 0)
		dZ = pWP->m_speedV;
	else if(vErr.z < 0)
		dZ = -pWP->m_speedV;

	mavlink_set_position_target_local_ned_t spt;
	spt.coordinate_frame = MAV_FRAME_LOCAL_OFFSET_NED;//MAV_FRAME_BODY_OFFSET_NED;
	spt.x = 0.0;
	spt.y = 0.0;
	spt.z = 0.0;
	spt.vx = 0;		//forward
	spt.vy = 0;		//right
	spt.vz = (float)dZ;	//down
	spt.yaw = (float)pWP->m_hdg * DEG_RAD;
	spt.yaw_rate = (float)180.0 * DEG_RAD;
	spt.type_mask = 0b0000000111000111;	//velocity

	m_pAP->m_pMavlink->setPositionTargetLocalNED(spt);
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

	if(!isActive())
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

bool APcopter_WP::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string* pState = m_pMC->getCurrentMissionName();
	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else
	{
		msg = "Waypoint";
	}
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Pos = (" + f2str(m_vPos.x) + ", "
				     	 	   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ", "
							   + f2str(m_vPos.w) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Target Pos = (" + f2str(m_vTarget.x) + ", "
							   + f2str(m_vTarget.y) + ", "
					           + f2str(m_vTarget.z) + ", "
				           	   + f2str(m_vTarget.w) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
