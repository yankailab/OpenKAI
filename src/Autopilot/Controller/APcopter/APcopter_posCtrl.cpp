#include "APcopter_posCtrl.h"

namespace kai
{

APcopter_posCtrl::APcopter_posCtrl()
{
	m_pAP = NULL;
	m_vTarget.init();
	m_vPos.init();

	m_vYaw = 180;
	m_bSetV = true;
	m_bSetP = false;

	for(int i=0;i<N_CTRL;i++)
	{
		m_ctrl[i].init();
	}
}

APcopter_posCtrl::~APcopter_posCtrl()
{
}

bool APcopter_posCtrl::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,vYaw);
	KISSm(pK,bSetV);
	KISSm(pK,bSetP);

	Kiss* pR;
	string iName;
	AP_POS_CTRL_TARGET* pRC = NULL;

	pR = pK->o("roll");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[C_ROLL];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	pR = pK->o("pitch");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[C_PITCH];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	pR = pK->o("yaw");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[C_YAW];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	pR = pK->o("alt");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_ctrl[C_ALT];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
	}

	pR = pK->o("target");
	if(!pR->empty())
	{
		pR->v("x", &m_vTarget.x);
		pR->v("y", &m_vTarget.y);
		pR->v("z", &m_vTarget.z);	//distance
		pR->v("yaw", &m_vTarget.w);	//heading
	}

	//link
	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int APcopter_posCtrl::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_posCtrl::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		releaseCtrl();
		clear();
		return;
	}

	m_ctrl[C_PITCH].update(m_vPos.y, m_vTarget.y);
	m_ctrl[C_ROLL].update(m_vPos.x, m_vTarget.x);
	m_ctrl[C_ALT].update(m_vPos.z, m_vTarget.z);
	m_ctrl[C_YAW].update(m_vPos.w, m_vTarget.w);

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.yaw_rate = (float)m_vYaw * DEG_RAD;
	m_spt.yaw = m_pAP->m_pMavlink->m_msg.attitude.yaw;
	if(m_vTarget.w >= 0)
		m_spt.yaw = (float)m_vTarget.w * DEG_RAD;

	m_spt.type_mask = 0b0000000111111111;

	if(m_bSetV)
	{
		m_spt.vx = m_ctrl[C_PITCH].m_v;			//forward
		m_spt.vy = m_ctrl[C_ROLL].m_v;			//right
		m_spt.vz = m_ctrl[C_ALT].m_v;			//down
		m_spt.type_mask &= 0b1111111111000111;	//velocity
	}

	if(m_bSetP)
	{
		m_spt.x = m_ctrl[C_PITCH].m_v;			//forward
		m_spt.y = m_ctrl[C_ROLL].m_v;			//right
		m_spt.z = m_ctrl[C_ALT].m_v;			//down
		m_spt.type_mask &= 0b1111111111111000;	//velocity
	}

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void APcopter_posCtrl::setTargetPos(vDouble4& vP)
{
	m_vTarget = vP;
}

void APcopter_posCtrl::setPos(vDouble4& vP)
{
	m_vPos = vP;
}

void APcopter_posCtrl::clear(void)
{
	m_ctrl[C_PITCH].clear();
	m_ctrl[C_ROLL].clear();
	m_ctrl[C_ALT].clear();
	m_ctrl[C_YAW].clear();
}

void APcopter_posCtrl::releaseCtrl(void)
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
	m_spt.yaw_rate = (float)m_vYaw * DEG_RAD;
	m_spt.type_mask = 0b0000000111000000;
	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

bool APcopter_posCtrl::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	pWin->addMsg(*this->getName());
	pWin->tabNext();

	if(!bActive())
	{
		pWin->addMsg("Inactive");
	}
	else
	{
		pWin->addMsg("Pos = (" + f2str(m_vPos.x) + ", "
						 + f2str(m_vPos.y) + ", "
						 + f2str(m_vPos.z) + ", "
						 + f2str(m_vPos.w) + ")");

		pWin->addMsg("Target = (" + f2str(m_vTarget.x) + ", "
						    + f2str(m_vTarget.y) + ", "
						    + f2str(m_vTarget.z) + ", "
						    + f2str(m_vTarget.w) + ")");

		pWin->addMsg("set target: V = (" + f2str(m_spt.vx) + ", "
						 + f2str(m_spt.vy) + ", "
						 + f2str(m_spt.vz) + "), P = ("
						 + f2str(m_spt.x) + ", "
						 + f2str(m_spt.y) + ", "
						 + f2str(m_spt.z) + ")");
	}

	pWin->tabPrev();

	circle(*pMat, Point(m_vPos.x*pMat->cols, m_vPos.y*pMat->rows),
			pMat->cols*pMat->rows*0.00005,
			Scalar(0, 255, 0), 2);
	circle(*pMat, Point(m_vTarget.x*pMat->cols, m_vTarget.y*pMat->rows),
			pMat->cols*pMat->rows*0.00005,
			Scalar(0, 0, 255), 2);

	return true;
}

bool APcopter_posCtrl::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(!bActive())
	{
		C_MSG("Inactive");
	}
	else
	{
		C_MSG("Pos = (" + f2str(m_vPos.x) + ", "
						+ f2str(m_vPos.y) + ", "
						+ f2str(m_vPos.z) + ", "
						+ f2str(m_vPos.w) + ")");

		C_MSG("Target = (" + f2str(m_vTarget.x) + ", "
						   + f2str(m_vTarget.y) + ", "
						   + f2str(m_vTarget.z) + ", "
						   + f2str(m_vTarget.w) + ")");

		C_MSG("set target: V = (" + f2str(m_spt.vx) + ", "
						 + f2str(m_spt.vy) + ", "
						 + f2str(m_spt.vz) + "), P = ("
						 + f2str(m_spt.x) + ", "
						 + f2str(m_spt.y) + ", "
						 + f2str(m_spt.z) + ")");
	}

	return true;
}

}
