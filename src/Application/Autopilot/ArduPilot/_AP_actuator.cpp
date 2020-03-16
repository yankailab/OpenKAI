#include "../ArduPilot/_AP_actuator.h"

namespace kai
{

_AP_actuator::_AP_actuator()
{
	m_pMav = NULL;
	m_pRCmode = NULL;
	m_iMode = -1;
}

_AP_actuator::~_AP_actuator()
{
}

bool _AP_actuator::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMav = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMav, "_Mavlink: " + iName + " not found");

	pK->v("iMode", &m_iMode);
	int iRC = -1;
	pK->v("iRCmode", &iRC);
	m_pRCmode = m_pMav->getRCinScaled(iRC);
	NULL_Fl(m_pRCmode, "iRCmode not correct");

	Kiss** pItr = pK->getChildItr();
	int i = 0;
	while (pItr[i])
	{
		Kiss* pA = pItr[i++];
		AP_actuator a;
		a.init();

		iName = "";
		pA->v("_ActuatorBase", &iName);
		a.m_pA = (_ActuatorBase*) (pK->root()->getChildInst(iName));
		NULL_Fl(a.m_pA, "_ActuatorBase: " + iName + " not found");

		iRC = -1;
		pA->v("iRCinScaled", &iRC);
		a.m_pRCinScaled = m_pMav->getRCinScaled(iRC);
		NULL_Fl(a.m_pRCinScaled, "iRCinScaled not correct");

		pA->v("vSpeed", &a.m_vSpeed);
		pA->v("iMode", &a.m_iMode);

		m_vActuator.push_back(a);
	}

	return true;
}

bool _AP_actuator::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_actuator::check(void)
{
	NULL__(m_pMav, -1);

	return this->_AutopilotBase::check();
}

void _AP_actuator::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateActuator();

		this->autoFPSto();
	}
}

void _AP_actuator::updateActuator(void)
{
	IF_(check() < 0);
	IF_(!bActive());

	if(*m_pRCmode > MAV_RC_SCALE || *m_pRCmode < -MAV_RC_SCALE)
	{
		m_iMode = -1;
		return;
	}

	if(*m_pRCmode < 0)
		m_iMode = 0;
	else if(*m_pRCmode > 0)
		m_iMode = 1;

	for (AP_actuator a : m_vActuator)
	{
		IF_CONT(a.m_iMode != m_iMode);

		a.update();
	}
}

void _AP_actuator::draw(void)
{
	IF_(check() < 0);
	this->_AutopilotBase::draw();
	drawActive();

	addMsg("iMode: "+i2str(m_iMode), 1);
}

}
