#include "_Rover_drive.h"

namespace kai
{

_Rover_drive::_Rover_drive()
{
	m_pCMD = NULL;
	m_pPID = NULL;
	m_nSpeed = 0.0;
	m_dSpeed = 0.0;

	m_ctrl.init();
	m_pCtrl = (void*)&m_ctrl;

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
	F_ERROR_F(pK->v("_Rover_CMD", &iName));
	m_pCMD = (_Rover_CMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	iName = "";
	pK->v("PID", &iName);
	m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPID, iName+": not found");

	return true;
}

bool _Rover_drive::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _Rover_drive::check(void)
{
	NULL__(m_pAB, -1);
	NULL__(m_pAB->m_pCtrl, -1);
	NULL__(m_pCMD, -1);
	NULL__(m_pPID, -1);

	return 0;
}

void _Rover_drive::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateDrive();

		this->autoFPSto();
	}
}

void _Rover_drive::updateDrive(void)
{
	IF_(check()<0);

	m_ctrl = *((ROVER_CTRL*)m_pAB->m_pCtrl);
	if(m_ctrl.m_hdg < 0.0)
	{
		m_nSpeed = 0.0;
		m_dSpeed = 0.0;
	}
	else
	{
		m_nSpeed = m_ctrl.m_nTargetSpeed;
		m_dSpeed = m_pPID->update(0.0, m_ctrl.m_targetHdgOffset, m_tStamp);
	}

	m_pCMD->setSpeed(m_nSpeed, m_dSpeed);
}

void _Rover_drive::draw(void)
{
	this->_AutopilotBase::draw();

	string msg;
	msg = "nSpeed=" + f2str(m_nSpeed) + ", dSpeed=" + f2str(m_dSpeed);
	addMsg(msg,1);

}

}
