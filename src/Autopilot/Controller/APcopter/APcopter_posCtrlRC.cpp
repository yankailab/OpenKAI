#include "APcopter_posCtrlRC.h"

namespace kai
{

APcopter_posCtrlRC::APcopter_posCtrlRC()
{
	for(int i=0;i<N_CTRL;i++)
	{
		m_rc[i].init();
	}

	resetRC();
}

APcopter_posCtrlRC::~APcopter_posCtrlRC()
{
}

bool APcopter_posCtrlRC::init(void* pKiss)
{
	IF_F(!this->APcopter_posCtrlBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss* pR;
	string iName;
	AP_POS_CTRL_RC* pRC = NULL;

	pR = pK->o("roll");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_rc[CTRL_ROLL];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
		pR->v("pwmL", &pRC->m_pwmL);
		pR->v("pwmM", &pRC->m_pwmM);
		pR->v("pwmH", &pRC->m_pwmH);
		pR->v("iChan", &pRC->m_iChan);
		pRC->setRCChannel(&m_rcO);
	}

	pR = pK->o("pitch");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_rc[CTRL_PITCH];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
		pR->v("pwmL", &pRC->m_pwmL);
		pR->v("pwmM", &pRC->m_pwmM);
		pR->v("pwmH", &pRC->m_pwmH);
		pR->v("iChan", &pRC->m_iChan);
		pRC->setRCChannel(&m_rcO);
	}

	pR = pK->o("yaw");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_rc[CTRL_YAW];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
		pR->v("pwmL", &pRC->m_pwmL);
		pR->v("pwmM", &pRC->m_pwmM);
		pR->v("pwmH", &pRC->m_pwmH);
		pR->v("iChan", &pRC->m_iChan);
		pRC->setRCChannel(&m_rcO);
	}

	pR = pK->o("alt");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_rc[CTRL_ALT];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
		pR->v("pwmL", &pRC->m_pwmL);
		pR->v("pwmM", &pRC->m_pwmM);
		pR->v("pwmH", &pRC->m_pwmH);
		pR->v("iChan", &pRC->m_iChan);
		pRC->setRCChannel(&m_rcO);
	}

	return true;
}

int APcopter_posCtrlRC::check(void)
{
	return this->APcopter_posCtrlBase::check();
}

void APcopter_posCtrlRC::update(void)
{
	this->APcopter_posCtrlBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		releaseRC();
		clear();
		return;
	}

	resetRC();

	m_rc[CTRL_PITCH].update(m_vPos.y, m_vTarget.y);
	m_rc[CTRL_ROLL].update(m_vPos.x, m_vTarget.x);
	m_rc[CTRL_ALT].update(m_vPos.z, m_vTarget.z);
	m_rc[CTRL_YAW].update(m_vPos.w, m_vTarget.w);

	m_pAP->m_pMavlink->rcChannelsOverride(m_rcO);
}

void APcopter_posCtrlRC::setTargetPos(vDouble4& vT)
{
	m_vTarget = vT;
}

void APcopter_posCtrlRC::setPos(vDouble4& vP)
{
	m_vPos = vP;
}

void APcopter_posCtrlRC::clear(void)
{
	m_rc[CTRL_PITCH].clear();
	m_rc[CTRL_ROLL].clear();
	m_rc[CTRL_ALT].clear();
	m_rc[CTRL_YAW].clear();
}

void APcopter_posCtrlRC::resetRC(void)
{
	m_rcO.chan1_raw = 0;
	m_rcO.chan2_raw = 0;
	m_rcO.chan3_raw = 0;
	m_rcO.chan4_raw = 0;
	m_rcO.chan5_raw = 0;
	m_rcO.chan6_raw = 0;
	m_rcO.chan7_raw = 0;
	m_rcO.chan8_raw = 0;
}

void APcopter_posCtrlRC::releaseRC(void)
{
	IF_(check()<0);

	resetRC();
	m_pAP->m_pMavlink->rcChannelsOverride(m_rcO);
}

bool APcopter_posCtrlRC::draw(void)
{
	IF_F(!this->APcopter_posCtrlBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg = *this->getName();
	pWin->addMsg(msg);

	pWin->tabNext();

	msg = "PWM = (" + i2str(m_rcO.chan1_raw) + ", "
					 + i2str(m_rcO.chan2_raw) + ", "
					 + i2str(m_rcO.chan3_raw) + ", "
					 + i2str(m_rcO.chan4_raw) + ", "
					 + i2str(m_rcO.chan5_raw) + ", "
					 + i2str(m_rcO.chan6_raw) + ", "
					 + i2str(m_rcO.chan7_raw) + ", "
					 + i2str(m_rcO.chan8_raw) + ")";
	pWin->addMsg(msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_posCtrlRC::console(int& iY)
{
	IF_F(!this->APcopter_posCtrlBase::console(iY));
	IF_F(check()<0);

	string msg;

	msg = "PWM = (" + i2str(m_rcO.chan1_raw) + ", "
					 + i2str(m_rcO.chan2_raw) + ", "
					 + i2str(m_rcO.chan3_raw) + ", "
					 + i2str(m_rcO.chan4_raw) + ", "
					 + i2str(m_rcO.chan5_raw) + ", "
					 + i2str(m_rcO.chan6_raw) + ", "
					 + i2str(m_rcO.chan7_raw) + ", "
					 + i2str(m_rcO.chan8_raw) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CONSOLE_X_MSG, msg.c_str());

	return true;
}

}
