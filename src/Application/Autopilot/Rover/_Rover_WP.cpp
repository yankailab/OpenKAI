#include "_Rover_WP.h"

namespace kai
{

_Rover_WP::_Rover_WP()
{
	m_pMavlink = NULL;
	m_nSpeed = 0.2;
	m_vPos.init();
	m_dRecP = 6e-5;
	m_err = 3e-5;

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

	pK->v("nSpeed",&m_nSpeed);
	pK->v("dRecP",&m_dRecP);
	pK->v("err",&m_err);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName+": not found");

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
	IF_(pW->type() != mission_wp);

	if(bMissionChanged())
	{
		pW->m_vWP = m_vWP;
		pW->m_iWP = 0;
		pW->m_dWP = 1;
	}

	m_vPos.x = m_pMavlink->m_mavMsg.global_position_int.lat * 1e-7;
	m_vPos.y = m_pMavlink->m_mavMsg.global_position_int.lon * 1e-7;
	m_vPos.z = -1.0;
	m_vPos.w = -1.0;

	IF_(m_vPos.x < 0.0 || m_vPos.x > 90.0);	//lat
	IF_(m_vPos.y < 0.0 || m_vPos.x > 180.0);//lon

	pW->setPos(m_vPos);

	MISSION_WAYPOINT* pWP = pW->getWaypoint();
	NULL_(pWP);

	m_ctrl.m_targetHdgOffset = dAngle(
						 m_vPos.x,
						 m_vPos.y,
						 pWP->m_vP.x,
						 pWP->m_vP.y
						 ) - m_ctrl.m_hdg;

	m_ctrl.m_nTargetSpeed = m_nSpeed;//pWP->m_vV.x;
}

void _Rover_WP::updateRecord(void)
{
	IF_(check()<0);
	IF_(!bActive())

	MissionBase* pRec = (MissionBase*)m_pMC->getCurrentMission();
	NULL_(pRec);
	IF_(pRec->type() != mission_base);

	if(bMissionChanged())
		m_vWP.clear();

	vDouble4 vP;
	vP.x = m_pMavlink->m_mavMsg.global_position_int.lat * 1e-7;
	vP.y = m_pMavlink->m_mavMsg.global_position_int.lon * 1e-7;
	vP.z = -1.0;
	vP.w = -1.0;

	IF_(vP.x < 0.0 || vP.x > 90.0);	//lat
	IF_(vP.y < 0.0 || vP.x > 180.0);//lon

	MISSION_WAYPOINT wp;
	wp.init();
	wp.m_vP = vP;
	wp.m_vErr.init(m_err);

	if(m_vWP.empty())
	{
		m_vWP.push_back(wp);
		return;
	}

	MISSION_WAYPOINT* pLast = &m_vWP[m_vWP.size()-1];
	IF_(abs(pLast->m_vV.x - wp.m_vV.x) + abs(pLast->m_vV.y - wp.m_vV.y) < m_dRecP);

	m_vWP.push_back(wp);
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

	msg = "lat=" + f2str(m_vPos.x,7)
		  + ", lon=" + f2str(m_vPos.y,7)
		  + ", targetHdgOffset=" + f2str(m_ctrl.m_targetHdgOffset)
		  + ", nTargetSpeed=" + f2str(m_ctrl.m_nTargetSpeed);
	addMsg(msg);

	msg = "nRecWP=" + i2str(m_vWP.size());
	addMsg(msg);

}

}
