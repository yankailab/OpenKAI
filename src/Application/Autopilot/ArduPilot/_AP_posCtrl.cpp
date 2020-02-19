#include "../ArduPilot/_AP_posCtrl.h"

namespace kai
{

_AP_posCtrl::_AP_posCtrl()
{
	m_pAP = NULL;

	m_vP.init();
	m_vTargetP.init();
	m_vTargetP.x = 0.5;
	m_vTargetP.y = 0.5;

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pAlt = NULL;
	m_vYaw = 90.0;

	m_sptLocal.vx = 0.0;
	m_sptLocal.vy = 0.0;
	m_sptLocal.vz = 0.0;
	m_sptLocal.x = 0.0;
	m_sptLocal.y = 0.0;
	m_sptLocal.z = 0.0;

	m_sptGlobal.vx = 0.0;
	m_sptGlobal.vy = 0.0;
	m_sptGlobal.vz = 0.0;
	m_sptGlobal.lat_int = 0.0;
	m_sptGlobal.lon_int = 0.0;
	m_sptGlobal.alt = 0.0;

}

_AP_posCtrl::~_AP_posCtrl()
{
}

bool _AP_posCtrl::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vTargetP",&m_vTargetP);
	pK->v("vTargetGlobal",&m_vTargetGlobal);
	pK->v("vYaw", &m_vYaw);

	string iName;

	iName = "";
	pK->v("PIDroll", &iName);
	m_pRoll = (PIDctrl*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("PIDpitch", &iName);
	m_pPitch = (PIDctrl*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("PIDalt", &iName);
	m_pAlt = (PIDctrl*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int _AP_posCtrl::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _AP_posCtrl::setPosLocal(void)
{
	float p = 0, r = 0, a = 0;
	if (m_pRoll)
		r = m_pRoll->update(m_vP.x, m_vTargetP.x, m_tStamp);

	if (m_pPitch)
		p = m_pPitch->update(m_vP.y, m_vTargetP.y, m_tStamp);

	if (m_pAlt)
		a = m_pAlt->update(m_vP.z, m_vTargetP.z, m_tStamp);
	else
		a = m_vP.z;


	m_sptLocal.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_sptLocal.vx = p;		//forward
	m_sptLocal.vy = r;		//right
	m_sptLocal.vz = a;		//down
	m_sptLocal.yaw = (float) m_vP.w * DEG2RAD;
	m_sptLocal.yaw_rate = (float) m_vYaw * DEG2RAD;
	m_sptLocal.type_mask  = 0b0000110111000111;	//set velocity

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_sptLocal);
}

void _AP_posCtrl::setPosGlobal(void)
{
	m_sptGlobal.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
	m_sptGlobal.lat_int = m_vTargetGlobal.x * 1e7;
	m_sptGlobal.lon_int = m_vTargetGlobal.y * 1e7;
	m_sptGlobal.alt = m_vTargetGlobal.z;
	m_sptGlobal.vx = 0.0;
	m_sptGlobal.vy = 0.0;
	m_sptGlobal.vz = 0.0;
	m_sptGlobal.yaw = (float) m_vTargetGlobal.w * DEG2RAD;
	m_sptGlobal.yaw_rate = m_vYaw * DEG2RAD;
	m_sptGlobal.type_mask = 0b0000110111111000; //set position

	m_pAP->m_pMavlink->setPositionTargetGlobalINT(m_sptGlobal);
}

void _AP_posCtrl::clear(void)
{
	if (m_pRoll)
		m_pRoll->reset();
	if (m_pPitch)
		m_pPitch->reset();
	if (m_pAlt)
		m_pAlt->reset();
}

void _AP_posCtrl::releaseCtrl(void)
{
	IF_(check() < 0);

	m_sptLocal.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_sptLocal.x = 0.0;
	m_sptLocal.y = 0.0;
	m_sptLocal.z = 0.0;
	m_sptLocal.vx = 0;
	m_sptLocal.vy = 0;
	m_sptLocal.vz = 0;
	m_sptLocal.yaw = (float) m_vTargetP.w * DEG2RAD;
	m_sptLocal.yaw_rate = 0.0;
	m_sptLocal.type_mask = 0b0000110111000111;
	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_sptLocal);


//	m_sptGlobal.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
//	m_sptGlobal.lat_int = 0.0;
//	m_sptGlobal.lon_int = 0.0;
//	m_sptGlobal.alt = 0.0;
//	m_sptGlobal.vx = 0.0;
//	m_sptGlobal.vy = 0.0;
//	m_sptGlobal.vz = 0.0;
//	m_sptGlobal.yaw = (float) m_vTargetP.w * DEG_RAD;
//	m_sptGlobal.yaw_rate = 0.0;
//	m_sptGlobal.type_mask = 0b0000110111000111;
//	m_pAP->m_pMavlink->setPositionTargetGlobalINT(m_sptGlobal);
}

void _AP_posCtrl::draw(void)
{
	this->_AutopilotBase::draw();
	if (!bActive())
		addMsg("Inactive",1);


	addMsg( "Local NED:");
	addMsg(	"vTargetP = (" + f2str(m_vTargetP.x) + ", "
					+ f2str(m_vTargetP.y) + ", " + f2str(m_vTargetP.z)
					+ ", " + f2str(m_vTargetP.w) + ")",1);
	addMsg(	"vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", "
					+ f2str(m_vP.z) + ", " + f2str(m_vP.w) + ")",1);
	addMsg(	"setV = (" + f2str(m_sptLocal.vx,7) + ", " + f2str(m_sptLocal.vy,7)
					+ ", " + f2str(m_sptLocal.vz,7) + ")",1);
	addMsg(	"setP = (" + f2str(m_sptLocal.x,7) + ", " + f2str(m_sptLocal.y,7)
					+ ", " + f2str(m_sptLocal.z,7) + ")",1);
	addMsg(	"yawRate=" + f2str(m_sptLocal.yaw_rate) + ", yaw=" + f2str(m_sptLocal.yaw),1);

	addMsg( "Global INT:");
	addMsg(	"vTargetGlobal = (" + f2str(m_vTargetGlobal.x,7) + ", "
					+ f2str(m_vTargetGlobal.y,7) + ", " + f2str(m_vTargetGlobal.z,7)
					+ ", " + f2str(m_vTargetGlobal.w) + ")",1);

}

}
