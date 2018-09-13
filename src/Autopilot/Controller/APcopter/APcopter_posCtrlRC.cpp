#include "APcopter_posCtrlRC.h"

namespace kai
{

APcopter_posCtrlRC::APcopter_posCtrlRC()
{
	m_pAP = NULL;
	m_fCtrl = 0;
	m_bON = false;

	m_vTarget.init();
	m_vPos.init();

	m_rcRoll.init();
	m_rcPitch.init();
	m_rcYaw.init();
	m_rcAlt.init();
	resetRC();
}

APcopter_posCtrlRC::~APcopter_posCtrlRC()
{
}

bool APcopter_posCtrlRC::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	m_fCtrl = 0;
	Kiss* pR;
	string iName;

	pR = pK->o("roll");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		m_rcRoll.m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_rcRoll.m_pPID, iName + ": not found");
		pR->v("pwmL", &m_rcRoll.m_pwmL);
		pR->v("pwmM", &m_rcRoll.m_pwmM);
		pR->v("pwmH", &m_rcRoll.m_pwmH);
		m_fCtrl |= (1 << RC_CHAN_ROLL);
	}

	pR = pK->o("pitch");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		m_rcPitch.m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_rcPitch.m_pPID, iName + ": not found");
		pR->v("pwmL", &m_rcPitch.m_pwmL);
		pR->v("pwmM", &m_rcPitch.m_pwmM);
		pR->v("pwmH", &m_rcPitch.m_pwmH);
		m_fCtrl |= (1 << RC_CHAN_PITCH);
	}

	pR = pK->o("yaw");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		m_rcYaw.m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_rcYaw.m_pPID, iName + ": not found");
		pR->v("pwmL", &m_rcYaw.m_pwmL);
		pR->v("pwmM", &m_rcYaw.m_pwmM);
		pR->v("pwmH", &m_rcYaw.m_pwmH);
		m_fCtrl |= (1 << RC_CHAN_YAW);
	}

	pR = pK->o("alt");
	if(!pR->empty())
	{
		iName = "";
		pR->v("PIDctrl", &iName);
		m_rcAlt.m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_rcAlt.m_pPID, iName + ": not found");
		pR->v("pwmL", &m_rcAlt.m_pwmL);
		pR->v("pwmM", &m_rcAlt.m_pwmM);
		pR->v("pwmH", &m_rcAlt.m_pwmH);
		m_fCtrl |= (1 << RC_CHAN_ALT);
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
	if(m_fCtrl & (1 << RC_CHAN_PITCH))
		m_rc.chan1_raw = (uint16_t)(m_rcPitch.m_pwmM + (int)m_rcPitch.m_pPID->update(m_vPos.y, m_vTarget.y));

	if(m_fCtrl & (1 << RC_CHAN_ROLL))
		m_rc.chan2_raw = (uint16_t)(m_rcRoll.m_pwmM + (int)m_rcRoll.m_pPID->update(m_vPos.x, m_vTarget.x));

	if(m_fCtrl & (1 << RC_CHAN_ALT))
		m_rc.chan3_raw = (uint16_t)(m_rcAlt.m_pwmM + (int)m_rcAlt.m_pPID->update(m_vPos.z, m_vTarget.z));

	if(m_fCtrl & (1 << RC_CHAN_YAW))
		m_rc.chan4_raw = (uint16_t)(m_rcYaw.m_pwmM + (int)m_rcYaw.m_pPID->update(m_vPos.w, m_vTarget.w));

	m_pAP->m_pMavlink->rcChannelsOverride(m_rc);
}

void APcopter_posCtrlRC::setTargetPos(vDouble4& vT, uint8_t fSet)
{
	if(m_rcRoll.m_pPID && (fSet & (1<<RC_CHAN_ROLL)))
	{
		m_vTarget.x = vT.x;
		m_rcRoll.m_pPID->reset();
	}

	if(m_rcPitch.m_pPID && (fSet & (1<<RC_CHAN_PITCH)))
	{
		m_vTarget.y = vT.y;
		m_rcPitch.m_pPID->reset();
	}

	if(m_rcAlt.m_pPID && (fSet & (1<<RC_CHAN_ALT)))
	{
		m_vTarget.z = vT.z;
		m_rcAlt.m_pPID->reset();
	}

	if(m_rcYaw.m_pPID && (fSet & (1<<RC_CHAN_YAW)))
	{
		m_vTarget.w = vT.w;
		m_rcYaw.m_pPID->reset();
	}
}

void APcopter_posCtrlRC::setPos(vDouble4& vP)
{
	m_vPos = vP;
}

void APcopter_posCtrlRC::bON(bool bON)
{
	m_bON = bON;
}

void APcopter_posCtrlRC::resetRC(void)
{
	m_rc.chan1_raw = 0;
	m_rc.chan2_raw = 0;
	m_rc.chan3_raw = 0;
	m_rc.chan4_raw = 0;
	m_rc.chan5_raw = 0;
	m_rc.chan6_raw = 0;
	m_rc.chan7_raw = 0;
	m_rc.chan8_raw = 0;
}

void APcopter_posCtrlRC::releaseRC(void)
{
	IF_(check()<0);

	resetRC();
	m_pAP->m_pMavlink->rcChannelsOverride(m_rc);
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
							   + f2str(m_vPos.z) + ")";
	}

	pWin->addMsg(&msg);

	circle(*pMat, Point(m_vTarget.x * pMat->cols,
						m_vTarget.y * pMat->rows),
			pMat->cols * pMat->rows * 0.00005, Scalar(0, 255, 0), 2);

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
							   + f2str(m_vPos.z) + ")";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
