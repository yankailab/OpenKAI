#include "_Rover_base.h"

namespace kai
{

_Rover_base::_Rover_base()
{
	m_pCMD = NULL;
	m_pMavlink = NULL;

	m_ctrl.init();
	m_pCtrl = (void*)&m_ctrl;
}

_Rover_base::~_Rover_base()
{
}

bool _Rover_base::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_Rover_CMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName+": not found");

	return true;
}

bool _Rover_base::start(void)
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

int _Rover_base::check(void)
{
	NULL__(m_pCMD, -1);
	NULL__(m_pMavlink, -1);

	return 0;
}

void _Rover_base::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateRover();

		this->autoFPSto();
	}
}

void _Rover_base::updateRover(void)
{
	IF_(check()<0);

	//hard switch priority
	if(m_pCMD->m_mode == rover_stop)
	{
		m_pMC->transit("STOP");
	}
	else if(m_pCMD->m_mode == rover_manual)
	{
		if(m_pCMD->m_action == rover_action3)
			m_pMC->transit("RECORD");
		else
			m_pMC->transit("MANUAL");

	}
	else if(m_pCMD->m_mode == rover_auto1)
	{
		if(m_pCMD->m_action == rover_action1)
			m_pMC->transit("WP");
		else if(m_pCMD->m_action == rover_action2)
			m_pMC->transit("STOP");
		else
			m_pMC->transit("STOP");

	}
	else if(m_pCMD->m_mode == rover_auto2)
	{
		m_pMC->transit("FOLLOW");
	}


	//sensor
	if(m_tStamp - m_pMavlink->m_mavMsg.time_stamps.global_position_int > USEC_1SEC)
	{
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, 5);
		return;
	}

	uint16_t h = m_pMavlink->m_mavMsg.global_position_int.hdg;
	if(h < UINT16_MAX)
	{
		m_ctrl.m_hdg = (float)h * 1e-2;
	}

	m_ctrl.m_nSpeed = m_pCMD->m_nSpeed;
	m_ctrl.m_nTargetSpeed = 0.0;
	m_ctrl.m_targetHdgOffset = 0.0;
}

void _Rover_base::draw(void)
{
	this->_AutopilotBase::draw();

	string msg = "hdg=" + f2str(m_ctrl.m_hdg)
			+ ", targetHdgOffset=" + f2str(m_ctrl.m_targetHdgOffset)
			+ ", nSpeed=" + f2str(m_ctrl.m_nSpeed)
			+ ", nTargetSpeed=" + f2str(m_ctrl.m_nTargetSpeed);
	addMsg(msg);
}

}
