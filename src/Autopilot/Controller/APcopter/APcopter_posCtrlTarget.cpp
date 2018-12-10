#include "APcopter_posCtrlTarget.h"

namespace kai
{

APcopter_posCtrlTarget::APcopter_posCtrlTarget()
{
	m_yawRate = 180;
	m_bSetV = true;

	for(int i=0;i<N_CTRL;i++)
	{
		m_ctrl[i].init();
	}
}

APcopter_posCtrlTarget::~APcopter_posCtrlTarget()
{
}

bool APcopter_posCtrlTarget::init(void* pKiss)
{
	IF_F(!this->APcopter_posCtrlBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,yawRate);
	KISSm(pK,bSetV);

	Kiss* pR;
	string iName;
	AP_POS_CTRL_TARGET* pRC = NULL;

	pR = pK->o("roll");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[CTRL_ROLL];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	pR = pK->o("pitch");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[CTRL_PITCH];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	pR = pK->o("yaw");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[CTRL_YAW];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	pR = pK->o("alt");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[CTRL_ALT];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	return true;
}

int APcopter_posCtrlTarget::check(void)
{
	return this->APcopter_posCtrlBase::check();
}

void APcopter_posCtrlTarget::update(void)
{
	this->APcopter_posCtrlBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		releaseCtrl();
		clear();
		return;
	}

	m_ctrl[CTRL_PITCH].update(m_vPos.y, m_vTarget.y);
	m_ctrl[CTRL_ROLL].update(m_vPos.x, m_vTarget.x);
	m_ctrl[CTRL_ALT].update(m_vPos.z, m_vTarget.z);
	m_ctrl[CTRL_YAW].update(m_vPos.w, m_vTarget.w);

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.yaw_rate = (float)m_yawRate * DEG_RAD;
	m_spt.yaw = m_pAP->m_pMavlink->m_msg.attitude.yaw;
	if(m_vTarget.w >= 0)
		m_spt.yaw = (float)m_vTarget.w * DEG_RAD;


	if(m_bSetV)
	{
		m_spt.x = 0.0;
		m_spt.y = 0.0;
		m_spt.z = 0.0;
		m_spt.vx = m_ctrl[CTRL_PITCH].m_v;		//forward
		m_spt.vy = m_ctrl[CTRL_ROLL].m_v;		//right
		m_spt.vz = m_ctrl[CTRL_ALT].m_v;		//down
		m_spt.type_mask = 0b0000000111000111;	//velocity
	}
	else
	{
		m_spt.x = m_ctrl[CTRL_PITCH].m_v;		//forward
		m_spt.y = m_ctrl[CTRL_ROLL].m_v;		//right
		m_spt.z = m_ctrl[CTRL_ALT].m_v;			//down
		m_spt.vx = 0.0;
		m_spt.vy = 0.0;
		m_spt.vz = 0.0;
		m_spt.type_mask = 0b0000000111111000;	//position
	}

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void APcopter_posCtrlTarget::setTargetPos(vDouble4& vT)
{
	m_vTarget = vT;
}

void APcopter_posCtrlTarget::setPos(vDouble4& vP)
{
	m_vPos = vP;
}

void APcopter_posCtrlTarget::clear(void)
{
	m_ctrl[CTRL_PITCH].clear();
	m_ctrl[CTRL_ROLL].clear();
	m_ctrl[CTRL_ALT].clear();
	m_ctrl[CTRL_YAW].clear();
}

void APcopter_posCtrlTarget::releaseCtrl(void)
{
	IF_(check()<0);

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.x = 0.0;
	m_spt.y = 0.0;
	m_spt.z = 0.0;
	m_spt.vx = 0;
	m_spt.vy = 0;
	m_spt.vz = 0;
	m_spt.yaw = m_pAP->m_pMavlink->m_msg.attitude.yaw;
	m_spt.yaw_rate = (float)m_yawRate * DEG_RAD;
	m_spt.type_mask = 0b0000000111000000;
	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

bool APcopter_posCtrlTarget::draw(void)
{
	IF_F(!this->APcopter_posCtrlBase::draw());
	IF_F(check()<0);
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	IF_T(!bActive());

	pWin->tabNext();

	pWin->addMsg("set target = (" + f2str(m_spt.vx) + ", "
					 + f2str(m_spt.vy) + ", "
					 + f2str(m_spt.vz) + "), P = ("
					 + f2str(m_spt.x) + ", "
					 + f2str(m_spt.y) + ", "
					 + f2str(m_spt.z) + ")");

	pWin->tabPrev();

	return true;
}

bool APcopter_posCtrlTarget::console(int& iY)
{
	IF_F(!this->APcopter_posCtrlBase::console(iY));
	IF_F(check()<0);
	IF_T(!bActive());
	string msg;

	C_MSG("set target = (" + f2str(m_spt.vx) + ", "
					 + f2str(m_spt.vy) + ", "
					 + f2str(m_spt.vz) + "), P = ("
					 + f2str(m_spt.x) + ", "
					 + f2str(m_spt.y) + ", "
					 + f2str(m_spt.z) + ")");

	return true;
}

}
