#include "../ArduPilot/_AP_CETCUS.h"

namespace kai
{

_AP_CETCUS::_AP_CETCUS()
{
	m_pAP = NULL;
	m_pC = NULL;

//	m_ieSend.init(100000);
}

_AP_CETCUS::~_AP_CETCUS()
{
}

bool _AP_CETCUS::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_CETCUS", &iName);
	m_pC = (_CETCUS*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pC, iName + ": not found");

	return true;
}

bool _AP_CETCUS::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_CETCUS::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pC,-1);

	return this->_AutopilotBase::check();
}

void _AP_CETCUS::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();

		updateCETCUS();

		this->autoFPSto();
	}
}

bool _AP_CETCUS::updateCETCUS(void)
{
	IF_F(check()<0);
	IF_F(!bActive());

	_Mavlink* pMav = m_pAP->m_pMavlink;

	m_pC->m_vPos = m_pAP->getGlobalPos();
	m_pC->m_vAtti = m_pAP->getApAttitude() * RAD2DEG;
	m_pC->m_course = m_pAP->getApHdg();
	m_pC->m_vSpeed.x = m_pAP->getApSpeed().len();

	//TODO

	return true;
}

void _AP_CETCUS::draw(void)
{
	this->_AutopilotBase::draw();
}

}
