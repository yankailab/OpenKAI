#include "_APcopter_posCtrl.h"

namespace kai
{

_APcopter_posCtrl::_APcopter_posCtrl()
{
	m_pAP = NULL;
	m_bFixYaw = false;
	m_mode = pc_setP;

	m_vP.init();
	m_vTargetP.init();
	m_vSpeed.init(1.0);

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

_APcopter_posCtrl::~_APcopter_posCtrl()
{
}

bool _APcopter_posCtrl::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vTargetP",&m_vTargetP);
	pK->v("vYaw", &m_vYaw);
	pK->v("mode", (int*)&m_mode);
	pK->v("bFixYaw", &m_bFixYaw);
	pK->v("vSpeed", &m_vSpeed);

	m_vSpeed.x = abs(m_vSpeed.x);
	m_vSpeed.y = abs(m_vSpeed.y);
	m_vSpeed.z = abs(m_vSpeed.z);

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
	pK->v("_APcopter_base", &iName);
	m_pAP = (_APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int _APcopter_posCtrl::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _APcopter_posCtrl::updateCtrl(void)
{
	if (m_bFixYaw)
	{
		mavlink_param_set_t D;
		D.param_type = MAV_PARAM_TYPE_INT8;
		D.param_value = 0;
		string id = "WP_YAW_BEHAVIOR";
		strcpy(D.param_id, id.c_str());
		m_pAP->m_pMavlink->param_set(D);
	}


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

	if (m_mode == pc_setV)
	{
		m_spt.vx = p;		//forward
		m_spt.vy = r;		//right
		m_spt.vz = a;		//down
		m_spt.type_mask &= 0b1111111111000111;
	}
	else if(m_mode == pc_setP)
	{
		m_spt.x = p;		//forward
		m_spt.y = r;		//right
		m_spt.z = a;		//down
		m_spt.type_mask &= 0b1111111111111000;
	}
	else if(m_mode == pc_setVP)
	{
		m_spt.vx = (p>0.0) ? m_vSpeed.x : -m_vSpeed.x;	//forward
		m_spt.vy = (r>0.0) ? m_vSpeed.y : -m_vSpeed.y;	//right
		m_spt.vz = (a>0.0) ? m_vSpeed.z : -m_vSpeed.z;	//down
		m_spt.x = p;		//forward
		m_spt.y = r;		//right
		m_spt.z = a;		//down

		m_spt.type_mask &= 0b1111111111000000;
	}

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void _APcopter_posCtrl::clear(void)
{
	if (m_pRoll)
		m_pRoll->reset();
	if (m_pPitch)
		m_pPitch->reset();
	if (m_pAlt)
		m_pAlt->reset();
}

void _APcopter_posCtrl::releaseCtrl(void)
{
	IF_(check() < 0);

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.x = 0.0;
	m_spt.y = 0.0;
	m_spt.z = 0.0;
	m_spt.vx = 0;
	m_spt.vy = 0;
	m_spt.vz = 0;
	m_spt.yaw = (float) m_vP.w * DEG_RAD;
	m_spt.yaw_rate = (float) m_vYaw * DEG_RAD;
	m_spt.type_mask = 0b0000000111000111;
	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void _APcopter_posCtrl::draw(void)
{
	this->_AutopilotBase::draw();
	if (!bActive())
	{
		addMsg("Inactive",1);
	}
	else
	{
		addMsg(
				"vTargetP = (" + f2str(m_vTargetP.x) + ", "
						+ f2str(m_vTargetP.y) + ", " + f2str(m_vTargetP.z)
						+ ", " + f2str(m_vTargetP.w) + ")",1);

		addMsg(
				"vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", "
						+ f2str(m_vP.z) + ", " + f2str(m_vP.w) + ")",1);

		addMsg(
				"set target: V = (" + f2str(m_spt.vx) + ", " + f2str(m_spt.vy)
						+ ", " + f2str(m_spt.vz) + "), P = (" + f2str(m_spt.x)
						+ ", " + f2str(m_spt.y) + ", " + f2str(m_spt.z) + ")",1);
	}
}

}
