#include "APcopter_posCtrlRC.h"

namespace kai
{

APcopter_posCtrlRC::APcopter_posCtrlRC()
{
	m_pAP = NULL;
	m_bON = false;

	m_vTarget.init();
	m_vPos.init();

	for(int i=0;i<N_RC_CHAN;i++)
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
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss* pR;
	string iName;
	AP_POS_CTRL_RC* pRC = NULL;

	pR = pK->o("roll");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		pRC = &m_rc[RC_CHAN_ROLL];
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
		pRC = &m_rc[RC_CHAN_PITCH];
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
		pRC = &m_rc[RC_CHAN_YAW];
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
		pRC = &m_rc[RC_CHAN_ALT];
		pRC->m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!pRC->m_pPID, iName + ": not found");
		pR->v("pwmL", &pRC->m_pwmL);
		pR->v("pwmM", &pRC->m_pwmM);
		pR->v("pwmH", &pRC->m_pwmH);
		pR->v("iChan", &pRC->m_iChan);
		pRC->setRCChannel(&m_rcO);
	}

	//link
	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int APcopter_posCtrlRC::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return 0;
}

void APcopter_posCtrlRC::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!isActive() || !m_bON)
	{
		releaseRC();
		return;
	}

	resetRC();
	AP_POS_CTRL_RC* pRC;

	pRC = &m_rc[RC_CHAN_ROLL];
	if(pRC->m_bON)
		pRC->updatePWM((uint16_t)(pRC->m_pwmM + (int)pRC->m_pPID->update(m_vPos.x, m_vTarget.x)));

	pRC = &m_rc[RC_CHAN_PITCH];
	if(pRC->m_bON)
		pRC->updatePWM((uint16_t)(pRC->m_pwmM + (int)pRC->m_pPID->update(m_vPos.y, m_vTarget.y)));

	pRC = &m_rc[RC_CHAN_ALT];
	if(pRC->m_bON)
		pRC->updatePWM((uint16_t)(pRC->m_pwmM + (int)pRC->m_pPID->update(m_vPos.z, m_vTarget.z)));

	pRC = &m_rc[RC_CHAN_YAW];
	if(pRC->m_bON)
		pRC->updatePWM((uint16_t)(pRC->m_pwmM + (int)pRC->m_pPID->update(m_vPos.w, m_vTarget.w)));

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

void APcopter_posCtrlRC::setCtrl(uint8_t iChan, bool bON)
{
	IF_(iChan >= N_RC_CHAN);

	m_rc[iChan].bON(bON);
}

void APcopter_posCtrlRC::bON(bool bON)
{
	m_bON = bON;
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
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg = *this->getName() + ": ";

	if(!isActive())
	{
		msg += "Inactive";
	}
	else
	{
		circle(*pMat, Point(m_vPos.x * pMat->cols,
							m_vPos.y * pMat->rows),
				pMat->cols * pMat->rows * 0.00005, Scalar(0, 0, 255), 2);

		msg += "Pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ", "
							   + f2str(m_vPos.w) + ")";
	}

	pWin->addMsg(&msg);

	return true;
}

bool APcopter_posCtrlRC::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else
	{
		msg = "Pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ", "
							   + f2str(m_vPos.w) + ")";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
