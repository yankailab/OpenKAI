#include "../ArduPilot/_AP_takeoff.h"

namespace kai
{

_AP_takeoff::_AP_takeoff()
{
	m_pAP = NULL;
	m_apMode = AP_COPTER_GUIDED;
}

_AP_takeoff::~_AP_takeoff()
{
}

bool _AP_takeoff::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("apMode", &m_apMode);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _AP_takeoff::start(void)
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

int _AP_takeoff::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);

	return this->_AutopilotBase::check();
}

void _AP_takeoff::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateMission();

		this->autoFPSto();
	}
}

void _AP_takeoff::updateMission(void)
{
	IF_(check()<0);
	IF_(!bActive());
	IF_(m_pMC->getMissionType() != mission_takeoff);
	IF_(!m_pAP->bApArmed());
	if(m_apMode >= 0)
	{
		IF_(m_pAP->getApMode() != m_apMode);
	}

	Takeoff* pTO = (Takeoff*)m_pMC->getMission();
	NULL_(pTO);

	if(EAQ(m_pAP->m_vGlobalPos.w, pTO->m_alt, 0.5) &&
	   m_pAP->m_pMavlink->m_mavMsg.m_heartbeat.system_status == MAV_STATE_ACTIVE)
	{
		pTO->complete();
		return;
	}

	m_pAP->m_pMavlink->clNavTakeoff(pTO->m_alt);
}

void _AP_takeoff::draw(void)
{
	IF_(check()<0);
	this->_AutopilotBase::draw();
	drawActive();

	addMsg("system status="+i2str(m_pAP->m_pMavlink->m_mavMsg.m_heartbeat.system_status));
	if(m_pAP->m_pMavlink->m_mavMsg.m_heartbeat.system_status == MAV_STATE_ACTIVE)
	{
		addMsg("Taken off", 1);
	}

}

}
