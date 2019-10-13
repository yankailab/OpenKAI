#include "_Rover_drive.h"

namespace kai
{

_Rover_drive::_Rover_drive()
{
	m_pAB = NULL;
	m_pCMD = NULL;

	m_ctrl.init();
}

_Rover_drive::~_Rover_drive()
{
}

bool _Rover_drive::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_AutopilotBase", &iName));
	m_pAB = (_AutopilotBase*) (pK->parent()->getChildInst(iName));
	NULL_Fl(m_pAB, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_Rover_CMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	return true;
}

int _Rover_drive::check(void)
{
	NULL__(m_pAB, -1);
	NULL__(m_pCMD, -1);

	return 0;
}

void _Rover_drive::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	IF_(!bActive());

	ROVER_CTRL* pCtrl = (ROVER_CTRL*)m_pAB->m_pCtrl;
	m_ctrl.m_vDrive = pCtrl->m_vDrive;

	//	//mission
	//	string mission = m_pMC->getCurrentMissionName();
	//	if(mission == "IDLE")
	//	{
	//		m_targetHdg = -1.0;
	//	}
	//	else if(m_targetHdg < 0.0)
	//	{
	//		m_targetHdg = m_hdg;
	//	}

}

void _Rover_drive::updatePWM(void)
{
//	IF_(m_hdg < 0.0);
//	IF_(m_targetHdg < 0.0);
//
//	float dSpeed = m_pPIDhdg->update(0.0, dHdg(m_hdg, m_targetHdg), m_tStamp);
//	string mission = m_pMC->getCurrentMissionName();
//	if(mission == "TAG")
//		dSpeed = 0.0;
//
//	uint16_t pPWM[ROVER_N_MOTOR];
//	for(int i=0; i<m_vPWM.size(); i++)
//		pPWM[m_vPWM[i].m_iChan] = m_vPWM[i].updatePWM(m_nSpeed, dSpeed);
//
//	m_pCMD->setPWM(m_vPWM.size(), pPWM);
}

void _Rover_drive::draw(void)
{
	this->_AutopilotBase::draw();

}

}
