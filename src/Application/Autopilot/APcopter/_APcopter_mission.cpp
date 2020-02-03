#include "_APcopter_mission.h"

namespace kai
{

_APcopter_mission::_APcopter_mission()
{
	m_pAP = NULL;
	m_iWP = -1;
}

_APcopter_mission::~_APcopter_mission()
{
}

bool _APcopter_mission::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("iWP",&m_iWP);

	string iName;
	iName = "";
	pK->v("_APcopter_base", &iName);
	m_pAP = (_APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _APcopter_mission::start(void)
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

int _APcopter_mission::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _APcopter_mission::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateMission();

		this->autoFPSto();
	}
}

void _APcopter_mission::updateMission(void)
{
	IF_(check()<0);

	int apMode = m_pAP->getApMode();
	int apMissionSeq = m_pAP->m_pMavlink->m_mavMsg.mission_current.seq;

	if(apMode == ALT_HOLD)
	{
		m_pMC->transit("STANDBY");
		return;
	}

	if(apMode == AUTO && apMissionSeq == m_iWP)
	{
		m_pAP->setApMode(GUIDED);
		return;
	}

	string mission = m_pMC->getCurrentMissionName();

	if(mission == "RTH")
	{
		IF_(apMode == ALT_HOLD);
		IF_(apMode == LOITER);
		IF_(apMode == STABILIZE);
		IF_(apMode == RTL);

		m_pAP->setApMode(RTL);
		return;
	}

	if(apMode == GUIDED)
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

void _APcopter_mission::draw(void)
{
	this->_AutopilotBase::draw();

}

}
