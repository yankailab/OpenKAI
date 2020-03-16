#include "_AProver_drive.h"

namespace kai
{

_AProver_drive::_AProver_drive()
{
	m_pAP = NULL;

	m_nSpeed = 0.0;
	m_kSpeed = 1.0;
	m_yaw = 0.0;

	m_speed = 1.0;	//1.0m/s
	m_yawMode = 1.0;
}

_AProver_drive::~_AProver_drive()
{
}

bool _AProver_drive::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nSpeed",&m_nSpeed);
	pK->v("kSpeed",&m_kSpeed);
	pK->v("yaw",&m_yaw);

	pK->v("speed",&m_speed);
	pK->v("yawMode",&m_yawMode);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _AProver_drive::start(void)
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

int _AProver_drive::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->_AutopilotBase::check();
}

void _AProver_drive::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateDrive();

		this->autoFPSto();
	}
}

bool _AProver_drive::updateDrive(void)
{
	IF_F(check() < 0);

	m_pAP->m_pMavlink->clNavSetYawSpeed(m_yaw,
										m_nSpeed * m_kSpeed * m_speed,
										m_yawMode);

	return true;
}

void _AProver_drive::setSpeed(float nSpeed)
{
	m_nSpeed = nSpeed;
}

void _AProver_drive::setYaw(float yaw)
{
	m_yaw = yaw;
}

void _AProver_drive::setYawMode(bool bRelative)
{
	if(bRelative)
		m_yawMode = 1.0;
	else
		m_yawMode = 0.0;
}

void _AProver_drive::draw(void)
{
	this->_AutopilotBase::draw();
}

}
