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

bool _Rover_WP::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _Rover_WP::check(void)
{
	NULL__(m_pAB, -1);
	NULL__(m_pAB->m_pCtrl, -1);
	NULL__(m_pMavlink, -1);
	NULL__(m_pCMD, -1);
	NULL__(m_pPIDhdg, -1);

	return 0;
}

void _Rover_WP::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateWP();

		this->autoFPSto();
	}
}

void _Rover_WP::updateWP(void)
{
	IF_(check()<0);

	ROVER_CTRL* pCtrl = (ROVER_CTRL*)m_pAB->m_pCtrl;
	if(!bActive())
	{
		m_ctrl = *pCtrl;
		return;
	}

	m_ctrl.m_hdg = pCtrl->m_hdg;
	m_ctrl.m_nSpeed = pCtrl->m_nSpeed;

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

	m_ctrl.m_targetHdgOffset = dAngle(
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

	string msg;

	if(!bActive())
	{
		msg = "Inactive";
		addMsg(msg,1);
		return;
	}

	msg = "hdg=" + f2str(m_ctrl.m_hdg)
			+ ", targetHdg=" + f2str(m_ctrl.m_targetHdgOffset)
			+ ", nSpeed=" + f2str(m_ctrl.m_nSpeed)
			+ ", nTargetSpeed=" + f2str(m_ctrl.m_nTargetSpeed);
	addMsg(msg);
}

}
