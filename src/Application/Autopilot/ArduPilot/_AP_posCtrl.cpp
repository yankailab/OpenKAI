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
	m_vYaw = 180.0;

	m_spt.vx = 0.0;
	m_spt.vy = 0.0;
	m_spt.vz = 0.0;
	m_spt.x = 0.0;
	m_spt.y = 0.0;
	m_spt.z = 0.0;

}

_AP_posCtrl::~_AP_posCtrl()
{
}

bool _AP_posCtrl::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vTargetP",&m_vTargetP);
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
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int _AP_posCtrl::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _AP_posCtrl::updateCtrl(void)
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


	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.yaw_rate = (float) m_vYaw * DEG_RAD;
	m_spt.yaw = (float) m_vP.w * DEG_RAD;
	m_spt.type_mask = 0b0000000111111111;

	m_spt.vx = p;		//forward
	m_spt.vy = r;		//right
	m_spt.vz = a;		//down
	m_spt.type_mask &= 0b1111111111000111;

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
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

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.x = 0.0;
	m_spt.y = 0.0;
	m_spt.z = 0.0;
	m_spt.vx = 0;
	m_spt.vy = 0;
	m_spt.vz = 0;
	m_spt.yaw = (float) m_vTargetP.w * DEG_RAD;
	m_spt.yaw_rate = 0.0;
	m_spt.type_mask = 0b0000000111000111;
	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void _AP_posCtrl::draw(void)
{
	this->_AutopilotBase::draw();
	if (!bActive())
		addMsg("Inactive",1);


	addMsg(	"vTargetP = (" + f2str(m_vTargetP.x) + ", "
					+ f2str(m_vTargetP.y) + ", " + f2str(m_vTargetP.z)
					+ ", " + f2str(m_vTargetP.w) + ")",1);

	addMsg(	"vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", "
					+ f2str(m_vP.z) + ", " + f2str(m_vP.w) + ")",1);

	addMsg(	"setV = (" + f2str(m_spt.vx,7) + ", " + f2str(m_spt.vy,7)
					+ ", " + f2str(m_spt.vz,7) + ")",1);

	addMsg(	"setP = (" + f2str(m_spt.x,7) + ", " + f2str(m_spt.y,7)
					+ ", " + f2str(m_spt.z,7) + ")",1);

	addMsg(	"yawRate=" + f2str(m_spt.yaw_rate) + ", yaw=" + f2str(m_spt.yaw),1);

}

}
