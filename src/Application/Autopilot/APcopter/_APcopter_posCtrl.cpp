#include "_APcopter_posCtrl.h"

namespace kai
{

_APcopter_posCtrl::_APcopter_posCtrl()
{
	m_pAP = NULL;
	m_vP.init();
	m_vTargetP.init();

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pAlt = NULL;
	m_vYaw = 180.0;

	m_bSetV = true;
	m_bSetP = false;
	m_bSetON = false;
	m_bFixYaw = false;

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
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vYaw", &m_vYaw);
	pK->v("bSetV", &m_bSetV);
	pK->v("bSetP", &m_bSetP);
	pK->v("bFixYaw", &m_bFixYaw);

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

bool _APcopter_posCtrl::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _APcopter_posCtrl::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_ActionBase::update();
		updateCtrl();

		this->autoFPSto();
	}
}

int _APcopter_posCtrl::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_ActionBase::check();
}

void _APcopter_posCtrl::updateCtrl(void)
{
	IF_(check() < 0);
	if(!bActive() || !m_bSetON)
	{
		clear();
		releaseCtrl();
		return;
	}

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

	if (m_bSetV)
	{
		m_spt.vx = p;		//forward
		m_spt.vy = r;		//right
		m_spt.vz = a;		//down
		m_spt.type_mask &= 0b1111111111000111;
	}

	if (m_bSetP)
	{
		m_spt.x = p;		//forward
		m_spt.y = r;		//right
		m_spt.z = a;		//down
		m_spt.type_mask &= 0b1111111111111000;
	}

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void _APcopter_posCtrl::setPos(vFloat4& vP)
{
	m_vP = vP;
}

void _APcopter_posCtrl::setTargetPos(vFloat4& vTargetP)
{
	m_vTargetP = vTargetP;
}

void _APcopter_posCtrl::ctrlEnable(bool bON)
{
	m_bSetON = bON;
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

bool _APcopter_posCtrl::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check() < 0);

	pWin->addMsg(*this->getName());
	pWin->tabNext();

	if (!bActive() || !m_bSetON)
	{
		pWin->addMsg("Inactive");
	}
	else
	{
		pWin->addMsg(
				"vTargetP = (" + f2str(m_vTargetP.x) + ", "
						+ f2str(m_vTargetP.y) + ", " + f2str(m_vTargetP.z)
						+ ", " + f2str(m_vTargetP.w) + ")");

		pWin->addMsg(
				"vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", "
						+ f2str(m_vP.z) + ", " + f2str(m_vP.w) + ")");

		pWin->addMsg(
				"set target: V = (" + f2str(m_spt.vx) + ", " + f2str(m_spt.vy)
						+ ", " + f2str(m_spt.vz) + "), P = (" + f2str(m_spt.x)
						+ ", " + f2str(m_spt.y) + ", " + f2str(m_spt.z) + ")");
	}

	pWin->tabPrev();

//	circle(*pMat, Point(m_vMyPos.x*pMat->cols, m_vMyPos.y*pMat->rows),
//			pMat->cols*pMat->rows*0.00005,
//			Scalar(0, 255, 0), 2);
//	circle(*pMat, Point(m_vTargetPos.x*pMat->cols, m_vTargetPos.y*pMat->rows),
//			pMat->cols*pMat->rows*0.00005,
//			Scalar(0, 0, 255), 2);

	return true;
}

bool _APcopter_posCtrl::console(int& iY)
{
	IF_F(!this->_ActionBase::console(iY));
	IF_F(check() < 0);

	string msg;

	if (!bActive() || !m_bSetON)
	{
		C_MSG("Inactive");
	}

	C_MSG(
			"vTargetP = (" + f2str(m_vTargetP.x) + ", " + f2str(m_vTargetP.y)
					+ ", " + f2str(m_vTargetP.z) + ", " + f2str(m_vTargetP.w)
					+ ")");

	C_MSG(
			"vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", "
					+ f2str(m_vP.z) + ", " + f2str(m_vP.w) + ")");

	C_MSG(
			"set target: V = (" + f2str(m_spt.vx) + ", " + f2str(m_spt.vy)
					+ ", " + f2str(m_spt.vz) + "), P = (" + f2str(m_spt.x)
					+ ", " + f2str(m_spt.y) + ", " + f2str(m_spt.z) + ")");

	return true;
}

}
