#include "APcopter_slamCalib.h"

namespace kai
{

APcopter_slamCalib::APcopter_slamCalib()
{
	m_pAP = NULL;
	m_bCalibrating = false;
	m_apModeCalib = POSHOLD;
	m_yawOffset = 0.0;
}

APcopter_slamCalib::~APcopter_slamCalib()
{
}

bool APcopter_slamCalib::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, apModeCalib);

	//link
	string iName;
	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int APcopter_slamCalib::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_slamCalib::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!isActive());
	IF_(m_pAP->m_apMode != m_apModeCalib);

	if(m_pAP->m_bApModeChanged)
	{
		m_vPos.clear();
	}



}

bool APcopter_slamCalib::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string* pMission = m_pMC->getCurrentMissionName();
	string msg;

	pWin->tabNext();

	if(!isActive())
	{
		msg = "Inactive";
		pWin->addMsg(&msg);
	}
	else
	{
		msg = "Calibrating: " + i2str(m_vPos.size()) + " Points; yawOffset = " + f2str((double)m_yawOffset);
		pWin->addMsg(&msg);
	}

	pWin->tabPrev();

	return true;
}

bool APcopter_slamCalib::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string* pState = m_pMC->getCurrentMissionName();
	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else
	{
		msg = "Calibrating: " + i2str(m_vPos.size()) + " Points; yawOffset = " + f2str((double)m_yawOffset);
	}
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
