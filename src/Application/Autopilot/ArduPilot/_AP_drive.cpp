#include "_AP_drive.h"

namespace kai
{

_AP_drive::_AP_drive()
{
	m_pAP = NULL;

	m_speed = 0.0;
	m_yaw = 0.0;
	m_yawMode = 1.0;
}

_AP_drive::~_AP_drive()
{
}

bool _AP_drive::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("speed",&m_speed);
	pK->v("yaw",&m_yaw);
	pK->v("yawMode",&m_yawMode);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _AP_drive::start(void)
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

int _AP_drive::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->_AutopilotBase::check();
}

void _AP_drive::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateDrive();

		this->autoFPSto();
	}
}

bool _AP_drive::updateDrive(void)
{
	IF_F(check() < 0);

	m_pAP->m_pMavlink->clNavSetYawSpeed(m_yaw, m_speed, m_yawMode);

	return true;
}

void _AP_drive::setSpeed(float v)
{
	m_speed = v;
}

void _AP_drive::setYaw(float v)
{
	m_yaw = v;
}

void _AP_drive::setYawMode(bool bRelative)
{
	if(bRelative)
		m_yawMode = 1.0;
	else
		m_yawMode = 0.0;
}

void _AP_drive::draw(void)
{
	this->_AutopilotBase::draw();
}

}
