#include "APcopter_posCtrlRC.h"

namespace kai
{

APcopter_posCtrlRC::APcopter_posCtrlRC()
{
	m_pAP = NULL;
	m_iModeEnable = ALT_HOLD;

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pYaw = NULL;
	m_pAlt = NULL;

	m_vTarget.init();
	m_vPos.init();

	m_pwmLow = 1000;
	m_pwmMidR = 1500;
	m_pwmMidP = 1500;
	m_pwmMidY = 1500;
	m_pwmMidA = 1500;
	m_pwmHigh = 2000;

}

APcopter_posCtrlRC::~APcopter_posCtrlRC()
{
}

bool APcopter_posCtrlRC::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,iModeEnable);
	KISSm(pK,pwmLow);
	KISSm(pK,pwmMidR);
	KISSm(pK,pwmMidP);
	KISSm(pK,pwmMidY);
	KISSm(pK,pwmMidA);
	KISSm(pK,pwmHigh);

	pK->v("x", &m_vTarget.x);
	pK->v("y", &m_vTarget.y);
	pK->v("z", &m_vTarget.z);

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("PIDroll", &iName);
	m_pRoll = (PIDctrl*) (pK->root()->getChildInstByName(iName));
	IF_Fl(!m_pRoll, iName + ": not found");

	iName = "";
	pK->v("PIDpitch", &iName);
	m_pPitch = (PIDctrl*) (pK->root()->getChildInstByName(iName));
	IF_Fl(!m_pPitch, iName + ": not found");

	iName = "";
	pK->v("PIDyaw", &iName);
	m_pYaw = (PIDctrl*) (pK->root()->getChildInstByName(iName));
	IF_Fl(!m_pYaw, iName + ": not found");

	iName = "";
	pK->v("PIDalt", &iName);
	m_pAlt = (PIDctrl*) (pK->root()->getChildInstByName(iName));
	IF_Fl(!m_pAlt, iName + ": not found");

	return true;
}

int APcopter_posCtrlRC::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pRoll,-1);
	NULL__(m_pPitch,-1);
	NULL__(m_pYaw,-1);
	NULL__(m_pAlt,-1);

	return 0;
}

void APcopter_posCtrlRC::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!isActive());
	_Mavlink* pMav = m_pAP->m_pMavlink;
	IF_(pMav->m_msg.heartbeat.custom_mode != m_iModeEnable);

	mavlink_rc_channels_override_t rc;
	rc.chan1_raw = (uint16_t)(m_pwmMidP + (int)m_pPitch->update(m_vPos.y, m_vTarget.y, m_vPos.z));
	rc.chan2_raw = (uint16_t)(m_pwmMidR + (int)m_pRoll->update(m_vPos.x, m_vTarget.x, m_vPos.z));
	rc.chan3_raw = 0;
	rc.chan4_raw = 0;
	rc.chan5_raw = 0;
	rc.chan6_raw = 0;
	rc.chan7_raw = 0;
	rc.chan8_raw = 0;
	m_pAP->m_pMavlink->rcChannelsOverride(rc);
}

void APcopter_posCtrlRC::setTargetPos(vDouble3& vT)
{
	m_vTarget = vT;
}

void APcopter_posCtrlRC::setPos(vDouble3& vT)
{
	m_vPos = vT;
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
