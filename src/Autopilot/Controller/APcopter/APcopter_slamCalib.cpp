#include "APcopter_slamCalib.h"

namespace kai
{

APcopter_slamCalib::APcopter_slamCalib()
{
	m_pAP = NULL;
	m_pSlam = NULL;
	m_dist = 1.0;
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
	KISSm(pK, dist);

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("APcopter_slam", &iName);
	m_pSlam = (APcopter_slam*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pSlam, iName + ": not found");

	return true;
}

int APcopter_slamCalib::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pSlam,-1);

	return this->ActionBase::check();
}

void APcopter_slamCalib::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!bActive());
	IF_(m_pAP->m_apMode != m_apModeCalib);

	if(m_pAP->m_bApModeChanged)
	{
		m_vPos.clear();
		vDouble2 iPos;
		iPos.init();
		m_vPos.push_back(iPos);
	}

	vDouble2 cPos;
	cPos.x = m_pSlam->m_vSlamPos.x;
	cPos.y = m_pSlam->m_vSlamPos.y;

	int i = m_vPos.size()- 1;
	vDouble2 dPos =  m_vPos[i] - cPos;

	IF_(dPos.len() < m_dist);

	m_vPos.push_back(m_vPos[i]);

	//TODO: approximate the path to line


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

	if(!bActive())
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

	if(!bActive())
	{
		msg = "Inactive";
	}
	else
	{
		msg = "Calibrating: " + i2str(m_vPos.size()) + " Points; yawOffset = " + f2str((double)m_yawOffset);
	}
	COL_MSG;
	iY++;
	mvaddstr(iY, CONSOLE_X_MSG, msg.c_str());

	return true;
}

}
