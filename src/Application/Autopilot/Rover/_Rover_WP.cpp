#include "_Rover_WP.h"

namespace kai
{

_Rover_WP::_Rover_WP()
{
	m_pMavlink = NULL;
	m_pCMD = NULL;
	m_pPIDhdg = NULL;

	m_ctrl.init();
	m_pCtrl = (void*)&m_ctrl;
}

_Rover_WP::~_Rover_WP()
{
}

bool _Rover_WP::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	double d = dAngle(
						35.854685,
						139.517741,
						35.854873, 139.517581
						);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_Rover_CMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	iName = "";
	pK->v("PIDhdg", &iName);
	m_pPIDhdg = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPIDhdg, iName+": not found");

	return true;
}

int _Rover_WP::check(void)
{
	NULL__(m_pAB, -1);
	NULL__(m_pMavlink, -1);
	NULL__(m_pCMD, -1);
	NULL__(m_pPIDhdg, -1);

	return 0;
}

void _Rover_WP::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	IF_(!bActive());

	ROVER_CTRL* pCtrl = (ROVER_CTRL*)m_pAB->m_pCtrl;
	m_ctrl.m_hdg = pCtrl->m_hdg;
	m_ctrl.m_nSpeed = pCtrl->m_nTargetSpeed;

	Waypoint* pW = (Waypoint*)m_pMC->getCurrentMission();
	NULL_(pW);

	vDouble4 vP;
	vP.x = m_pMavlink->m_mavMsg.global_position_int.lat * 1e-7;
	vP.y = m_pMavlink->m_mavMsg.global_position_int.lon * 1e-7;
	vP.z = -1.0;
	vP.w = -1.0;
	pW->setPos(vP);

	MISSION_WAYPOINT* pWP = pW->getWaypoint();
	NULL_(pWP);

	m_ctrl.m_targetHdg = dAngle(
						 vP.x,
						 vP.y,
						 pWP->m_vP.x,
						 pWP->m_vP.y
						 );

	m_ctrl.m_nTargetSpeed = pWP->m_vV.x;
}

void _Rover_WP::draw(void)
{
	this->_AutopilotBase::draw();

	string msg = "mode=" + c_roverModeName[m_pCMD->m_mode]
			+ ", hdg=" + f2str(m_ctrl.m_hdg)
			+ ", targetHdg=" + f2str(m_ctrl.m_targetHdg)
			+ ", nSpeed=" + f2str(m_ctrl.m_nSpeed)
			+ ", nTargetSpeed=" + f2str(m_ctrl.m_nTargetSpeed);
	addMsg(msg);
}

}
