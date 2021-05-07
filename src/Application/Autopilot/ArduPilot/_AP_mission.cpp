#include "../ArduPilot/_AP_mission.h"

namespace kai
{

_AP_mission::_AP_mission()
{
	m_pAP = NULL;
//	m_pAPdescent = NULL;
}

_AP_mission::~_AP_mission()
{
}

bool _AP_mission::init(void* pKiss)
{
	IF_F(!this->_StateBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	string n;
	n = "";
	pK->v("_AP_base", &n);
	m_pAP = (_AP_base*) (pK->getInst(n));
	IF_Fl(!m_pAP, n + ": not found");

//	n = "";
//	pK->v("_AP_descent", &n);
//	m_pAPdescent = (_AP_descent*) (pK->getInst(n));
//	IF_Fl(!m_pAPdescent, n + ": not found");

	return true;
}

bool _AP_mission::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AP_mission::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);
//	NULL__(m_pAPdescent, -1);

	return this->_StateBase::check();
}

void _AP_mission::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		this->_StateBase::update();
		updateMission();

		m_pT->autoFPSto();
	}
}

void _AP_mission::updateMission(void)
{
	IF_(check()<0);

	int apMode = m_pAP->getApMode();
	string mission = m_pSC->getStateName();

	if(apMode != AP_COPTER_GUIDED)
	{
		m_pSC->transit("STANDBY");
		return;
	}

	if(mission == "STANDBY")
	{
		m_pSC->transit("TAKEOFF");
		return;
	}

//	if(mission == "FOLLOW")
//	{
//		if(m_pAPdescent->m_bTarget)
//			m_pSC->transit("DESCENT");
//		return;
//	}
//
//	if(mission == "DESCENT")
//	{
//		if(!m_pAPdescent->m_bTarget)
//			m_pSC->transit("FOLLOW");
//		return;
//	}

	if(mission == "RTH")
	{
		IF_(apMode != AP_COPTER_GUIDED);

		m_pAP->setApMode(AP_COPTER_RTL);
		return;
	}

}

}
