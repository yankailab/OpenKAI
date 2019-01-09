#include "APcopter_posCtrl.h"

namespace kai
{

APcopter_posCtrl::APcopter_posCtrl()
{
	m_pAP = NULL;
	m_vTargetPos.init();
	m_vMyPos.init();

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pAlt = NULL;
	m_vYaw = 180.0;

	m_bSetV = true;
	m_bSetP = false;
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

	pR = pK->o("roll");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		m_pRoll = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pRoll, iName + ": not found");
	}

	pR = pK->o("pitch");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		m_pPitch = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pPitch, iName + ": not found");
	}

	pR = pK->o("alt");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		m_pAlt = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pAlt, iName + ": not found");
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

	double p=0,r=0,a=0;
	if(m_pRoll)
		r = m_pRoll->update(m_vMyPos.x, m_vTargetPos.x, m_tStamp);

	if(m_pPitch)
		p = m_pPitch->update(m_vMyPos.y, m_vTargetPos.y, m_tStamp);

	if(m_pAlt)
		a = m_pAlt->update(m_vMyPos.z, m_vTargetPos.z, m_tStamp);

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.yaw_rate = (float)m_vYaw * DEG_RAD;
	m_spt.yaw = (float)m_vTargetPos.w * DEG_RAD;

	m_spt.type_mask = 0b0000000111111111;

	if(m_bSetV)
	{
		m_spt.vx = p;			//forward
		m_spt.vy = r;			//right
		m_spt.vz = a;			//down
		m_spt.type_mask &= 0b1111111111000111;
	}

	if(m_bSetP)
	{
		m_spt.x = p;			//forward
		m_spt.y = r;			//right
		m_spt.z = a;			//down
		m_spt.type_mask &= 0b1111111111111000;
	}

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void APcopter_posCtrl::setPos(vDouble4& vMyPos, vDouble4& vTargetPos)
{
	m_vMyPos = vMyPos;
	m_vTargetPos = vTargetPos;
}

void APcopter_posCtrl::clear(void)
{
	if(m_pRoll)
		m_pRoll->reset();
	if(m_pPitch)
		m_pPitch->reset();
	if(m_pAlt)
		m_pAlt->reset();
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
		pWin->addMsg("Pos = (" + f2str(m_vMyPos.x) + ", "
						 + f2str(m_vMyPos.y) + ", "
						 + f2str(m_vMyPos.z) + ", "
						 + f2str(m_vMyPos.w) + ")");

		pWin->addMsg("Target = (" + f2str(m_vTargetPos.x) + ", "
						    + f2str(m_vTargetPos.y) + ", "
						    + f2str(m_vTargetPos.z) + ", "
						    + f2str(m_vTargetPos.w) + ")");

		pWin->addMsg("set target: V = (" + f2str(m_spt.vx) + ", "
						 + f2str(m_spt.vy) + ", "
						 + f2str(m_spt.vz) + "), P = ("
						 + f2str(m_spt.x) + ", "
						 + f2str(m_spt.y) + ", "
						 + f2str(m_spt.z) + ")");
	}

	pWin->tabPrev();

	circle(*pMat, Point(m_vMyPos.x*pMat->cols, m_vMyPos.y*pMat->rows),
			pMat->cols*pMat->rows*0.00005,
			Scalar(0, 255, 0), 2);
	circle(*pMat, Point(m_vTargetPos.x*pMat->cols, m_vTargetPos.y*pMat->rows),
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
		C_MSG("Pos = (" + f2str(m_vMyPos.x) + ", "
						+ f2str(m_vMyPos.y) + ", "
						+ f2str(m_vMyPos.z) + ", "
						+ f2str(m_vMyPos.w) + ")");

		C_MSG("Target = (" + f2str(m_vTargetPos.x) + ", "
						   + f2str(m_vTargetPos.y) + ", "
						   + f2str(m_vTargetPos.z) + ", "
						   + f2str(m_vTargetPos.w) + ")");

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
