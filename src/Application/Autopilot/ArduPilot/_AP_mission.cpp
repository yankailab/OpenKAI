#include "../ArduPilot/_AP_mission.h"

namespace kai
{

_AP_mission::_AP_mission()
{
	m_pAP = NULL;
	m_iWP = -1;
}

_AP_mission::~_AP_mission()
{
}

bool _AP_mission::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("iWP",&m_iWP);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _AP_mission::start(void)
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

int _AP_mission::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _AP_mission::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateMission();

		this->autoFPSto();
	}
}

void _AP_mission::updateMission(void)
{
	IF_(check()<0);

	int apMode = m_pAP->getApMode();
	int apMissionSeq = m_pAP->m_pMavlink->m_mavMsg.m_mission_current.seq;

	if(apMode == AP_COPTER_ALT_HOLD ||
		apMode == AP_COPTER_LOITER ||
		apMode == AP_COPTER_STABILIZE
	)
	{
		m_pMC->transit("STANDBY");
		return;
	}

	if(apMode == AP_COPTER_AUTO && apMissionSeq == m_iWP)
	{
		m_pAP->setApMode(AP_COPTER_GUIDED);
		return;
	}

	string mission = m_pMC->getCurrentMissionName();

	if(mission == "RTH")
	{
		IF_(apMode == AP_COPTER_ALT_HOLD);
		IF_(apMode == AP_COPTER_LOITER);
		IF_(apMode == AP_COPTER_STABILIZE);
		IF_(apMode == AP_COPTER_RTL);

		m_pAP->setApMode(AP_COPTER_RTL);
		return;
	}

	if(apMode == AP_COPTER_GUIDED)
	{
		IF_(mission == "RELEASE");
		IF_(mission == "RTH");

		if(mission != "LAND")
		{
			m_pMC->transit("LAND");
			return;
		}
	}

}

void _AP_mission::draw(void)
{
	this->_AutopilotBase::draw();

}

}
